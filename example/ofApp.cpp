#include "ofApp.h"

int numInstances = 16;

void ofApp::setup(){

	ofBackground(22);

	bool headless = true;
	string chromeBin = "/Applications/Inet_Apps/Browsers/Google Chrome.app/Contents/MacOS/Google Chrome"; //standard
	string canaryBin = "/Users/oriol/Desktop/Google Chrome Canary.app/Contents/MacOS/Google Chrome Canary"; //canary
	string chromeBetaBin = "/Users/oriol/Desktop/Google Chrome.app/Contents/MacOS/Google Chrome"; //beta
	string chosenBin = chromeBin;

	ofAddListener(chromes.eventChromeReady, this, &ofApp::onChromeReady);
	ofAddListener(chromes.eventPixelsReady, this, &ofApp::onPixelsReady);

	chromes.setup(numInstances, chosenBin, "127.0.0.1", 15000, headless);

	urls = {
		"http://arstechnica.com",
		"http://news.ycombinator.com",
		"http://github.com",
		"http://apple.com",
		"https://www.macrumors.com",
		"http://twitter.com",
		"http://engadget.com",
		"http://flickr.com",
		"http://youtube.com",
		"http://uri.cat",
		"http://ara.cat",
		"http://gencat.cat",
		"http://microsoft.com",
		"http://nvidia.com",
		"http://intel.com",
		"http://raspberrypi.org",
		"http://amazon.com",
		"http://nintendo.com",
		"http://www.sciencespacerobots.com",
		"http://google.com",
		"https://www.fastcompany.com",
//sites with post-load animations
//		"https://www.google.com/chrome/index.html",
//		"https://www.apple.com/imac-pro/",
//		"https://www.apple.com/imac/",
//		"https://www.apple.com/macbook/",
//		"https://www.apple.com/macbook-air/",
//		"https://www.apple.com/mac-pro/",

	};

	#ifdef TIME_PROFILE
	TIME_SAMPLE_DISABLE();
	#endif
}


void ofApp::update(){

	float dt = 1./60;
	chromes.update(dt);

}


void ofApp::draw(){

	int n = textures.size();

	ofRectangle rect = ofGetCurrentViewport();
	int pad = 20;

	float ar = ofGetWidth() / ofGetHeight();
	float nx = sqrtf(n / ar) * ar;
	float ny = sqrtf(n * ar) / ar;

	int nn = ceil(nx) * ceil(ny);
	if( nn > n){
		if(nn - n > nx){
			ny-=1;
		}
	}

	float xx = rect.x;
	float yy = rect.y;

	for(auto & it : textures){
		ofTexture & tex = it.second;
		ofRectangle frame = ofRectangle(xx, yy, rect.width / ceil(nx), rect.height / ceil(ny));
		ofRectangle paddedFrame = frame;
		paddedFrame.x += pad;
		paddedFrame.y += pad;
		paddedFrame.width -= 2 * pad;
		paddedFrame.height -= 2 * pad;

		if(tex.isAllocated() && tex.getWidth()){
			ofRectangle texR = ofRectangle(0,0,tex.getWidth(), tex.getHeight());
			texR.scaleTo(paddedFrame, OF_ASPECT_RATIO_KEEP, OF_ALIGN_HORZ_CENTER, OF_ALIGN_VERT_TOP);
			tex.draw(texR);
			ofDrawBitmapString(loadedUrls[it.first], texR.x + 4 - 1, texR.getBottom() + 14);
		}
		xx += ceil(frame.width);
		if(xx >= rect.x + rect.width){
			yy += frame.height;
			xx = rect.x;
		}
	}

	chromes.drawStatus(15,15);
}


void ofApp::onChromeReady(ofxChromePool& who){
	int numToLoad = chromes.getNumInstances();

	ofVec2f winSize = ofVec2f(ofRandom(800, 1400), ofRandom(600, 1500));

	for(int i = 0; i < numToLoad; i++){
		string url = urls[(int)floor(ofRandom(urls.size()))];
		chromes.loadPage(url, winSize);
	}
}


void ofApp::onPixelsReady(ofxChrome::PagePixels& data){

	ofLogNotice("ofxChrome") << "pixels ready : " << data.pixels.getWidth() << " x " << data.pixels.getHeight();

	ofDisableArbTex();
	textures[data.who].clear();
	textures[data.who].loadData(data.pixels);
	textures[data.who].generateMipmap();
	textures[data.who].enableMipmap();
	textures[data.who].setTextureMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

	loadedUrls[data.who] = data.url;

	//load another one
	string url = urls[(int)floor(ofRandom(urls.size()))];

	ofVec2f winSize = ofVec2f(ofRandom(800, 1400), ofRandom(600, 1500));
	chromes.loadPage(url, winSize); //schedule another load

}


void ofApp::keyPressed(int key){

	ofVec2f winSize = ofVec2f(ofRandom(800, 1400), ofRandom(600, 1500));

	if(key=='1'){
		chromes.loadPage("http://uri.cat", winSize, 10, true);
	}

	if(key=='2'){
		chromes.loadPage("http://github.com", winSize);
	}

	if(key=='3'){
		chromes.loadPage("http://apple.com", winSize);
	}

	if(key=='4'){
		chromes.loadPage("https://www.youtube.com", winSize);
	}

	if(key=='5'){
		chromes.loadPage("http://yahoo.com/", winSize);
	}

	if(key=='6'){
		string html = "<html><body>It's this time in OpenFrameworks: " + ofGetTimestampString() +  "</body></html>";
		chromes.loadHTML(html, winSize);
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
