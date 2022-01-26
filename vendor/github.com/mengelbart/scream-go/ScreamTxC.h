/*
 * Copyright (c) 2020 Mathis Engelbart All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#ifndef SCREAM_TX_H
#define SCREAM_TX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

    typedef void* RtpQueueIfaceC;
    RtpQueueIfaceC* RtpQueueIfaceInit(int);
    void RtpQueueIfaceFree(RtpQueueIfaceC*);

    typedef void* ScreamTxC;
    ScreamTxC* ScreamTxInit(void);
    void ScreamTxFree(ScreamTxC*);

    void ScreamTxRegisterNewStream(ScreamTxC*,
            RtpQueueIfaceC*,
            unsigned int,
            float,
            float,
            float,
            float);
    void ScreamTxNewMediaFrame(ScreamTxC*, unsigned int, unsigned int, int);
    float ScreamTxIsOkToTransmit(ScreamTxC*, unsigned int, unsigned int);
    float ScreamTxAddTransmitted(ScreamTxC*, unsigned int, unsigned int, int, unsigned int, bool);
    void ScreamTxIncomingStdFeedback(ScreamTxC*,
        unsigned int,
        void*,
        int size);
    float ScreamTxGetTargetBitrate(ScreamTxC*, unsigned int);
    char* ScreamTxGetStatistics(ScreamTxC*, unsigned int);

#ifdef __cplusplus
}
#endif

#endif
