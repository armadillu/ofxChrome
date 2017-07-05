#pragma once

#include "ofMain.h"
#include "ofxChromePool.h"

#define TIME_PROFILE //comment this out to avoid the ofxTimeMeasurements dependency

#ifdef TIME_PROFILE
	#include "ofxTimeMeasurements.h"
#endif


class ofApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);


	ofxChromePool chromes;

	map<ofxChrome*,ofTexture> textures;
	map<ofxChrome*,string> loadedUrls;

	void onPixelsReady(ofxChrome::PagePixels& data);
	void onChromeReady(ofxChromePool&);

	vector<string> urls;
};
