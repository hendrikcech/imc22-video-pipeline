/*
 * Copyright (c) 2015, Ericsson AB. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */

#ifndef RTP_QUEUE
#define RTP_QUEUE

/*
* Implements a simple RTP packet queue, one RTP queue
* per stream {SSRC,PT}
*/

class RtpQueueIface {
public:
    virtual void clear() = 0;
    virtual int sizeOfNextRtp() = 0;
    virtual int seqNrOfNextRtp() = 0;
    virtual int bytesInQueue() = 0; // Number of bytes in queue
    virtual int sizeOfQueue() = 0;  // Number of items in queue
    virtual float getDelay(float currTs) = 0;
    virtual int getSizeOfLastFrame() = 0;
};

class RtpQueueItem {
public:
    RtpQueueItem();
    char packet[2000];
    int size;
    unsigned short seqNr;
    float ts;
    bool used;
};

const int kRtpQueueSize = 1024;
class RtpQueue : public RtpQueueIface {
public:
    RtpQueue();

    void push(void *rtpPacket, int size, unsigned short seqNr, float ts);
    bool pop(void *rtpPacket, int &size, unsigned short &seqNr);
    int sizeOfNextRtp();
    int seqNrOfNextRtp();
    int bytesInQueue(); // Number of bytes in queue
    int sizeOfQueue();  // Number of items in queue
    float getDelay(float currTs);
    bool sendPacket(void *rtpPacket, int &size, unsigned short &seqNr);
    void clear();
    int getSizeOfLastFrame() {return sizeOfLastFrame;};
    void setSizeOfLastFrame(int sz) {sizeOfLastFrame=sz;};
    void computeSizeOfNextRtp();

    RtpQueueItem *items[kRtpQueueSize];
    int head; // Pointer to last inserted item
    int tail; // Pointer to the oldest item
    int nItems;
    int sizeOfLastFrame;

    int bytesInQueue_;
    int sizeOfQueue_;
    int sizeOfNextRtp_;
};

#endif
