// Copyright (c) 2015, Ericsson AB. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice, this
// list of conditions and the following disclaimer in the documentation and/or other
// materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
// OF SUCH DAMAGE.

#include "RtpQueue.h"
#include <iostream>
#include <string.h>
using namespace std;
/*
* Implements a simple RTP packet queue
*/

RtpQueueItem::RtpQueueItem() {
    used = false;
    size = 0;
    seqNr = 0;
}


RtpQueue::RtpQueue() {
    for (int n=0; n < kRtpQueueSize; n++) {
        items[n] = new RtpQueueItem();
    }
    head = -1;
    tail = 0;
    nItems = 0;
    sizeOfLastFrame = 0;
    bytesInQueue_ = 0;
    sizeOfQueue_ = 0;
    sizeOfNextRtp_ = -1;
}

void RtpQueue::push(void *rtpPacket, int size, unsigned short seqNr, float ts) {
    int ix = head+1;
    if (ix == kRtpQueueSize) ix = 0;
    if (items[ix]->used) {
      /*
      * RTP queue is full, do a drop tail i.e ignore new RTP packets
      */
      return;
    }
    head = ix;
    items[head]->seqNr = seqNr;
    items[head]->size = size;
    items[head]->ts = ts;
    items[head]->used = true;
    bytesInQueue_ += size;
    sizeOfQueue_ += 1;
    memcpy(items[head]->packet, rtpPacket, size);
    computeSizeOfNextRtp();
}

bool RtpQueue::pop(void *rtpPacket, int& size, unsigned short& seqNr) {
    if (items[tail]->used == false) {
        return false;
        sizeOfNextRtp_ = -1;
    } else {
        size = items[tail]->size;
        memcpy(rtpPacket,items[tail]->packet,size);
        seqNr = items[tail]->seqNr;
        items[tail]->used = false;
        tail++; if (tail == kRtpQueueSize) tail = 0;
        bytesInQueue_ -= size;
        sizeOfQueue_ -= 1;
        computeSizeOfNextRtp();
        return true;
    }
}

void RtpQueue::computeSizeOfNextRtp() {
    if (!items[tail]->used) {
        sizeOfNextRtp_ = - 1;
    } else {
        sizeOfNextRtp_ = items[tail]->size;
    }
}

int RtpQueue::sizeOfNextRtp() {
    return sizeOfNextRtp_;
}

int RtpQueue::seqNrOfNextRtp() {
    if (!items[tail]->used) {
        return -1;
    } else {
        return items[tail]->seqNr;
    }
}

int RtpQueue::bytesInQueue() {
    return bytesInQueue_;
}

int RtpQueue::sizeOfQueue() {
    return sizeOfQueue_;
}

float RtpQueue::getDelay(float currTs) {
    if (items[tail]->used == false) {
        return 0;
    } else {
        return currTs-items[tail]->ts;
    }
}

bool RtpQueue::sendPacket(void *rtpPacket, int& size, unsigned short& seqNr) {
    if (sizeOfQueue() > 0) {
        pop(rtpPacket, size, seqNr);
        return true;
    }
    return false;
}

void RtpQueue::clear() {
    for (int n=0; n < kRtpQueueSize; n++) {
        items[n]->used = false;
    }
    head = -1;
    tail = 0;
    nItems = 0;
    bytesInQueue_ = 0;
    sizeOfQueue_ = 0;
    sizeOfNextRtp_ = -1;
}
