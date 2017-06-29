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
#include <mutex>
#include <condition_variable>

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
	time = 0;
}


bool ofxChrome::launchChrome(){

	//--remote-debugging-port=9222 --disable-gpu --headless --no-first-run  --disable-new-profile-management  --user-data-dir="~/Desktop/chromeTest" about:blank
	vector<string> args = {
		"--remote-debugging-port=" + ofToString(chromeRemoteDebugPort),
		"--disable-gpu",
		"--no-first-run",
		"--enable-devtools-experiments",
		"--enable-experimental-web-platform-features",
		"--window-size=640,1300", //TODO!
		"--disable-overlay-scrollbar",
		"--disable-smooth-scrolling",
		"--disable-threaded-scrolling",
		"--hide-scrollbars",
		//"--default-background-color=0",
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

	while(!ok && numTry < 30){ //lets find a tab in chrome to connect a WS to
		string url = "http://" + chromeAddress + ":" + chromePort + "/json";
		ofLogNotice("ofxChrome") << "trying to connect to Chrome: " << url;
		ofHttpResponse res = ofLoadURL(url);
		int retryInterval = 250; //ms
		
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
				ofSleepMillis(retryInterval);
			}

		}else{
			ofLogError("ofxChrome") << "Can't connect to Chrome Debugger...";
			ofSleepMillis(retryInterval);
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
		ofNotifyEvent(eventChromeReady, *this, this);
	}else{
		state = FAILED_TO_START_CHROME;
		ofLogError("ofxChrome") << "could not connect to chrome! giving up!";
		ofNotifyEvent(eventChromeSetupFailed, *this, this);
	}
}


void ofxChrome::onProcessEnded(ofxExternalProcess::Result & res){
	ofLogWarning("ofxChrome") << "chrome external process ended with status code: " << res.statusCode << "\nprocess output follows: " << endl << res.stdOutput;
}


void ofxChrome::update(float dt){

	time += dt;
	if(state == LAUNCHING_CHROME && chromeBinaryPath != ""){
		if (time > 1.5){ //TODO not the best option here
			openWebSocket();
		}
	}
	chromeProcess.update();

	if(currentTransaction){
		//if
	}
}


