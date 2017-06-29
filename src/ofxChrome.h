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
#include "AsyncHandler.h"
#include <mutex>
#include <condition_variable>

class ofxChrome{

public:
	
	ofxChrome();
	~ofxChrome();

	//if "chromeBinaryPath" is not empty, it will launch chrome for you as an external process
	//and keep control of it (ie watchdog, etc)
	void setup(string chromeBinaryPath, string chromeRemoteDebugIP, int chromeRemoteDebugPort, bool headless);

	void update(float dt);
	void draw(int x, int y);

	bool loadPage(string url, bool fullPage = false); //if fullPage==true, it will return the whole page height vs only the only visible part that would fit in the browser window
	void loadHTML(const string & html, bool fullPage = false);

	//bool setTransparentBackground(bool trans); //TODO!
	bool setWindowSize(int w, int h);

	struct PagePixels{
		ofPixels pixels;
		ofxChrome * who;
		string url;
	};

	ofFastEvent<ofxChrome> eventChromeSetupFailed; 	//
	ofFastEvent<ofxChrome> eventChromeReady; 		//
	ofFastEvent<PagePixels> eventPixelsRead; 		//


	// WebSocket callbacks - dont call directly!!!!
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

	enum TransactionType{
		LOAD_PAGE,
	};

	enum LoadPageStatus{
		SETTING_UP,
		LOADING_PAGE,
		REQUESTING_DOM,
		REQUESTING_BODY,
		QUERYING_FRAME_SIZE,
		SETTING_WINDOW_SIZE,
		FORCE_VIEWPORT,
		GET_PIXEL_DATA,
		DONE
	};

	State state = UNINITED;

	ofxLibwebsockets::Client ws;

	int msgID = 1; //to keep track of commands sent to chrome

	//chrome contact info
	string chromeBinaryPath;
	string chromeRemoteDebugIP;
	int chromeRemoteDebugPort;
	bool headlessMode;

	ofxExternalProcess chromeProcess;
	void onProcessEnded(ofxExternalProcess::Result & res);

	bool launchChrome();
	void openWebSocket();

	float time;

	struct AsyncInput{
		string url;
		string html;
		ofVec2f browserWinSize;
		bool fullPage;
	};

	struct AsyncOutput{
		ofPixels pixels;
		string url;
	};

	struct LoadPageInfo{
		LoadPageStatus state;
		int numFramesLoading;
		int numFramesLoaded;
		bool loadEventFired;
		LoadPageInfo(){ numFramesLoading = numFramesLoaded = 0; loadEventFired = false;}
	};

	struct Transaction{

		int ID;
		ofxLibwebsockets::Client * websocket;
		TransactionType type;
		LoadPageInfo loadingInfo; //only applies to LOAD_PAGE type

		std::mutex mutex;
		std::condition_variable semaphore;

		AsyncHandler<AsyncInput, AsyncOutput> async;
		bool readyToDelete;

		int DomRootNodeID;
		int bodyNodeID;
		ofVec2f bodySize;
		ofPixels pixels;
		bool isCustomHtml;

		Transaction(){readyToDelete = true; DomRootNodeID = bodyNodeID = -1;}
	};

	Transaction* currentTransaction = nullptr;

	ofVec2f browserWinSize = ofVec2f(1280, 720);

	void enableRequiredNotifications(bool enabled);
	void browserFetch(const string & url, const string & html, bool fullpage);

};

