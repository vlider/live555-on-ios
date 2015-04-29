//
//  StreamServerMediaSubsession.hh
//  streamer-test-8
//
//  Created by Valerii Lider on 4/29/15.
//  Copyright (c) 2015 Valerii Lider. All rights reserved.
//

#ifndef _STREAM_SERVER_MEDIA_SUBSESSION_HH
#define _STREAM_SERVER_MEDIA_SUBSESSION_HH

#ifndef _ON_DEMAND_SERVER_MEDIA_SUBSESSION_HH
#include "OnDemandServerMediaSubsession.hh"
#endif

class StreamServerMediaSubsession: public OnDemandServerMediaSubsession {
public:
    static StreamServerMediaSubsession* createNew(UsageEnvironment& env, Boolean reuseFirstSource);
protected:
    StreamServerMediaSubsession(UsageEnvironment& env, Boolean reuseFirstSource);
    virtual ~StreamServerMediaSubsession();
    
    virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);
};

#endif
