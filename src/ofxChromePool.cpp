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


void ofxChromePool::setup(int numChromeInstances, string chromeBinaryPath, string chromeRemoteDebugIP, bool headless){

	numChromeInstances = MAX(1, numChromeInstances);

	int basePort = 35260;
	int port = basePort;
	for(int i = 0; i < numChromeInstances; i++){

		auto c = new ofxChrome();

		ofAddListener(c->eventChromeReady, this, &ofxChromePool::onChromeReady);
		ofAddListener(c->eventChromeSetupFailed, this, &ofxChromePool::onChromeSetupFailed);
		ofAddListener(c->eventPixelsReady, this, &ofxChromePool::onPixelsReady);

		c->setup(chromeBinaryPath, chromeRemoteDebugIP, port, headless);
		chromes[c] = ChromeState();
		port++;
	}
}

void ofxChromePool::onChromeReady(ofxChrome& who){
	ofLogError("ofxChromePool") << "chrome " << &who << " setup!";
	chromes[&who].setup = true;
	chromes[&who].busy = false;
}

void ofxChromePool::onChromeSetupFailed(ofxChrome& who){
	ofLogError("ofxChromePool") << "chrome setup failed!";
	chromes[&who].setup = false;
	chromes[&who].busy = false;
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

	string msg = "ofxChromePool:";
	int c = 0;
	for(auto & it : chromes){
		msg += "\n   [" + ofToString(c) + "] setup: " + ofToString(it.second.setup) + " busy: " + ofToString(it.second.busy);
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