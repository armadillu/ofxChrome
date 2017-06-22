//
//  ofxChrome.cpp
//  ofxofxChrome
//
//  Created by Oriol Ferrer Mesià on 21/06/2017.
//
//

#include "ofxChrome.h"
#include "Poco/Base64Decoder.h"
#include "Poco/StreamCopier.h"

ofxChrome::ofxChrome(){

}

ofxChrome::~ofxChrome(){
	ofLogWarning("ofxChrome") << "~ofxChrome()";
	chromeProcess.kill();
	chromeProcess.waitForThread(false);
	ofLogNotice("ofxChrome") << "done waiting it seems...";
	while(chromeProcess.isThreadRunning()){
		ofLogWarning("ofxChrome") << "still running?";
		ofSleepMillis(5);
	}
	ws.close();
}

void ofxChrome::setup(string chromeBinaryPath, string chromeRemoteDebugIP, int chromeRemoteDebugPort, bool headless){

	this->headlessMode = headless;
	this->chromeBinaryPath = chromeBinaryPath;
	this->chromeRemoteDebugIP = chromeRemoteDebugIP;
	this->chromeRemoteDebugPort = chromeRemoteDebugPort;

	if(chromeBinaryPath.size()){
		bool ok = launchChrome();
		state = LAUNCHING_CHROME;
	}else{
		state = CONNECTING_TO_WS;
		openWebSocket();
	}
}


bool ofxChrome::launchChrome(){

	//--remote-debugging-port=9222 --disable-gpu --headless --no-first-run  --disable-new-profile-management  --user-data-dir="~/Desktop/chromeTest" about:blank
	vector<string> args = {
		"--remote-debugging-port=" + ofToString(chromeRemoteDebugPort),
		"--disable-gpu",
		"--no-first-run",
		"--enable-devtools-experiments",
		"--enable-experimental-web-platform-features",
		"--window-size=640,480", //TODO!
		"--disable-overlay-scrollbar",
		"--disable-smooth-scrolling",
		"--disable-threaded-scrolling",
		"--hide-scrollbars",
		"--default-background-color=0",
		"--profile-directory=" + ofGetTimestampString(), //start new session every time
		"about:blank"		
	};
	if(headlessMode){
		args.insert(args.begin(), "--headless");
	}

	chromeProcess.setLivePipe(ofxExternalProcess::IGNORE_OUTPUT);
	chromeProcess.setup(".", chromeBinaryPath, args);
	ofAddListener(chromeProcess.eventProcessEnded, this, &ofxChrome::onProcessEnded);
	
	chromeProcess.executeInThreadAndNotify();
	return true; //todo!
}


void ofxChrome::openWebSocket(){

	state = CONNECTING_TO_WS;
	ofxLibwebsockets::ClientOptions options = ofxLibwebsockets::defaultClientOptions();

	string chromeAddress = chromeRemoteDebugIP;
	string chromePort = ofToString(chromeRemoteDebugPort);

	int numTry = 0;
	bool ok = false;
	string jsonStr;
	string wsRequestPath;

	while(!ok && numTry < 10){ //lets find a tab in chrome to connect a WS to
		string url = "http://" + chromeAddress + ":" + chromePort + "/json";
		ofLogNotice("ofxChrome") << "trying to connect to Chrome: " << url;
		ofHttpResponse res = ofLoadURL(url);
		if(res.status == 200){
			jsonStr = res.data.getData();
			ofLogNotice() << jsonStr;

			Json::Reader reader;
			Json::Value json;

			reader.parse(jsonStr, json, false);
			string websocketURL;

			for(auto & it : json){
				if(it["type"].asString() == "page"){
					if(it["title"].asString() == "about:blank"){
						websocketURL = it["webSocketDebuggerUrl"].asString();
					}
				}
			}

			auto split = ofSplitString(websocketURL, chromePort);
			if (split.size() > 1){
				wsRequestPath = split.back();
				ofLogNotice("ofxChrome")<< "websocket found: " << websocketURL;
				ok = true;
			}else{
				ofLogError("ofxChrome") << "wtf! Can't find a Chrome websocket!";
			}

		}else{
			ofLogError("ofxChrome") << "Can't connect to Chrome Debugger...";
			ofSleepMillis(250);
		}
		numTry++;
	}

	if(ok){
		options.host = chromeAddress;
		options.port = ofToInt(chromePort);
		options.bUseSSL = false;
		options.channel = wsRequestPath;
		options.ka_time     = 1;
		options.ka_probes   = 1;
		options.ka_interval = 1;

		ws.addListener(this);

		ok = ws.connect(options);
		ofLogNotice("ofxChrome") << "Open WebSocket: " << ok;
		state = READY_FOR_COMMANDS;
	}else{
		state = FAILED_TO_START_CHROME;
		ofLogError("ofxChrome") << "could not connect to chrome! giving up!";
	}
}


void ofxChrome::onProcessEnded(ofxExternalProcess::Result & res){
	ofLogWarning("ofxChrome") << "chrome external process ended with status code: " << res.statusCode << "\nprocess output follows: " << endl << res.stdOutput;
}


void ofxChrome::update(float dt){

	time += dt;
	if(state == LAUNCHING_CHROME && chromeBinaryPath != ""){
		if (time > 1.5){
			openWebSocket();
		}
	}
	chromeProcess.update();
}


