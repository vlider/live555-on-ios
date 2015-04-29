//
//  StreamDeviceSource.cpp
//  streamer-test-8
//
//  Created by Valerii Lider on 4/29/15.
//  Copyright (c) 2015 Valerii Lider. All rights reserved.
//

#include "StreamDeviceSource.hh"
#include <GroupsockHelper.hh> // for "gettimeofday()"

#include "Global.h"

StreamDeviceSource *globalStreamDeviceSource = NULL;

StreamDeviceSource*
StreamDeviceSource::createNew(UsageEnvironment& env, StreamDeviceParameters params) {
  return new StreamDeviceSource(env, params);
}

EventTriggerId StreamDeviceSource::eventTriggerId = 0;

unsigned StreamDeviceSource::referenceCount = 0;

StreamDeviceSource::StreamDeviceSource(UsageEnvironment& env, StreamDeviceParameters params)
  : FramedSource(env), fParams(params) {
  if (referenceCount == 0) {
    // Any global initialization of the device would be done here:
      globalStreamDeviceSource = this;
  }
  ++referenceCount;

  // Any instance-specific initialization of the device would be done here:

  // We arrange here for our "deliverFrame" member function to be called
  // whenever the next frame of data becomes available from the device.
  //
  // If the device can be accessed as a readable socket, then one easy way to do this is using a call to
  //     envir().taskScheduler().turnOnBackgroundReadHandling( ... )
  // (See examples of this call in the "liveMedia" directory.)
  //
  // If, however, the device *cannot* be accessed as a readable socket, then instead we can implement it using 'event triggers':
  // Create an 'event trigger' for this device (if it hasn't already been done):
  if (eventTriggerId == 0) {
    eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
  }
}

StreamDeviceSource::~StreamDeviceSource() {
  // Any instance-specific 'destruction' (i.e., resetting) of the device would be done here:

  --referenceCount;
  if (referenceCount == 0) {
    // Any global 'destruction' (i.e., resetting) of the device would be done here:
      
      globalStreamDeviceSource = NULL;
      
    // Reclaim our 'event trigger'
    envir().taskScheduler().deleteEventTrigger(eventTriggerId);
    eventTriggerId = 0;
  }
}

void StreamDeviceSource::doGetNextFrame() {
  // This function is called (by our 'downstream' object) when it asks for new data.

    // If a new frame of data is immediately available to be delivered, then do this now:
    if (0 != globalCircularBuffer.fillCount) {
        deliverFrame();
    }
}

Boolean StreamDeviceSource::isCurrentlyAwaitingData() const {return (0 == globalCircularBuffer.fillCount);}

void StreamDeviceSource::deliverFrame0(void* clientData) {
    globalStreamDeviceSource->deliverFrame();
}

void StreamDeviceSource::deliverFrame() {
  // This function is called when new frame data is available from the device.
  // We deliver this data by copying it to the 'downstream' object, using the following parameters (class members):
  // 'in' parameters (these should *not* be modified by this function):
  //     fTo: The frame data is copied to this address.
  //         (Note that the variable "fTo" is *not* modified.  Instead,
  //          the frame data is copied to the address pointed to by "fTo".)
  //     fMaxSize: This is the maximum number of bytes that can be copied
  //         (If the actual frame is larger than this, then it should
  //          be truncated, and "fNumTruncatedBytes" set accordingly.)
  // 'out' parameters (these are modified by this function):
  //     fFrameSize: Should be set to the delivered frame size (<= fMaxSize).
  //     fNumTruncatedBytes: Should be set iff the delivered frame would have been
  //         bigger than "fMaxSize", in which case it's set to the number of bytes
  //         that have been omitted.
  //     fPresentationTime: Should be set to the frame's presentation time
  //         (seconds, microseconds).  This time must be aligned with 'wall-clock time' - i.e., the time that you would get
  //         by calling "gettimeofday()".
  //     fDurationInMicroseconds: Should be set to the frame's duration, if known.
  //         If, however, the device is a 'live source' (e.g., encoded from a camera or microphone), then we probably don't need
  //         to set this variable, because - in this case - data will never arrive 'early'.
  // Note the code below.
    
  if (isCurrentlyAwaitingData()) return; // we're not ready for the data yet

    // allocate temporary buffer
    static void *buffer = NULL;
    if (NULL == buffer) {
        buffer = malloc(8192);
    }
    // by specifying length = 8192 we telling to discardBytesFromGlobalCircularBuffer that it may copy maximum 8192 bytes
    int32_t length  = 8192;
    discardBytesFromGlobalCircularBuffer(&buffer, &length);
    
    u_int8_t* newFrameDataStart = (u_int8_t*)buffer;
    unsigned newFrameSize = length;

  // Deliver the data here:
  if (newFrameSize > fMaxSize) {
    fFrameSize = fMaxSize;
    fNumTruncatedBytes = newFrameSize - fMaxSize;
  } else {
    fFrameSize = newFrameSize;
  }
  gettimeofday(&fPresentationTime, NULL); // If you have a more accurate time - e.g., from an encoder - then use that instead.
  // If the device is *not* a 'live source' (e.g., it comes instead from a file or buffer), then set "fDurationInMicroseconds" here.
  memmove(fTo, newFrameDataStart, fFrameSize);

    // After delivering the data, inform the reader that it is now available:
    nextTask() = envir().taskScheduler().scheduleDelayedTask(0, (TaskFunc*)FramedSource::afterGetting, this);
}