bool ofxChrome::loadPage(string url, int & requestID){
	if(currentTransaction == nullptr){

		ofLogNotice("ofxChrome") << "loadPage() \"" << url << "\" ID: " << msgID;

		Transaction * t = new Transaction();
		t->websocket = &ws;
		t->type = LOAD_PAGE;
		requestID = t->ID = msgID;

		AsyncInput input;
		input.url = url;

		currentTransaction = t;
		msgID += 10;

		std::function<AsyncOutput(const AsyncInput&)> asyncFunc = [t](const AsyncInput & in){ //note we are capturing the transaction

			int w = 1280;
			int h = 10;
			int internalID = 0;

			ofLogNotice("ofxChrome") << "####################  set viewport size" << w << " x " << h;
			t->loadingInfo.state = SETTING_WINDOW_SIZE;
			string size = "\"width\":" + ofToString(w) + ",\"height\":" + ofToString(h);
			//string command = "{\"id\":" + ofToString(msgID) + ",\"method\":\"Browser.setWindowBounds\",\"params\":{\"windowID\":0,\"bounds\":{\"width\":" + ofToString(w) + "}}}";
			string command;

			command = "{\"id\":" + ofToString(t->ID + internalID) + ",\"method\":\"Emulation.setDeviceMetricsOverride\",\"params\":{" + size + ",\"deviceScaleFactor\":0.0,\"mobile\":false,\"fitWindow\":false}}";
			t->websocket->send(command); internalID++;

			command = "{\"id\":" + ofToString(t->ID + internalID) + ",\"method\":\"Emulation.setVisibleSize\",\"params\":{" + size + "}}";
			t->websocket->send(command); internalID++;

			/*/////*/

			AsyncOutput r;
			t->loadingInfo.state = SETTING_UP;
			string navigateUrlMsg = "{\"id\":" + ofToString(t->ID + internalID) + ",\"method\":\"Page.navigate\",\"params\":{\"url\":\"" + in.url + "\"}}";
			t->websocket->send(navigateUrlMsg);
			internalID++;

			ofLogNotice("ofxChrome") << "####################  requesting load of " << in.url;
			t->loadingInfo.state = LOADING_PAGE;
			std::unique_lock<std::mutex> lock(t->mutex); //wait until semaphore is GO!
			t->semaphore.wait(lock);	//at this point, we are waiting for Chrome to reply - so when the websocket gets data, will check for the
										//transaction ID and notify the semaphhore when to proceed


			string getDOM = "{\"id\":" + ofToString(t->ID + internalID) + ",\"method\":\"DOM.getDocument\"}";
			ofLogNotice("ofxChrome") << "####################  requesting DOM >> " << getDOM;
			t->websocket->send(getDOM);
			internalID++;
			t->loadingInfo.state = REQUESTING_DOM;
			t->semaphore.wait(lock);


			string params = "{\"nodeId\":" + ofToString(t->DomRootNodeID) + ",\"selector\":\"body\"}";
			string getBody = "{\"id\":" + ofToString(t->ID + internalID) + ",\"method\":\"DOM.querySelector\", \"params\":" + params + "}";
			ofLogNotice("ofxChrome") << "####################  requesting BODY >> " << getDOM;
			t->websocket->send(getBody);
			internalID++;
			t->loadingInfo.state = REQUESTING_BODY;
			t->semaphore.wait(lock);

			params = "{\"nodeId\":" + ofToString(t->bodyNodeID) + "}";
			string getHeight = "{\"id\":" + ofToString(t->ID + internalID) + ",\"method\":\"DOM.getBoxModel\", \"params\":" + params + "}";
			ofLogNotice("ofxChrome") << "####################  requesting HEIGHT >> " << getHeight;
			t->websocket->send(getHeight);
			internalID++;
			t->loadingInfo.state = QUERYING_FRAME_SIZE;
			t->semaphore.wait(lock);

			size = "\"width\":" + ofToString((int)t->bodySize.x) + ",\"height\":" + ofToString((int)t->bodySize.y);
			command = "{\"id\":" + ofToString(t->ID + internalID) + ",\"method\":\"Emulation.setVisibleSize\",\"params\":{" + size + "}}";
			ofLogNotice("ofxChrome") << "####################  setting FRAME SIZE >> " << command;
			t->websocket->send(command);
			internalID++;
			t->loadingInfo.state = SETTING_WINDOW_SIZE;
			t->semaphore.wait(lock);

			command = "{\"id\":" + ofToString(t->ID + internalID) + ",\"method\":\"Emulation.forceViewport\",\"params\":{\"x\":0,\"y\":0,\"scale\":1}}";
			ofLogNotice("ofxChrome") << "####################  setting FORCE VIEWPORT >> " << command;
			t->websocket->send(command);
			internalID++;
			t->loadingInfo.state = FORCE_VIEWPORT;
			t->semaphore.wait(lock);

			ofLogNotice("ofxChrome") << "####################  requesting PIXEL DATA >> " << getHeight;
			string saveScreenshotCmd = "{\"id\":" + ofToString(t->ID + internalID) + ",\"method\":\"Page.captureScreenshot\",\"params\":{}}";
			t->websocket->send(saveScreenshotCmd);
			internalID++;
			t->loadingInfo.state = GET_PIXEL_DATA;
			t->semaphore.wait(lock);

			t->loadingInfo.state = DONE;
			r.pixels = t->pixels;
			return r;
		};

		std::function<void(const AsyncOutput &)> asyncResultReadyFunc = [this](const AsyncOutput & res){
			ofLogNotice("ofxChrome") << "async result ready!";

			delete currentTransaction;
			currentTransaction = nullptr;
			PagePixels pp;
			pp.pixels = res.pixels;
			ofNotifyEvent(eventPixelsRead, pp, this);
		};

		t->async.startTask(asyncFunc, input, asyncResultReadyFunc);

	}else{
		ofLogError("ofxChrome") << "currently busy! cant loadPage() now!";
	}
}


bool ofxChrome::setWindowSize(int w, int h){
	ofLogNotice("ofxChrome") << "SetWindowSize: " << w << " x " << h << " ID: " << msgID;
	//Browser.getWindowForTarget
	//Browser.setWindowBounds

	string size = "\"width\":" + ofToString(w) + ",\"height\":" + ofToString(h);
	//string command = "{\"id\":" + ofToString(msgID) + ",\"method\":\"Browser.setWindowBounds\",\"params\":{\"windowID\":0,\"bounds\":{\"width\":" + ofToString(w) + "}}}";
	string command;

	command = "{\"id\":" + ofToString(msgID) + ",\"method\":\"Emulation.setDeviceMetricsOverride\",\"params\":{" + size + ",\"deviceScaleFactor\":0.0,\"mobile\":false,\"fitWindow\":false}}";
	ws.send(command); msgID++;

	command = "{\"id\":" + ofToString(msgID) + ",\"method\":\"Emulation.setVisibleSize\",\"params\":{" + size + "}}";
	ws.send(command); msgID++;

}


