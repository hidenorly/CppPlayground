/*
  Copyright (C) 2024 hidenorly

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <iostream>
#include <vector>
#include <utility>
#include <chrono>
#include <stdexcept>
#include <mutex>

template<typename Frame> class SharedFrameBuffer
{
protected:
  float mSamplingRatePerSecond;
  std::vector< std::pair<std::chrono::system_clock::time_point, Frame> > mFrames;
  int mFramePos;
  int mStoringSize;
  std::chrono::system_clock::time_point mLastTime;
  std::chrono::milliseconds mFrameDurationChronoMs;
  std::mutex mMutex;

public:
  SharedFrameBuffer(float nSamplingRatePerSecond=60.0f, int storingSize=0 /* infinite */):mSamplingRatePerSecond(nSamplingRatePerSecond),mFramePos(0), mStoringSize(storingSize){
    mLastTime = std::chrono::system_clock::now();
    mFrameDurationChronoMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::duration<double, std::milli>(1000.0f/nSamplingRatePerSecond));
  }

  virtual ~SharedFrameBuffer(){

  }

  void enqueueFrames(const std::vector<Frame>& frames){
    std::lock_guard<std::mutex> lock(mMutex);
    auto now = std::chrono::system_clock::now();
    auto current = mLastTime;
    if(now < current){
      std::cout << "Feed is larger than consuming" << std::endl;
    }
    for(auto frame : frames){
      mFrames.push_back(std::make_pair(current, frame));
      mFramePos++;
      current += mFrameDurationChronoMs;
    }
    if( mStoringSize && mFrames.size() > mStoringSize ){
      // mFrames.erase(mFrames.begin(), mFrames.begin()+(mStoringSize-mFrames.size()));
      for(int i=0; i<mStoringSize-mFrames.size(); i++){
        mFrames.erase(mFrames.begin()); 
      }
    }
    mLastTime = current;
  }

  bool isEmpty(std::chrono::system_clock::time_point nPTS = std::chrono::system_clock::from_time_t(0)){
    std::lock_guard<std::mutex> lock(mMutex);
    if( nPTS == std::chrono::system_clock::from_time_t(0) ){
      return mFrames.empty();
    } else {
      for(auto& frame: mFrames){
        if(frame.first>=nPTS){
          return false;
        }
      }
    }

    return true;
  }

  Frame dequeueFrame(std::chrono::system_clock::time_point nPTS = std::chrono::system_clock::from_time_t(0)){
    std::lock_guard<std::mutex> lock(mMutex);
    Frame result;
    if( !mFrames.empty() ){
      if(nPTS == std::chrono::system_clock::from_time_t(0)){
        result = mFrames.front().second;
      } else {
        bool found = false;
        for(auto& frame: mFrames){
          if(frame.first>=nPTS){
            result = frame.second;
            found = true;
            break;
          }
        }
        if( !found ){
          throw std::invalid_argument("wrong pts");;
        }
      }
    }
    return result;
  }

 std::vector<Frame> dequeueFrames(
  std::chrono::system_clock::time_point startPTS = std::chrono::system_clock::from_time_t(0), 
  std::chrono::system_clock::time_point endPTS = std::chrono::system_clock::from_time_t(0), 
  std::chrono::milliseconds durationMilliSeconds =  
       std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double, std::milli>(1000.0f/60.0f))){
    std::vector<Frame> result;
    for(std::chrono::system_clock::time_point pos = startPTS; pos<endPTS; pos=pos+durationMilliSeconds){
      try {
        result.push_back( dequeueFrame(pos) );
      } catch (const std::invalid_argument& e) {
        break;
      }
    }
    return result;
  }

};