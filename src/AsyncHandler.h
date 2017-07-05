//
//  AsyncHandler.h
//  BaseApp
//
//  Created by Oriol Ferrer Mesià on 26/06/2017.
//
//

#pragma once
#include "ofMain.h"
#include <future>

template <class A, class R>
class AsyncHandler{

public:

	AsyncHandler(){
		ofAddListener(ofEvents().update, this, &AsyncHandler::update);
	};

	~AsyncHandler(){
		ofRemoveListener(ofEvents().update, this, &AsyncHandler::update);
	};


	//provide:
 	//	1 - your task,
	//	2 - your task argument,
	// 	3 - a func to handle what to do when task is done
	bool startTask(std::function<R(const A & )> taskFunc,
				   const A & taskArg,
				   std::function<void(const R &)> asyncResultReadyFunc){

		if(state == IDLE){
			future = std::async(std::launch::async, taskFunc, taskArg);
			state = BUSY;
			this->asyncResultReadyFunc = asyncResultReadyFunc;
			//ofLogNotice("AsyncHandler") << "starting task";
			return true;
		}else{
			//ofLogError("AsyncHandler") << "can't do task - busy!";
			return false;
		}
	}


	void update(ofEventArgs & ){
		if(isReady()){
			ofLogNotice("AsyncHandler") << "task ready";
			asyncResultReadyFunc(future.get());
		}
	}

	bool isBusy(){return state == BUSY;}
	bool isDone(){return state == DONE;}

protected:

	std::future<R> future;

	bool isReady(){
		if(future.valid()){
			std::future_status status = future.wait_for(std::chrono::microseconds(0));
			if(status == std::future_status::ready){
				return true;
			}
		}
		return false;
	}

	enum State{
		IDLE,
		BUSY,
		DONE
	};

	std::function<void(const R &)> asyncResultReadyFunc;
	State state = IDLE;
};

