#include "ofApp.h"


void ofApp::setup(){

	bool headless = false;
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
	tex.clear();
	tex.loadData(data.pixels);
}


void ofApp::keyPressed(int key){


	if(key=='1'){
		chrome.loadPage("http://uri.cat", true);
	}

	if(key=='2'){
		chrome.loadPage("http://apple.com", false);
	}

	if(key=='3'){
		chrome.loadPage("https://www.youtube.com", false);
	}

	if(key=='4'){
		chrome.loadPage("http://uri.cat/blank.html", false);
	}

	if(key=='5'){
		string html = "<html><body>It's this time in OpenFrameworks: " + ofGetTimestampString() +  "</body></html>";
		chrome.loadHTML(html, true);
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
