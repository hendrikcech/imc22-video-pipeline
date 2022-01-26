package gst

/*
#cgo pkg-config: gstreamer-1.0

#include "gst.h"

*/
import "C"
import (
	"bytes"
	"errors"
	"io"
	"log"
	"sync"
	"unsafe"
)

var ErrUnknownCodec = errors.New("unknown codec")

// StartMainLoop starts GLib's main loop
// It needs to be called from the process' main thread
// Because many gstreamer plugins require access to the main thread
// See: https://golang.org/pkg/runtime/#LockOSThread
func StartMainLoop() {
	C.gstreamer_send_start_mainloop()
}

var pipelines = map[int]*Pipeline{}
var pipelinesLock sync.Mutex

type Pipeline struct {
	id          int
	pipeline    *C.GstElement
	writer      *io.PipeWriter
	reader      *io.PipeReader
	pipelineStr string
	payloder    string
	codec       string
}

func NewPipeline(codec, src string) (*Pipeline, error) {
	pipelineStr := "appsink name=appsink"
	var payloader string

	switch codec {
	case "vp8":
		payloader = "rtpvp8pay"
		pipelineStr = src + " ! vp8enc name=encoder error-resilient=partitions keyframe-max-dist=10 auto-alt-ref=true cpu-used=5 deadline=1 ! rtpvp8pay name=rtpvp8pay mtu=1200 seqnum-offset=0 ! " + pipelineStr

	case "vp9":
		payloader = "rtpvp9pay"
		pipelineStr = src + " ! vp9enc name=encoder keyframe-max-dist=10 auto-alt-ref=true cpu-used=5 ! rtpvp9pay name=rtpvp9pay mtu=1200 seqnum-offset=0 ! " + pipelineStr

	case "h264":
		payloader = "rtph264pay"
		pipelineStr = src + " ! x264enc name=encoder pass=5 speed-preset=4 tune=4 ! rtph264pay name=rtph264pay mtu=1200 seqnum-offset=0 ! " + pipelineStr

	default:
		return nil, ErrUnknownCodec
	}

	pipelineStrUnsafe := C.CString(pipelineStr)
	defer C.free(unsafe.Pointer(pipelineStrUnsafe))

	pipelinesLock.Lock()
	defer pipelinesLock.Unlock()

	r, w := io.Pipe()
	sp := &Pipeline{
		id:          len(pipelines),
		pipeline:    C.gstreamer_send_create_pipeline(pipelineStrUnsafe),
		pipelineStr: pipelineStr,
		payloder:    payloader,
		codec:       codec,
		writer:      w,
		reader:      r,
	}
	pipelines[sp.id] = sp
	return sp, nil
}

func (p *Pipeline) Read(buf []byte) (int, error) {
	return p.reader.Read(buf)
}

func (p *Pipeline) String() string {
	return p.pipelineStr
}

func (p *Pipeline) Start() {
	C.gstreamer_send_start_pipeline(p.pipeline, C.int(p.id))
}

func (p *Pipeline) Stop() {
	C.gstreamer_send_stop_pipeline(p.pipeline)
}

func (p *Pipeline) Destroy() {
	C.gstreamer_send_destroy_pipeline(p.pipeline)
}

var eosHandler func()

func HandleSrcEOS(handler func()) {
	eosHandler = handler
}

//export goHandleSendEOS
func goHandleSendEOS() {
	if eosHandler != nil {
		eosHandler()
	}
}

func (p *Pipeline) setPropertyUint(name string, prop string, value uint) {
	cName := C.CString(name)
	cProp := C.CString(prop)
	cValue := C.uint(value)

	defer C.free(unsafe.Pointer(cName))
	defer C.free(unsafe.Pointer(cProp))

	C.gstreamer_send_set_property_uint(p.pipeline, cName, cProp, cValue)
}

func (p *Pipeline) getPropertyUint(name string, prop string) uint {
	cName := C.CString(name)
	cProp := C.CString(prop)

	defer C.free(unsafe.Pointer(cName))
	defer C.free(unsafe.Pointer(cProp))

	return uint(C.gstreamer_get_property_uint(p.pipeline, cName, cProp))
}

func (p *Pipeline) SSRC() uint {
	return p.getPropertyUint(p.payloder, "ssrc")
}

func (p *Pipeline) SetSSRC(ssrc uint) {
	p.setPropertyUint(p.payloder, "ssrc", ssrc)
}

func (p *Pipeline) SetBitRate(bitrate uint) {
	value := bitrate
	prop := "bitrate"
	switch p.codec {
	case "vp8", "vp9":
		prop = "target-bitrate"
	case "h264":
		value = value / 1000
	}
	//previous := p.getPropertyUint("encoder", prop)
	p.setPropertyUint("encoder", prop, value)
	//next := p.getPropertyUint("encoder", prop)
	//fmt.Printf("updating bitrate for codec %v: %v => %v (got %v, value=%v)\n", p.codec, previous, next, bitrate, value)
}

func (p *Pipeline) GetBitrate() uint {
	prop := "bitrate"
	if p.codec == "vp8" || p.codec == "vp9" {
		prop = "target-bitrate"
	}
	return p.getPropertyUint(p.codec, prop)
}

//export goHandlePipelineBuffer
func goHandlePipelineBuffer(buffer unsafe.Pointer, bufferLen C.int, pipelineID C.int) {
	pipelinesLock.Lock()
	pipeline, ok := pipelines[int(pipelineID)]
	pipelinesLock.Unlock()
	defer C.free(buffer)
	if !ok {
		log.Printf("no pipeline with ID %v, discarding buffer", int(pipelineID))
		return
	}

	bs := C.GoBytes(buffer, bufferLen)
	n, err := io.Copy(pipeline.writer, bytes.NewReader(bs))
	if err != nil {
		log.Printf("failed to write %v bytes to writer: %v", n, err)
	}
	if n != int64(bufferLen) {
		log.Printf("different buffer size written: %v vs. %v", n, bufferLen)
	}
}

func (p *Pipeline) Close() error {
	p.Stop()
	p.Destroy()
	p.writer.Close()
	return nil
}
