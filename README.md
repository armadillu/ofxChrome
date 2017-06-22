# ofxChrome

This attempts to run the Google Chrome web browser in [headless mode](https://developers.google.com/web/updates/2017/04/headless-chrome) and controlling it through their [DevTools](https://chromedevtools.github.io/devtools-protocol/tot/). This implies launching Chrome with the debugger enabled (```--remote-debugging-port=9222```), sending an http request to ```http://chrome-ip:9222/json``` to get a list of open pages to get a WebSocket @ to connect to, and connecting to that WebSocket to take control of the browser.

One of the features I'm shooting for is being able to get "snapshots" from the browser.
You can do that through their DevTools API; you can request a capture and it returns a base64 encoded png/jpeg of the browser window.

This is very much a WIP and the API will change.

# Dependencies
ofxPoco, [ofxExternalProcess](http://github.com/armadillu/ofxExternalProcess), [ofxLibwebsockets](https://github.com/robotconscience/ofxLibwebsockets.git).

# License

```ofxChrome``` is made available under the [MIT](http://opensource.org/licenses/MIT) license.  
Chrome is of coursed owned by Google!
