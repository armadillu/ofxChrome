#include "ofApp.h"


void ofApp::setup(){

	bool headless = true;
	string chromeBin = "/Applications/Inet_Apps/Browsers/Google Chrome.app/Contents/MacOS/Google Chrome"; //standard
	chromeBin = "/Users/oriol/Desktop/Google Chrome Canary.app/Contents/MacOS/Google Chrome Canary"; //canary
	chromeBin = "/Users/oriol/Desktop/Google Chrome.app/Contents/MacOS/Google Chrome"; //beta

	chrome.setup(chromeBin, "127.0.0.1", 9222, headless);

	ofAddListener(chrome.eventPixelsRead, this, &ofApp::onPixelsReady);
}


void ofApp::update(){

	float dt = 1./60;
	chrome.update(dt);
	
}


void ofApp::draw(){

	chrome.draw(30,30);

	if(tex.isAllocated()){
		ofRectangle r = ofRectangle(0,0,tex.getWidth(), tex.getHeight());
		r.scaleTo(ofGetCurrentViewport());
		tex.draw(r);
	}
}

void ofApp::onPixelsReady(ofxChrome::PagePixels& data){
	ofLogNotice() << "ofxChrome pixels ready : " << data.pixels.getWidth() << " x " << data.pixels.getHeight();
	tex.loadData(data.pixels);
}


void ofApp::keyPressed(int key){


	if(key=='1'){
		int reqID;
		chrome.loadPage("http://uri.cat", reqID);
	}

	if(key=='2'){
		int reqID;
		chrome.loadPage("http://apple.com", reqID);
	}

	if(key=='3'){
		int reqID;
		chrome.loadPage("https://www.youtube.com", reqID);
	}

	if(key=='4'){
		int reqID;
		chrome.loadPage("http://uri.cat/blank.html", reqID);
	}

}


void ofApp::keyReleased(int key){

}


void ofApp::mouseMoved(int x, int y ){

}


void ofApp::mouseDragged(int x, int y, int button){

}


void ofApp::mousePressed(int x, int y, int button){

}


void ofApp::mouseReleased(int x, int y, int button){

}


void ofApp::mouseEntered(int x, int y){

}


void ofApp::mouseExited(int x, int y){

}


void ofApp::windowResized(int w, int h){

}


void ofApp::gotMessage(ofMessage msg){

}


void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
