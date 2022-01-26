package quic

import (
	"fmt"
	"io"
	"sync"
	"time"

	"github.com/lucas-clemente/quic-go/internal/protocol"
	"github.com/lucas-clemente/quic-go/internal/utils"
	"github.com/lucas-clemente/quic-go/internal/wire"
)

type dgram struct {
	f        *wire.DatagramFrame
	dequeued chan struct{}
	enqueued time.Time
}
type datagramQueue struct {
	lock      sync.Mutex
	sendQueue []*dgram
	rcvQueue  chan []byte

	closeErr error
	closed   chan struct{}

	hasData func()

	dequeued chan struct{}

	logger utils.Logger

	delayLogger io.Writer
}

func newDatagramQueue(hasData func(), logger utils.Logger) *datagramQueue {
	//dw, err := os.Create("/log/delay.log")
	//if err != nil {
	//	panic(err)
	//}
	return &datagramQueue{
		lock:        sync.Mutex{},
		sendQueue:   []*dgram{},
		rcvQueue:    make(chan []byte, protocol.DatagramRcvQueueLen),
		closeErr:    nil,
		closed:      make(chan struct{}),
		hasData:     hasData,
		dequeued:    make(chan struct{}),
		logger:      logger,
		delayLogger: io.Discard,
	}
}

// AddAndWait queues a new DATAGRAM frame for sending.
// It blocks until the frame has been dequeued.
func (h *datagramQueue) AddAndWait(f *wire.DatagramFrame, sentCB func(error)) error {
	select {
	case <-h.closed:
		return h.closeErr
	default:
	}

	ch := make(chan struct{})

	h.lock.Lock()
	h.sendQueue = append(h.sendQueue, &dgram{
		f:        f,
		dequeued: ch,
		enqueued: time.Now(),
	})
	h.lock.Unlock()
	h.hasData()

	go func(cb func(error), c chan struct{}) {
		select {
		case <-c:
			if sentCB != nil {
				cb(nil)
			}
		case <-h.closed:
			if sentCB != nil {
				cb(h.closeErr)
			}
		}
	}(sentCB, ch)
	return nil
}

func (h *datagramQueue) CanAdd(maxSize protocol.ByteCount, version protocol.VersionNumber) bool {
	h.lock.Lock()
	defer h.lock.Unlock()

	if len(h.sendQueue) == 0 {
		return false
	}
	s := h.sendQueue[0].f.MaxDataLen(maxSize, version)
	return protocol.ByteCount(len(h.sendQueue[0].f.Data)) < s
}

// Get dequeues a DATAGRAM frame for sending.
func (h *datagramQueue) Get() *wire.DatagramFrame {
	h.lock.Lock()
	defer h.lock.Unlock()

	if len(h.sendQueue) == 0 {
		return nil
	}
	d := h.sendQueue[0]
	h.sendQueue = h.sendQueue[1:]
	d.dequeued <- struct{}{}
	now := time.Now()
	fmt.Fprintf(h.delayLogger, "%v, %v\n", now.UnixMilli(), now.Sub(d.enqueued).Milliseconds())
	return d.f
}

// HandleDatagramFrame handles a received DATAGRAM frame.
func (h *datagramQueue) HandleDatagramFrame(f *wire.DatagramFrame) {
	data := make([]byte, len(f.Data))
	copy(data, f.Data)
	select {
	case h.rcvQueue <- data:
	default:
		h.logger.Debugf("Discarding DATAGRAM frame (%d bytes payload)", len(f.Data))
	}
}

// Receive gets a received DATAGRAM frame.
func (h *datagramQueue) Receive() ([]byte, error) {
	select {
	case data := <-h.rcvQueue:
		return data, nil
	case <-h.closed:
		return nil, h.closeErr
	}
}

func (h *datagramQueue) CloseWithError(e error) {
	h.closeErr = e
	close(h.closed)
}
