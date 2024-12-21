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

typedef int Frame; // tentative

class FrameBuffer
{
protected:
  float mSamplingRatePerSecond;
  std::vector< std::pair<std::chrono::system_clock::time_point, Frame> > mFrames;
  int mFramePos;
  std::chrono::system_clock::time_point mLastTime;
  std::chrono::milliseconds mFrameDurationChronoMs;
  std::mutex mMutex;

public:
  FrameBuffer(float nSamplingRatePerSecond=1.0f):mSamplingRatePerSecond(nSamplingRatePerSecond),mFramePos(0)
  {
    mLastTime = std::chrono::system_clock::now();
    mFrameDurationChronoMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::duration<double, std::milli>(1000.0f/nSamplingRatePerSecond));
  }

  virtual ~FrameBuffer(){

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
    mLastTime = current;
  }

  bool isEmpty(){
    std::lock_guard<std::mutex> lock(mMutex);
    return mFrames.empty();
  }

  Frame dequeueFrame(std::chrono::system_clock::time_point nPTS = std::chrono::system_clock::from_time_t(0)){
    std::lock_guard<std::mutex> lock(mMutex);
    Frame result;
    if( !mFrames.empty() ){
      if(nPTS == std::chrono::system_clock::from_time_t(0)){
        result = mFrames.front().second;
        mFrames.erase(mFrames.begin());        
      } else {
        int nCount = 0;
        bool found = false;
        for(auto& frame: mFrames){
          if(frame.first>=nPTS){
            result = frame.second;
            found = true;
            break;
          }
          nCount++;
        }
        if( found ){
          mFrames.erase(mFrames.begin(), mFrames.begin()+nCount);
        } else {
          mFrames.erase(mFrames.begin(), mFrames.end());
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


int main()
{
  double fps = 60.0f;
  auto frameDurationChronoMs = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::duration<double, std::milli>(1000.0f/fps));
  auto startPos = std::chrono::system_clock::now();
  std::vector<Frame> frames = { 0, 1, 2, 3, 4 };

  // test case 1 - simple in/out
  FrameBuffer buffers;
  buffers.enqueueFrames( frames );

  while( !buffers.isEmpty() ){
    std::cout << buffers.dequeueFrame() << std::endl;
  }

  // test case 2 - with fps and pts
  auto current = startPos;
  FrameBuffer buffers2(fps);

  buffers2.enqueueFrames( frames );
  current += frameDurationChronoMs;
  try{
    while( !buffers2.isEmpty() ){
      std::cout << buffers2.dequeueFrame(current) << std::endl;
      current += frameDurationChronoMs;
    }
  } catch (const std::invalid_argument& e) {
    std::cout << e.what() << std::endl;
  }

  // test case 3 - frames with fps and pts
  FrameBuffer buffers3(fps);

  buffers3.enqueueFrames( frames );
  std::vector<Frame> frames3 = buffers3.dequeueFrames( startPos, startPos+frameDurationChronoMs*5, frameDurationChronoMs);
  for(auto& frame : frames3){
      std::cout << frame << std::endl;
  }

  return 0;
}