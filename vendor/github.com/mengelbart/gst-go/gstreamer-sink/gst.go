package gst

/*
#cgo pkg-config: gstreamer-1.0 gstreamer-app-1.0

#include "gst.h"

*/
import "C"
import (
	"errors"
	"sync"
	"log"
	"unsafe"
)

var ErrUnknownCodec = errors.New("unknown codec")

type FpsMeasurement struct {
	FpsCurrent float64
	FpsAverage float64
	LossRate   float64
}

// StartMainLoop starts GLib's main loop
// It needs to be called from the process' main thread
// Because many gstreamer plugins require access to the main thread
// See: https://golang.org/pkg/runtime/#LockOSThread
func StartMainLoop() {
	C.gstreamer_receive_start_mainloop()
}

var pipelines = map[int]*Pipeline{}
var pipelinesLock sync.Mutex

type Pipeline struct {
	id          int
	pipeline    *C.GstElement
	pipelineStr string
	fpsChan   chan FpsMeasurement
}

func NewPipeline(codecName, dst string) (*Pipeline, error) {
	pipelineStr := "appsrc name=src ! application/x-rtp"

	switch codecName {
	case "vp8":
		pipelineStr += ", encoding-name=VP8-DRAFT-IETF-01 ! rtpjitterbuffer ! rtpvp8depay ! decodebin ! videoconvert ! " + dst

	case "vp9":
		pipelineStr += ", encoding-name=VP9-DRAFT-IETF-01 ! rtpjitterbuffer ! rtpvp9depay ! decodebin ! videoconvert ! " + dst

	case "h264":
		pipelineStr += " ! rtpjitterbuffer ! rtph264depay ! decodebin ! videoconvert ! " + dst

	case "h264-save":
		pipelineStr += " ! rtpjitterbuffer ! rtph264depay ! tee name=t ! queue ! h264parse ! matroskamux ! filesink location=/tmp/se-video.mkv t. ! queue ! decodebin ! videoconvert ! " + dst

	default:
		return nil, ErrUnknownCodec
	}

	pipelineStrUnsafe := C.CString(pipelineStr)
	defer C.free(unsafe.Pointer(pipelineStrUnsafe))
	sp := &Pipeline{
		id: len(pipelines),
		pipeline:    C.gstreamer_receive_create_pipeline(pipelineStrUnsafe),
		pipelineStr: pipelineStr,
	}
	pipelines[sp.id] = sp
	return sp, nil
}

func (p *Pipeline) String() string {
	return p.pipelineStr
}

// Start starts the GStreamer Pipeline
func (p *Pipeline) Start() {
	C.gstreamer_receive_start_pipeline(p.pipeline)
}

func (p *Pipeline) Stop() {
	C.gstreamer_receive_stop_pipeline(p.pipeline)
}

func (p *Pipeline) Destroy() {
	C.gstreamer_receive_destroy_pipeline(p.pipeline)
}

var eosHandler func()

func HandleSinkEOS(handler func()) {
	eosHandler = handler
}

//export goHandleReceiveEOS
func goHandleReceiveEOS() {
	if eosHandler != nil {
		eosHandler()
	}
}

// Push pushes a buffer on the appsrc of the GStreamer Pipeline
func (p *Pipeline) Write(buffer []byte) (n int, err error) {
	n = len(buffer)
	b := C.CBytes(buffer)
	defer C.free(b)
	C.gstreamer_receive_push_buffer(p.pipeline, b, C.int(len(buffer)))
	return
}

func (p *Pipeline) Close() error {
	p.Stop()
	p.Destroy()
	close(p.fpsChan)
	return nil
}

func (p *Pipeline) ConnectFpsSignal(elementName string) (chan FpsMeasurement) {
	if p.fpsChan != nil {
		panic("Can connect fps signal only once")
	}
	p.fpsChan = make(chan FpsMeasurement)

	cElementName := C.CString(elementName)
	defer C.free(unsafe.Pointer(cElementName))

	C.gstreamer_connect_fps_signal(p.pipeline, cElementName, C.int(p.id))

	return p.fpsChan
}

//export goOnFpsSignal
func goOnFpsSignal(current_fps, loss_rate, average_fps C.double, pipelineID C.int) {
	pipelinesLock.Lock()
	pipeline, ok := pipelines[int(pipelineID)]
	pipelinesLock.Unlock()
	if !ok {
		log.Printf("no pipeline with ID %v, discarding fps signal", int(pipelineID))
		return
	}
	pipeline.fpsChan <- FpsMeasurement{float64(current_fps), float64(average_fps), float64(loss_rate)}
}
