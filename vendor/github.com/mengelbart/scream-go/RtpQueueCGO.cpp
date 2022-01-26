// Copyright (c) 2020 Mathis Engelbart All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "RtpQueueCGO.h"

RtpQueueCGO::RtpQueueCGO(int id) {
    this->id = id;
}

void RtpQueueCGO::clear() {
    goClear(this->id);
}

int RtpQueueCGO::sizeOfNextRtp() {
    return goSizeOfNextRtp(this->id);
}

int RtpQueueCGO::seqNrOfNextRtp() {
    return goSeqNrOfNextRtp(this->id);
}

int RtpQueueCGO::bytesInQueue() {
    return goBytesInQueue(this->id);
}

int RtpQueueCGO::sizeOfQueue() {
    return goSizeOfQueue(this->id);
}

float RtpQueueCGO::getDelay(float currTs) {
    return goGetDelay(this->id, currTs);
}

int RtpQueueCGO::getSizeOfLastFrame() {
    return goGetSizeOfLastFrame(this->id);
}

