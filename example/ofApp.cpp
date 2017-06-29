#include "ofApp.h"


void ofApp::setup(){

	bool headless = true;
	string chromeBin = "/Applications/Inet_Apps/Browsers/Google Chrome.app/Contents/MacOS/Google Chrome"; //standard
	string canaryBin = "/Users/oriol/Desktop/Google Chrome Canary.app/Contents/MacOS/Google Chrome Canary"; //canary
	string chromeBetaBin = "/Users/oriol/Desktop/Google Chrome.app/Contents/MacOS/Google Chrome"; //beta

	string chosenBin = chromeBin;
	chrome.setup(chosenBin, "127.0.0.1", 9222, headless);
	chrome2.setup(chosenBin, "127.0.0.1", 9333, headless);

	ofAddListener(chrome.eventPixelsRead, this, &ofApp::onPixelsReady);
	ofAddListener(chrome2.eventPixelsRead, this, &ofApp::onPixelsReady);
}


void ofApp::update(){

	float dt = 1./60;
	chrome.update(dt);
	chrome2.update(dt);

}


void ofApp::draw(){

	if(tex.isAllocated()){
		ofRectangle r = ofRectangle(0,0,tex.getWidth(), tex.getHeight());
		auto viewport = ofGetCurrentViewport();
		viewport.width *= 0.5;
		r.scaleTo(viewport);
		tex.draw(r);
	}

	if(tex2.isAllocated()){
		ofRectangle r = ofRectangle(0,0,tex2.getWidth(), tex2.getHeight());
		auto viewport = ofGetCurrentViewport();
		viewport.width *= 0.5;
		viewport.x += viewport.width;
		r.scaleTo(viewport);
		tex2.draw(r);
	}
}


void ofApp::onPixelsReady(ofxChrome::PagePixels& data){

	ofLogNotice() << "ofxChrome pixels ready : " << data.pixels.getWidth() << " x " << data.pixels.getHeight();

	if(data.who == &chrome){
		tex.clear();
		tex.loadData(data.pixels);
	}

	if(data.who == &chrome2){
		tex2.clear();
		tex2.loadData(data.pixels);
	}
}


void ofApp::keyPressed(int key){


	if(key=='1'){
		chrome.loadPage("http://uri.cat", true);
	}

	if(key=='2'){
		chrome.loadPage("http://apple.com", false);
	}

	if(key=='3'){
		chrome2.loadPage("https://www.youtube.com", false);
	}

	if(key=='4'){
		chrome2.loadPage("http://uri.cat/blank.html", false);
	}

	if(key=='5'){
		string html = "<html><body>It's this time in OpenFrameworks: " + ofGetTimestampString() +  "</body></html>";
		chrome2.loadHTML(html, true);
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
