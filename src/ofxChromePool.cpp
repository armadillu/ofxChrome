//
//  ofxChromePool.cpp
//  ofxChromeHeadless
//
//  Created by Oriol Ferrer Mesià on 05/07/2017.
//
//

#include "ofxChromePool.h"

ofxChromePool::ofxChromePool(){
}

ofxChromePool::~ofxChromePool(){

	for(auto & it : chromes){
		delete it.first;
	}
	chromes.clear();
}


void ofxChromePool::setup(int numChromeInstances, string chromeBinaryPath, string chromeRemoteDebugIP, int basePort, bool headless){

	chromeHeadless = headless;
	chromeIP = chromeRemoteDebugIP;
	chromePath = chromeBinaryPath;
	numChromeInstances = MAX(1, numChromeInstances);
 	basePort = MAX(1001,basePort);

	port = basePort;
	for(int i = 0; i < numChromeInstances; i++){
		newChrome();
		port += 2;
	}
}


void ofxChromePool::newChrome(){
	auto c = new ofxChrome();
	ofAddListener(c->eventChromeReady, this, &ofxChromePool::onChromeReady);
	ofAddListener(c->eventChromeSetupFailed, this, &ofxChromePool::onChromeSetupFailed);
	ofAddListener(c->eventPixelsReady, this, &ofxChromePool::onPixelsReady);
	c->setup(chromePath, chromeIP, port, chromeHeadless);
	chromes[c] = ChromeState();
}

void ofxChromePool::onChromeReady(ofxChrome& who){
	ofLogError("ofxChromePool") << "chrome " << &who << " setup!";
	chromes[&who].setup = true;
	chromes[&who].busy = false;
}

void ofxChromePool::onChromeSetupFailed(ofxChrome& who){
	ofLogError("ofxChromePool") << "chrome setup failed! making a new one?";
	chromes[&who].setup = false;
	chromes[&who].busy = false;

	delete &who;
	chromes.erase(chromes.find(&who));

	newChrome();
}


void ofxChromePool::onPixelsReady(ofxChrome::PagePixels& data){
	chromes[data.who].busy = false;
	ofNotifyEvent(eventPixelsReady, data, this);
}

void ofxChromePool::update(float dt){

	for(auto & it : chromes){
		it.first->update(dt);
	}
}


void ofxChromePool::drawStatus(int x, int y){

	string msg = "## ofxChromePool ##";
	int c = 0;
	for(auto & it : chromes){
		msg += "\n[" + ofToString(c) + "] setup: " + ofToString(it.second.setup) + " busy: " + ofToString(it.second.busy);
		c++;
	}

	ofDrawBitmapStringHighlight(msg, x, y);
}


bool ofxChromePool::loadPage(string url, bool fullPage){

	auto c = getAvaialbleChrome();
	if(c){
		chromes[c].busy = true;
		c->loadPage(url, fullPage);
	}else{
		return false;
	}
}


bool ofxChromePool::loadHTML(const string & html, bool fullPage){

	auto c = getAvaialbleChrome();
	if(c){
		chromes[c].busy = true;
		c->loadHTML(html, fullPage);
	}else{
		return false;
	}
}


ofxChrome* ofxChromePool::getAvaialbleChrome(){

	for(auto & it : chromes){
		if(it.second.setup && !it.second.busy){
			return it.first;
		}
	}
	return nullptr;
}