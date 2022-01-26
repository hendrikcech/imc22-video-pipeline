/*
 * Copyright (c) 2020 Mathis Engelbart All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#ifndef SCREAM_RX_H
#define SCREAM_RX_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

    typedef struct {
        int size;
        bool result;
    } Feedback;

    typedef void* ScreamRxC;
    ScreamRxC* ScreamRxInit(unsigned int ssrc);
    void ScreamRxFree(ScreamRxC*);

    void ScreamRxReceive(ScreamRxC*, unsigned int, void*, unsigned int, int, unsigned int, unsigned char);
    bool ScreamRxIsFeedback(ScreamRxC*, unsigned int);
    Feedback* ScreamRxGetFeedback(ScreamRxC*, unsigned int, bool, unsigned char *buf);

    bool ScreamRxGetFeedbackResult(Feedback*);
    int ScreamRxGetFeedbackSize(Feedback*);
    unsigned char* ScreamRxGetFeedbackBuffer(Feedback*);

#ifdef __cplusplus
}
#endif

#endif
