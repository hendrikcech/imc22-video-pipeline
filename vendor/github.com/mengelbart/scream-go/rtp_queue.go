package scream

import "C"
import (
	"sync"
)

// RTPQueue implements a simple RTP packet queue. One RTPQueue should be used
// per SSRC stream.
type RTPQueue interface {
	// SizeOfNextRTP returns the size of the next item in the queue.
	SizeOfNextRTP() int

	// SeqNrOfNextRTP returns the RTP sequence number of the next item in the queue
	SeqNrOfNextRTP() uint16

	// BytesInQueue returns the total number of bytes in the queue, i.e. the
	// sum of the sizes of all items in the queue.
	BytesInQueue() int

	// SizeOfQueue returns the number of items in the queue.
	SizeOfQueue() int

	// GetDelay returns the delay of the last item in the queue.
	// ts is given in seconds.
	GetDelay(ts float64) float64

	// GetSizeOfLastFrame returns the size of the latest pushed item.
	GetSizeOfLastFrame() int

	// Clear empties the queue.
	Clear()
}

var srcPipelinesLock sync.Mutex
var rtpQueues = map[uint32]RTPQueue{}

//export goClear
func goClear(id C.int) {
	srcPipelinesLock.Lock()
	defer srcPipelinesLock.Unlock()
	rtpQueues[uint32(id)].Clear()
}

//export goSizeOfNextRtp
func goSizeOfNextRtp(id C.int) C.int {
	srcPipelinesLock.Lock()
	defer srcPipelinesLock.Unlock()
	return C.int(rtpQueues[uint32(id)].SizeOfNextRTP())
}

//export goSeqNrOfNextRtp
func goSeqNrOfNextRtp(id C.int) C.int {
	srcPipelinesLock.Lock()
	defer srcPipelinesLock.Unlock()
	return C.int(rtpQueues[uint32(id)].SeqNrOfNextRTP())
}

//export goBytesInQueue
func goBytesInQueue(id C.int) C.int {
	srcPipelinesLock.Lock()
	defer srcPipelinesLock.Unlock()
	return C.int(rtpQueues[uint32(id)].BytesInQueue())
}

//export goSizeOfQueue
func goSizeOfQueue(id C.int) C.int {
	srcPipelinesLock.Lock()
	defer srcPipelinesLock.Unlock()
	return C.int(rtpQueues[uint32(id)].SizeOfQueue())
}

//export goGetDelay
func goGetDelay(id C.int, currTs C.float) C.float {
	srcPipelinesLock.Lock()
	defer srcPipelinesLock.Unlock()
	return C.float(rtpQueues[uint32(id)].GetDelay(float64(currTs)))
}

//export goGetSizeOfLastFrame
func goGetSizeOfLastFrame(id C.int) C.int {
	srcPipelinesLock.Lock()
	defer srcPipelinesLock.Unlock()
	return C.int(rtpQueues[uint32(id)].GetSizeOfLastFrame())
}
