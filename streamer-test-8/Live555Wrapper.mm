//
//  Live555Wrapper.m
//  streamer-test-8
//
//  Created by Valerii Lider on 4/29/15.
//  Copyright (c) 2015 Valerii Lider. All rights reserved.
//

#import "Live555Wrapper.h"

#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>

#include "StreamServerMediaSubsession.hh"
#include "Global.h"
#include "StreamDeviceSource.hh"

UsageEnvironment* env;

// To make the second and subsequent client for each stream reuse the same
// input stream as the first client (rather than playing the file from the
// start for each client), change the following "False" to "True":
Boolean reuseFirstSource = True;

@implementation Live555Wrapper

+ (instancetype)sharedInstance {
    static Live555Wrapper *_gSharedWrapper = NULL;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _gSharedWrapper = [Live555Wrapper new];
    });
    return _gSharedWrapper;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        initializeGlobalCircularBufferWithSize(8192 * 10);
    }
    return self;
}


- (void)publish {
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0), ^{
        
        TaskScheduler* scheduler = BasicTaskScheduler::createNew();
        env = BasicUsageEnvironment::createNew(*scheduler);
        
        UserAuthenticationDatabase* authDB = NULL;
#ifdef ACCESS_CONTROL
        // To implement client access control to the RTSP server, do the following:
        authDB = new UserAuthenticationDatabase;
        authDB->addUserRecord("username1", "password1"); // replace these with real strings
        // Repeat the above with each <username>, <password> that you wish to allow
        // access to the server.
#endif
        
        // Create the RTSP server:
        RTSPServer* rtspServer = RTSPServer::createNew(*env, 8554, authDB);
        if (rtspServer == NULL) {
            *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
            exit(1);
        }
        
        char const* descriptionString
        = "Session streamed by \"testOnDemandRTSPServer\"";
        
        // Set up each of the possible streams that can be served by the
        // RTSP server.  Each such stream is implemented using a
        // "ServerMediaSession" object, plus one or more
        // "ServerMediaSubsession" objects for each audio/video substream.
        
        // A WAV audio stream:
        {
            char const* streamName = "stream1";
            ServerMediaSession* sms
            = ServerMediaSession::createNew(*env, streamName, streamName,
                                            descriptionString);
            
            sms->addSubsession(StreamServerMediaSubsession::createNew(*env, reuseFirstSource));
            rtspServer->addServerMediaSession(sms);
            
            [self anounceStream:rtspServer withSession:sms withName:[NSString stringWithUTF8String:streamName]];
        }
        
        // Also, attempt to create a HTTP server for RTSP-over-HTTP tunneling.
        // Try first with the default HTTP port (80), and then with the alternative HTTP
        // port numbers (8000 and 8080).
        
        if (rtspServer->setUpTunnelingOverHTTP(80) || rtspServer->setUpTunnelingOverHTTP(8000) || rtspServer->setUpTunnelingOverHTTP(8080)) {
            *env << "\n(We use port " << rtspServer->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling.)\n";
        } else {
            *env << "\n(RTSP-over-HTTP tunneling is not available.)\n";
        }
        
        env->taskScheduler().doEventLoop(); // does not return
    });
}

- (void)newFramesAvailable {
    env->taskScheduler().triggerEvent(StreamDeviceSource::eventTriggerId);
}

- (void)anounceStream:(RTSPServer *)server withSession:(ServerMediaSession *)session withName:(NSString *)sessionName {
    
    char* url = server->rtspURL(session);
    UsageEnvironment& env = server->envir();
    env << "\n\"" << sessionName.UTF8String << "\" stream \"";
    env << "Play this stream using the URL \"" << url << "\"\n";
    delete[] url;
}

@end
