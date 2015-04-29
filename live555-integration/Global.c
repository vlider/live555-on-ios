//
//  Global.c
//  streamer-test-8
//
//  Created by Valerii Lider on 4/29/15.
//  Copyright (c) 2015 Valerii Lider. All rights reserved.
//

#include "Global.h"

/** global circular buffer for storing mic input data
 */
TPCircularBuffer globalCircularBuffer;

/** initializes circular buffer with specified size
 function may fail to allocate memory, and in this case it will return false
 */
bool initializeGlobalCircularBufferWithSize(uint32_t size) {
    return TPCircularBufferInit(&globalCircularBuffer, size);
}

/** appends new bytes into to circular buffer
 */
void appendBytesIntoGlobalCircularBuffer(const void *bytes, int32_t length) {
    
    int32_t availableBytes = 0;
    void *buffer = TPCircularBufferHead(&globalCircularBuffer, &availableBytes);
    if (NULL != buffer && availableBytes >= length) {
        memcpy(buffer, bytes, length);
        TPCircularBufferProduce(&globalCircularBuffer, length);
    }
}

/** copies bytes from circulr buffer and mark them as ready for overwrite
 */
void discardBytesFromGlobalCircularBuffer(void **bytes, int32_t *availableBytes) {
    
    int32_t available = 0;
    void *buffer = TPCircularBufferTail(&globalCircularBuffer, &available);
    if (0 < available && NULL != buffer) {
        int32_t min = (*availableBytes > available) ? available : *availableBytes;
        memcpy(*bytes, buffer, min);
        TPCircularBufferConsume(&globalCircularBuffer, min);
    }
}