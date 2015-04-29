//
//  StreamDeviceSource.hh
//  streamer-test-8
//
//  Created by Valerii Lider on 4/29/15.
//  Copyright (c) 2015 Valerii Lider. All rights reserved.
//

#ifndef _STREAM_DEVICE_SOURCE_HH
#define _STREAM_DEVICE_SOURCE_HH

#ifndef _FRAMED_SOURCE_HH
#include "FramedSource.hh"
#endif

#include <WAVAudioFileSource.hh>
#include <uLawAudioFilter.hh>
#include <SimpleRTPSink.hh>

class StreamDeviceSource;
extern StreamDeviceSource *globalStreamDeviceSource;

// The following class can be used to define specific encoder parameters
class StreamDeviceParameters {
public:
    StreamDeviceParameters()
    : fAudioFormat(WA_PCM),
    fBitsPerSample(16),
    fSamplingFrequency(44100),
    fNumChannels(1) {
        
    }
    
    ~StreamDeviceParameters() {
        
    }
    
public:
    unsigned char fAudioFormat;
    unsigned char fBitsPerSample;
    unsigned fSamplingFrequency;
    unsigned fNumChannels;
};

class StreamDeviceSource: public FramedSource {
public:
  static StreamDeviceSource* createNew(UsageEnvironment& env,
				 StreamDeviceParameters params);

public:
  static EventTriggerId eventTriggerId;
  // Note that this is defined here to be a static class variable, because this code is intended to illustrate how to
  // encapsulate a *single* device - not a set of devices.
  // You can, however, redefine this to be a non-static member variable.

protected:
  StreamDeviceSource(UsageEnvironment& env, StreamDeviceParameters params);
  // called only by createNew(), or by subclass constructors
  virtual ~StreamDeviceSource();

private:
    Boolean isCurrentlyAwaitingData() const;
  // redefined virtual functions:
  virtual void doGetNextFrame();
  //virtual void doStopGettingFrames(); // optional

private:
  static void deliverFrame0(void* clientData);
  void deliverFrame();

private:
  static unsigned referenceCount; // used to count how many instances of this class currently exist
  StreamDeviceParameters fParams;
};

#endif
