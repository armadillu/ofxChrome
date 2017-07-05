//
//  ofxChromePool.h
//  ofxChromeHeadless
//
//  Created by Oriol Ferrer Mesià on 05/07/2017.
//
//

#pragma once
#include "ofxChrome.h"

class ofxChromePool{

public:
	
	ofxChromePool();
	~ofxChromePool();

	void setup(int numChromeInstances, string chromeBinaryPath, string chromeRemoteDebugIP, int basePort, bool headless);

	void update(float dt);
	void drawStatus(int x, int y);

	int getNumInstances(){return chromes.size();}

	ofFastEvent<ofxChromePool> 			eventChromeReady;
	ofFastEvent<ofxChrome> 				eventChromeSetupFailed; 	//
	ofFastEvent<ofxChrome::PagePixels> 	eventPixelsReady;

	bool loadPage(string url, bool fullPage = false); //if fullPage==true, it will return the whole page height vs only the only visible part that would fit in the browser window
	bool loadHTML(const string & html, bool fullPage = false);


	void onPixelsReady(ofxChrome::PagePixels& data);
	void onChromeReady(ofxChrome&);
	void onChromeSetupFailed(ofxChrome&);

protected:

	struct ChromeState{
		bool setup = false;
		bool busy = false;
	};

	void newChrome();
	map<ofxChrome*, ChromeState> chromes;

	ofxChrome* getAvaialbleChrome();

	bool chromeHeadless;
	string chromeIP;
	string chromePath;

	int port; //currently used port (incs)
};

