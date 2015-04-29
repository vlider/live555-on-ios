//
//  Global.h
//  streamer-test-8
//
//  Created by Valerii Lider on 4/29/15.
//  Copyright (c) 2015 Valerii Lider. All rights reserved.
//

#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <sys/types.h>
#include "TPCircularBuffer.h"

#ifdef __cplusplus
extern "C" {
#endif
    
extern TPCircularBuffer globalCircularBuffer;
    
bool initializeGlobalCircularBufferWithSize(uint32_t size);
void appendBytesIntoGlobalCircularBuffer(const void *bytes, int32_t length);
void discardBytesFromGlobalCircularBuffer(void **bytes, int32_t *length);

#ifdef __cplusplus
}
#endif

#endif//__GLOBAL_H__
