// Copyright (c) 2020 Mathis Engelbart All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package scream

/*
#cgo CPPFLAGS: -Wno-overflow -Wno-write-strings
#include "ScreamRxC.h"
#include <stdlib.h>
*/
import "C"
import (
	"unsafe"
)

// Rx implements the receiver side of SCReAM
type Rx struct {
	screamRx *C.ScreamRxC
}

// NewRx creates a new Rx instance. One Rx is created for each source SSRC
func NewRx(ssrc uint32) *Rx {
	return &Rx{
		screamRx: C.ScreamRxInit(C.uint(ssrc)),
	}
}

// Receive needs to be called each time an RTP packet is received
func (r *Rx) Receive(ntpTime uint64, ssrc uint32, size int, seqNr uint16, ceBits uint8) {
	C.ScreamRxReceive(r.screamRx, C.uint(ntpTime), nil, C.uint(ssrc), C.int(size), C.uint(seqNr), C.uchar(ceBits))
}

// IsFeedback returns TRUE if an RTP packet has been received and there is pending feedback
func (r *Rx) IsFeedback(ntpTime uint64) bool {
	return bool(C.ScreamRxIsFeedback(r.screamRx, C.uint(ntpTime)))
}

// CreateStandardizedFeedback creates a feedback packet according to
// https://tools.ietf.org/wg/avtcore/draft-ietf-avtcore-cc-feedback-message/
// Current implementation implements -02 version
// It is up to the wrapper application to prepend this RTCP with SR or RR when needed
func (r *Rx) CreateStandardizedFeedback(ntpTime uint64, isMark bool) (bool, []byte) {

	buf := make([]byte, 2048)
	ptr := unsafe.Pointer(&buf[0])
	ret := C.ScreamRxGetFeedback(r.screamRx, C.uint(ntpTime), C.bool(isMark), (*C.uchar)(ptr))
	defer C.free(unsafe.Pointer(ret))

	size := C.ScreamRxGetFeedbackSize(ret)
	result := make([]byte, size)
	copy(result, buf)

	return bool(C.ScreamRxGetFeedbackResult(ret)), result
}
