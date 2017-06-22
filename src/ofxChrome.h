//
//  ofxChrome.h
//  ofxofxChrome
//
//  Created by Oriol Ferrer Mesià on 21/06/2017.
//
//

#pragma once
#include "ofMain.h"
#include "ofxExternalProcess.h"
#include "ofxLibwebsockets.h"


class ofxChrome{

public:
	
	ofxChrome();
	~ofxChrome();

	//if "chromeBinaryPath" is not empty, it will launch chrome for you as an external process
	//and keep control of it (ie watchdog, etc)
	void setup(string chromeBinaryPath, string chromeRemoteDebugIP, int chromeRemoteDebugPort);

	void update(float dt);
	void draw(int x, int y);

	bool navigateToPage(string url, int & requestID);

	bool requestScreenshot(int & requestID);

	// websocket methods
	void onConnect( ofxLibwebsockets::Event& args );
	void onOpen( ofxLibwebsockets::Event& args );
	void onClose( ofxLibwebsockets::Event& args );
	void onIdle( ofxLibwebsockets::Event& args );
	void onMessage( ofxLibwebsockets::Event& args );
	void onBroadcast( ofxLibwebsockets::Event& args );

protected:

	enum State{
		UNINITED,
		LAUNCHING_CHROME,
		CONNECTING_TO_WS,
		READY_FOR_COMMANDS,
		FAILED_TO_START_CHROME
	};

	State state = UNINITED;

	ofxLibwebsockets::Client ws;

	int msgID = 1; //to keep track of commands sent to chrome
	vector<int> pendingPngDataIDs;

	//chrome contact info
	string chromeBinaryPath;
	string chromeRemoteDebugIP;
	int chromeRemoteDebugPort;

	ofxExternalProcess chromeProcess;
	void onProcessEnded(ofxExternalProcess::Result & res);

	bool launchChrome();
	void openWebSocket();

	float time = 0;
};

