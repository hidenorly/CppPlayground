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

#include "SharedFrameBuffer.hpp"
#include <chrono>
#include <stdexcept>
#include <thread>

typedef int Frame; // tentative
using SharedFrameBuffer = TSharedFrameBuffer<Frame>;

using PSharedFrameBuffer = TSharedFrameBuffer<std::shared_ptr<Frame>>;


void test_parallel(void)
{
  double fps = 60.0f;
  SharedFrameBuffer buffers(fps, 100);
  auto frameDurationChronoMs = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::duration<double, std::milli>(1000.0f/fps));
  auto startPos = std::chrono::system_clock::now();

  auto producer = [&]() {
    for (int i = 0; i < 100; i++) {
      buffers.enqueueFrames( std::vector<Frame>(i) );
      std::this_thread::sleep_for(frameDurationChronoMs);
    }
  };

  auto consumer = [&]() {
    auto current = startPos;
    int i=0;
    int j=0;
    std::this_thread::sleep_for(frameDurationChronoMs);
    try{
      do {
        Frame frame = buffers.dequeueFrame(current);
        std::cout << frame << std::endl;
        if( i == frame ){
          i++;
        }
        current += frameDurationChronoMs;
        std::this_thread::sleep_for(frameDurationChronoMs);
        j++;
        if(j>100) break;
      } while( !buffers.isEmpty() );
    } catch (const std::invalid_argument& e) {
      std::cout << e.what() << std::endl;
    }
    std::cout << "matched count :" << i << std::endl;
  };

  std::thread producerThread(producer);
  std::this_thread::sleep_for(frameDurationChronoMs/3);
  std::thread consumerThread(consumer);
  std::thread consumerThread2(consumer);

  producerThread.join();
  consumerThread.join();
  consumerThread2.join();
}


int main()
{
  double fps = 60.0f;
  auto frameDurationChronoMs = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::duration<double, std::milli>(1000.0f/fps));
  auto startPos = std::chrono::system_clock::now();
  std::vector<Frame> frames = { 0, 1, 2, 3, 4 };

  auto current = startPos;

  // test case 5
  std::cout << "test case5" << std::endl;
  current = startPos;
  SharedFrameBuffer buffers5(fps, 10);

  buffers5.enqueueFrames( frames );
  buffers5.enqueueFrames( frames );
  current += frameDurationChronoMs;
  try{
    while( !buffers5.isEmpty() ){
      std::cout << buffers5.dequeueFrame(current) << std::endl;
      current += frameDurationChronoMs;
    }
  } catch (const std::invalid_argument& e) {
    std::cout << e.what() << std::endl;
  }

  current = startPos;
  try{
    while( !buffers5.isEmpty() ){
      std::cout << buffers5.dequeueFrame(current) << std::endl;
      current += frameDurationChronoMs;
    }
  } catch (const std::invalid_argument& e) {
    std::cout << e.what() << std::endl;
  }

  // test case 6
  std::cout << "test case6" << std::endl;
  current = startPos;
  std::vector<std::shared_ptr<Frame>> pframes;
  for(auto frame : frames){
    pframes.push_back( std::make_shared<Frame>(frame) );
  }
  PSharedFrameBuffer buffers6(fps, 10);
  buffers6.enqueueFrames( pframes );
  buffers6.enqueueFrames( pframes );

  current += frameDurationChronoMs;
  try{
    while( !buffers6.isEmpty() ){
      std::cout << *(buffers6.dequeueFrame(current)) << std::endl;
      current += frameDurationChronoMs;
    }
  } catch (const std::invalid_argument& e) {
    std::cout << e.what() << std::endl;
  }

  current = startPos;
  try{
    while( !buffers6.isEmpty() ){
      std::cout << *(buffers6.dequeueFrame(current)) << std::endl;
      current += frameDurationChronoMs;
    }
  } catch (const std::invalid_argument& e) {
    std::cout << e.what() << std::endl;
  }

  //test case 7
  std::cout << "test case7" << std::endl;
  test_parallel();


  return 0;
}