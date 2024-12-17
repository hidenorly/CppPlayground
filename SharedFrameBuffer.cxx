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

typedef int Frame; // tentative

class FrameBuffer
{
protected:
  float mSamplingRatePerSecond;
  std::vector<std::pair<int, Frame>> mFrames;
  int mFramePos;

public:
  FrameBuffer(float nSamplingRatePerSecond=1.0f):mSamplingRatePerSecond(nSamplingRatePerSecond),mFramePos(0)
  {

  }

  virtual ~FrameBuffer(){

  }

  void enqueueFrames(const std::vector<Frame>& frames){
    // TODO: frame number to PTS
    for(auto frame : frames){
      mFrames.push_back(std::make_pair(mFramePos++, frame));
    }
  }

  bool isEmpty(){
    return mFrames.empty();
  }

  Frame dequeueFrame(int nPTS=0){
    // TODO: search frame from PTS
    Frame result;
    if( !mFrames.empty() ){
      result = mFrames.front().second;
      mFrames.erase(mFrames.begin());
    }
    return result;
  }
};


int main()
{
  std::vector<Frame> frames = { 0, 1, 2, 3, 4 };
  FrameBuffer buffers;
  buffers.enqueueFrames( frames );

  while( !buffers.isEmpty() ){
    std::cout << buffers.dequeueFrame() << std::endl;
  }

  return 0;
}