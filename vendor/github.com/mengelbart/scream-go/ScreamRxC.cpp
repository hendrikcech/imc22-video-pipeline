// Copyright (c) 2020 Mathis Engelbart All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <cstdlib>

#include "ScreamRx.h"
#include "ScreamRxC.h"

ScreamRxC* ScreamRxInit(unsigned int ssrc) {
    ScreamRx* ret = new ScreamRx(ssrc);
    return (void**) ret;
}

void ScreamRxFree(ScreamRxC* s) {
    ScreamRx* srx = (ScreamRx*) s;
    delete srx;
}

void ScreamRxReceive(ScreamRxC* s,
        unsigned int time_ntp,
        void* rtpPacket,
        unsigned int ssrc,
        int size,
        unsigned int seqNr,
        unsigned char ceBits
    ){

    ScreamRx* srx = (ScreamRx*) s;
    srx->receive(time_ntp, rtpPacket, ssrc, size, seqNr, ceBits);
}

bool ScreamRxIsFeedback(ScreamRxC* s, unsigned int time_ntp) {
    ScreamRx* srx = (ScreamRx*) s;
    return srx->isFeedback(time_ntp);
}

Feedback* ScreamRxGetFeedback(ScreamRxC* s, unsigned int time_ntp, bool isMark, unsigned char *buf) {
    ScreamRx* srx = (ScreamRx*) s;
    Feedback *fb = (Feedback*)malloc(sizeof(Feedback));
    fb->result = srx->createStandardizedFeedback(time_ntp, isMark, buf, fb->size);
    return fb;
}

bool ScreamRxGetFeedbackResult(Feedback* fb) {
    return fb->result;
}

int ScreamRxGetFeedbackSize(Feedback* fb) {
    return fb->size;
}