bool ofxChrome::navigateToPage(string url, int & requestID){
	ofLogNotice("ofxChrome") << "loading url: " << url << " ID: " << msgID;
	string navigateUrlMsg = "{\"id\":" + ofToString(msgID) + ",\"method\":\"Page.navigate\",\"params\":{\"url\":\"" + url + "\"}}";
	requestID = msgID;
	ws.send(navigateUrlMsg); msgID++;
}


bool ofxChrome::requestScreenshot(int & requestID){
	ofLogNotice("ofxChrome") << "Request Screenshot: " << " ID: " << msgID;
	string saveScreenshotMsg = "{\"id\":" + ofToString(msgID) + ",\"method\":\"Page.captureScreenshot\",\"params\":{}}";
	pendingPngDataIDs.push_back(msgID);
	requestID = msgID;
	ws.send(saveScreenshotMsg); msgID++;
}

bool ofxChrome::setWindowSize(int w, int h){
	ofLogNotice("ofxChrome") << "SetWindowSize: " << w << " x " << h << " ID: " << msgID;
	//Browser.getWindowForTarget
	//Browser.setWindowBounds

	string size = "\"width\":" + ofToString(w) + ",\"height\":" + ofToString(h);
	//string command = "{\"id\":" + ofToString(msgID) + ",\"method\":\"Browser.setWindowBounds\",\"params\":{\"windowID\":0,\"bounds\":{\"width\":" + ofToString(w) + "}}}";
	string command;

	command = "{\"id\":" + ofToString(msgID) + ",\"method\":\"Emulation.setVisibleSize\",\"params\":{" + size + "}}";
	ws.send(command); msgID++;

	command = "{\"id\":" + ofToString(msgID) + ",\"method\":\"Emulation.setDeviceMetricsOverride\",\"params\":{" + size + ",\"deviceScaleFactor\":1.0,\"mobile\":true,\"fitWindow\":true}}";
	ws.send(command); msgID++;
}


bool ofxChrome::setTransparentBackground(bool trans){
	ofLogNotice("ofxChrome") << "setTransparentBackground: " << trans;
	string saveScreenshotMsg = "{\"id\":" + ofToString(msgID) + ",\"method\":\"Page.setDocumentContent\",\"params\":{\"r\":255,\"g\":0,\"b\":0,\"a\":255}}";
	ws.send(saveScreenshotMsg); msgID++;
}

void ofxChrome::setPageNotifications(bool enabled){

	ofLogNotice("ofxChrome") << "setPageNotifications: " << enabled << " ID: " << msgID;
	string saveScreenshotMsg = "{\"id\":" + ofToString(msgID) + ",\"method\":\"Page.enable\"}";
	ws.send(saveScreenshotMsg); msgID++;

}

void ofxChrome::loadHTML(const string & html, int & requestID){
	string saveScreenshotMsg = "{\"id\":" + ofToString(msgID) + ",\"method\":\"Emulation.setDefaultBackgroundColor\",\"params\":{\"color\":{\"r\":255,\"g\":0,\"b\":0,\"a\":255}}}";
	ws.send(saveScreenshotMsg); msgID++;
}

void ofxChrome::draw(int x, int y){
	ofDrawBitmapStringHighlight(chromeProcess.getSmartOutput(), x, y);
}

void ofxChrome::onConnect( ofxLibwebsockets::Event& args ){
	ofLogNotice("ofxChrome") << "ofxLibwebsockets on connected";
}


void ofxChrome::onOpen( ofxLibwebsockets::Event& args ){
	ofLogNotice("ofxChrome") << "ofxLibwebsockets on open";
	//setTransparentBackground(true);
	setPageNotifications(true); //enable page notifications so we get events for loaded webpages
}


void ofxChrome::onClose( ofxLibwebsockets::Event& args ){
	ofLogNotice("ofxChrome") << "ofxLibwebsockets on close";
}


void ofxChrome::onIdle( ofxLibwebsockets::Event& args ){
	//ofLogNotice("ofxChrome") << "ofxLibwebsockets on idle";
}


void ofxChrome::onMessage( ofxLibwebsockets::Event& args ){
	ofLogNotice("ofxChrome") << "ofxLibwebsockets got message: " << args.message.substr(0, MIN(100,args.message.size()) );

	Json::Reader reader;
	Json::Value json;
	bool ok = reader.parse(args.message, json, false);

	if(ok){
		int thisID = json["id"].asInt();
		auto it = find(pendingPngDataIDs.begin(), pendingPngDataIDs.end(), thisID);
		if( it != pendingPngDataIDs.end()){

			//decode base64 data
			istringstream istr(json["result"]["data"].asString());
			Poco::Base64Decoder decoder(istr);
			ofBuffer buf = ofBuffer(decoder);
			ofPixels pix;
			ofLoadImage(pix, buf);
			ofSaveImage(pix, "test.png");

			pendingPngDataIDs.erase(it);
		}
	}else{
		ofLogError("ofxChrome") << "cant parse msg from chrome?";
	}
}


void ofxChrome::onBroadcast( ofxLibwebsockets::Event& args ){
	ofLogNotice("ofxChrome") << "ofxLibwebsockets got broadcast " << args.message;
}
