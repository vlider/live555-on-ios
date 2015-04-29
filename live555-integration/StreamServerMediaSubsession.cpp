//
//  StreamServerMediaSubsession.cpp
//  streamer-test-8
//
//  Created by Valerii Lider on 4/29/15.
//  Copyright (c) 2015 Valerii Lider. All rights reserved.
//

#include "StreamServerMediaSubsession.hh"

#include "StreamDeviceSource.hh"

StreamServerMediaSubsession::StreamServerMediaSubsession(UsageEnvironment& env, Boolean reuseFirstSource)
  : OnDemandServerMediaSubsession(env, reuseFirstSource) {
}

StreamServerMediaSubsession* StreamServerMediaSubsession::createNew(UsageEnvironment& env, Boolean reuseFirstSource) {
    return new StreamServerMediaSubsession(env, reuseFirstSource);
}

StreamServerMediaSubsession::~StreamServerMediaSubsession() {
}


FramedSource* StreamServerMediaSubsession::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate) {
    
    FramedSource* resultSource = NULL;
    do {
        StreamDeviceParameters parameters = StreamDeviceParameters();
        StreamDeviceSource* wavSource = StreamDeviceSource::createNew(envir(), parameters);
        if (wavSource == NULL) break;
        
        // Get attributes of the audio source:
        
        parameters.fAudioFormat = WA_PCM;
        parameters.fBitsPerSample = 16;
        // We handle only 4,8,16,20,24 bits-per-sample audio:
        if (parameters.fBitsPerSample%4 != 0 || parameters.fBitsPerSample < 4 || parameters.fBitsPerSample > 24 || parameters.fBitsPerSample == 12) {
            envir() << "The input file contains " << parameters.fBitsPerSample << " bit-per-sample audio, which we don't handle\n";
            break;
        }
        parameters.fSamplingFrequency = 44100.00;
        parameters.fNumChannels = 1;
        unsigned bitsPerSecond = parameters.fSamplingFrequency * parameters.fBitsPerSample * parameters.fNumChannels;
        
        // Add in any filter necessary to transform the data prior to streaming:
        resultSource = wavSource; // by default
        if (parameters.fAudioFormat == WA_PCM) {
            if (parameters.fBitsPerSample == 16) {
                // Note that samples in the WAV audio file are in little-endian order.
                // Add a filter that converts from little-endian to network (big-endian) order:
                resultSource = EndianSwap16::createNew(envir(), wavSource);
            } else if (parameters.fBitsPerSample == 20 || parameters.fBitsPerSample == 24) {
                // Add a filter that converts from little-endian to network (big-endian) order:
                resultSource = EndianSwap24::createNew(envir(), wavSource);
            }
        }
        
        estBitrate = (bitsPerSecond+500)/1000; // kbps
        return resultSource;
    } while (0);
    
    return NULL;
}

RTPSink* StreamServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource) {
    do {
        StreamDeviceParameters parameters = StreamDeviceParameters();
        
        char const* mimeType;
        unsigned char payloadFormatCode = rtpPayloadTypeIfDynamic; // by default, unless a static RTP payload type can be used
        if (parameters.fAudioFormat == WA_PCM) {
            if (parameters.fBitsPerSample == 16) {
                
                mimeType = "L16";
                if (parameters.fSamplingFrequency == 44100 && parameters.fNumChannels == 2) {
                    payloadFormatCode = 10; // a static RTP payload type
                } else if (parameters.fSamplingFrequency == 44100 && parameters.fNumChannels == 1) {
                    payloadFormatCode = 11; // a static RTP payload type
                }
            } else if (parameters.fBitsPerSample == 20) {
                mimeType = "L20";
            } else if (parameters.fBitsPerSample == 24) {
                mimeType = "L24";
            } else { // fBitsPerSample == 8 (we assume that fBitsPerSample == 4 is only for WA_IMA_ADPCM)
                mimeType = "L8";
            }
        } else if (parameters.fAudioFormat == WA_PCMU) {
            mimeType = "PCMU";
            if (parameters.fSamplingFrequency == 8000 && parameters.fNumChannels == 1) {
                payloadFormatCode = 0; // a static RTP payload type
            }
        } else if (parameters.fAudioFormat == WA_PCMA) {
            mimeType = "PCMA";
            if (parameters.fSamplingFrequency == 8000 && parameters.fNumChannels == 1) {
                payloadFormatCode = 8; // a static RTP payload type
            }
        } else if (parameters.fAudioFormat == WA_IMA_ADPCM) {
            mimeType = "DVI4";
            // Use a static payload type, if one is defined:
            if (parameters.fNumChannels == 1) {
                if (parameters.fSamplingFrequency == 8000) {
                    payloadFormatCode = 5; // a static RTP payload type
                } else if (parameters.fSamplingFrequency == 16000) {
                    payloadFormatCode = 6; // a static RTP payload type
                } else if (parameters.fSamplingFrequency == 11025) {
                    payloadFormatCode = 16; // a static RTP payload type
                } else if (parameters.fSamplingFrequency == 22050) {
                    payloadFormatCode = 17; // a static RTP payload type
                }
            }
        } else { //unknown format
            break;
        }
        
        return SimpleRTPSink::createNew(envir(), rtpGroupsock,
                                        payloadFormatCode, parameters.fSamplingFrequency,
                                        "audio", mimeType, parameters.fNumChannels);
    } while (0);
    
    // An error occurred:
    return NULL;
}