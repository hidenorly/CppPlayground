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
  auto current = startPos;

  auto producer = [&]() {
      for (int i = 0; i < 100; ++i) {
        buffers.enqueueFrames( std::vector<Frame>(i) );
        std::this_thread::sleep_for(frameDurationChronoMs/2);
      }
  };

  auto consumer = [&]() {
      try{
        while( !buffers.isEmpty() ){
          std::this_thread::sleep_for(frameDurationChronoMs);
          std::cout << buffers.dequeueFrame(current) << std::endl;
          current += frameDurationChronoMs;
        }
      } catch (const std::invalid_argument& e) {
        std::cout << e.what() << std::endl;
      }
  };

  std::thread producerThread(producer);
  std::thread consumerThread(consumer);

  producerThread.join();
  consumerThread.join();
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
  test_parallel();


  return 0;
}