bool ofxChrome::setTransparentBackground(bool trans){
	ofLogNotice("ofxChrome") << "setTransparentBackground: " << trans;
	string saveScreenshotMsg = "{\"id\":" + ofToString(msgID) + ",\"method\":\"Page.setDocumentContent\",\"params\":{\"r\":255,\"g\":0,\"b\":0,\"a\":255}}";
	ws.send(saveScreenshotMsg); msgID++;
}

void ofxChrome::setPageNotifications(bool enabled){

	ofLogNotice("ofxChrome") << "setPageNotifications: " << enabled << " ID: " << msgID;
	string command = "{\"id\":" + ofToString(msgID) + ",\"method\":\"Page.enable\"}";
	ws.send(command); msgID++;

	command = "{\"id\":" + ofToString(msgID) + ",\"method\":\"DOM.enable\"}";
	ws.send(command); msgID++;

	command = "{\"id\":" + ofToString(msgID) + ",\"method\":\"Network.enable\"}";
	//ws.send(command); msgID++;

}

void ofxChrome::loadHTML(const string & html, int & requestID){
}

void ofxChrome::draw(int x, int y){
	ofDrawBitmapStringHighlight(chromeProcess.getSmartOutput(), x, y);
}

# pragma mark - WebsocketCallbacks

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
	ofLogNotice("ofxChrome") << "ofxLibwebsockets got message: " << ofToString(ofGetElapsedTimef()) << " - " << args.message.substr(0, MIN(100,args.message.size()) );

	Json::Reader reader;
	Json::Value json;
	bool ok = reader.parse(args.message, json, false);

	if(ok){

		bool msgHasID = json["id"].isInt();
		bool msgHasMethod = json["method"].isString();

		if(msgHasID){ //msg refers to a transaction

			int thisID = json["id"].asInt();

			if(currentTransaction == nullptr){
				ofLogError("ofxChrome") << "cant find a transaction for this ID? " << thisID;
			}else{

				if(currentTransaction->type == LOAD_PAGE){

					if(currentTransaction->loadingInfo.state == REQUESTING_DOM){
						if(!json["result"]["root"]["nodeId"].isNull()){
							currentTransaction->DomRootNodeID = json["result"]["root"]["nodeId"].asInt();
						}else ofLogError("ofxChrome") << "cant get DOM?";
						currentTransaction->semaphore.notify_all(); //dom is ready
					}

					if(currentTransaction->loadingInfo.state == REQUESTING_BODY){
						if(!json["result"]["nodeId"].isNull()){
							currentTransaction->bodyNodeID = json["result"]["nodeId"].asInt();
						}else ofLogError("ofxChrome") << "cant get BODY?";
						currentTransaction->semaphore.notify_all();
					}

					if(currentTransaction->loadingInfo.state == QUERYING_FRAME_SIZE){
						if(!json["result"]["model"]["width"].isNull()){
							currentTransaction->bodySize.x = json["result"]["model"]["width"].asInt();
							currentTransaction->bodySize.y = json["result"]["model"]["height"].asInt();
						}else ofLogError("ofxChrome") << "cant get BODY SIZE?";
						ofLogNotice() << args.message;
						currentTransaction->semaphore.notify_all();
					}

					if(currentTransaction->loadingInfo.state == SETTING_WINDOW_SIZE){
						currentTransaction->semaphore.notify_all();
					}
					if(currentTransaction->loadingInfo.state == FORCE_VIEWPORT){
						currentTransaction->semaphore.notify_all();
					}

					if(currentTransaction->loadingInfo.state == GET_PIXEL_DATA){
						//decode base64 data
						if(!json["result"]["data"].isNull()){
							istringstream istr(json["result"]["data"].asString());
							Poco::Base64Decoder decoder(istr);
							ofLoadImage(currentTransaction->pixels, ofBuffer(decoder));
						}
						currentTransaction->semaphore.notify_all();
					}
				}
			}
		}

		if(msgHasMethod){ //msg most likely refers to a "method"

			string method = json["method"].asString();
			//bool frameLoading = method == "Page.frameStartedLoading";
			//bool frameStoppedLoading = method == "Page.frameStoppedLoading";
			if(method == "Page.loadEventFired"){ //page is done loading!
				if(currentTransaction){
					if(currentTransaction->type == LOAD_PAGE){
						currentTransaction->semaphore.notify_all(); //page is loaded, carry on the LOAD_PAGE transaction
					}else{
						ofLogError("ofxChrome") << "wtf transaction type makes no sense for this reply? " << method;
					}
				}
			}
		}

	}else{
		ofLogError("ofxChrome") << "cant parse msg from chrome?";
	}
}


void ofxChrome::onBroadcast( ofxLibwebsockets::Event& args ){
	ofLogNotice("ofxChrome") << "ofxLibwebsockets got broadcast " << args.message;
}
