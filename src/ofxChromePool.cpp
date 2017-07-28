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
	numInstances =  MAX(1, numChromeInstances);
 	basePort = MAX(1001,basePort);

	port = basePort + ofRandom(100);
	for(int i = 0; i < numInstances; i++){
		newChrome();
	}
}


void ofxChromePool::newChrome(){
	auto c = new ofxChrome();
	ofAddListener(c->eventChromeReady, this, &ofxChromePool::onChromeReady);
	ofAddListener(c->eventChromeSetupFailed, this, &ofxChromePool::onChromeSetupFailed);
	ofAddListener(c->eventPixelsReady, this, &ofxChromePool::onPixelsReady);
	c->setup(chromePath, chromeIP, port, chromeHeadless);
	port++;
	mutex.lock();
	chromes[c] = ChromeState();
	mutex.unlock();
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
	mutex.lock();
	chromes.erase(chromes.find(&who));
	mutex.unlock();

	newChrome();
}


void ofxChromePool::onPixelsReady(ofxChrome::PagePixels& data){
	chromes[data.who].busy = false;
	ofNotifyEvent(eventPixelsReady, data, this);
}

void ofxChromePool::update(float dt){

	mutex.lock();
	map<ofxChrome*, ChromeState> chromesCopy = chromes;
	mutex.unlock();

	for(auto & it : chromesCopy){
		it.first->update(dt);
	}

	if(!notifiedSetup){
		int nSetup = 0;
		for(auto & it : chromesCopy){
			if(it.second.setup) nSetup++;
		}
		if(nSetup == numInstances){
			ofNotifyEvent(eventChromeReady, *this, this);
		}
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


bool ofxChromePool::loadPage(string url, ofVec2f winSize, float timeout, bool fullPage){

	auto c = getAvaialbleChrome();
	if(c){
		chromes[c].busy = true;
		c->setLoadTimeout(timeout);
		c->setWindowSize(winSize.x, winSize.y);
		c->loadPage(url, fullPage);
		return true;
	}else{
		return false;
	}
}


bool ofxChromePool::loadHTML(const string & html, ofVec2f winSize, float timeout, bool fullPage){

	auto c = getAvaialbleChrome();
	if(c){
		chromes[c].busy = true;
		c->setLoadTimeout(timeout);
		c->setWindowSize(winSize.x, winSize.y);
		c->loadHTML(html, fullPage);
		return true
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
