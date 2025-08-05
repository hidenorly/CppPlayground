/*
  Copyright (C) 2025 hidenorly

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

// clang++ -std=c++20 Adapter.cxx

#include <iostream>
#include <memory>
#include <string>


class IAudioService
{
public:
  virtual float getVolume(void) = 0;
  virtual void setVolume(float volume) = 0;
  virtual void mute(void) = 0;
  virtual void unmute(void) = 0;
};


class AudioHalMockService
{
protected:
  float mVolume = 0.0f;

public:
  AudioHalMockService(){};
  virtual ~AudioHalMockService() = default;

  virtual float getMockVolume(void){
    return mVolume;
  }

  virtual void setMockVolume(float volume){
    if( volume < 0.0f ) volume = 0.0f;
    if( volume > 1.0f ) volume = 1.0f;
    mVolume = volume;
  }
};

class MockAudioServiceAdapter : public IAudioService
{
protected:
  std::shared_ptr<AudioHalMockService> mService;
  float mVolume;
  bool isMuted;

public:
  MockAudioServiceAdapter():mVolume(0.0f),isMuted(false){
    mService = std::make_shared<AudioHalMockService>();
  };

  virtual ~MockAudioServiceAdapter() = default;

  virtual void setVolume(float volume){
    if( isMuted ) return;
    if( volume < 0.0f ) volume = 0.0f;
    if( volume > 1.0f ) volume = 1.0f;
    mVolume = volume;
    if( mService ){
      mService->setMockVolume(mVolume);
    }
  }

  virtual float getVolume(void){
    if( isMuted ) return 0.0f;
    if( mService ){
      return mService->getMockVolume();
    }
    return mVolume;
  }

  void mute(void){
    if( mService ){
      mService->setMockVolume(0.0f);
    }
    isMuted = true;
  }

  void unmute(void){
    if( mService ){
      mService->setMockVolume(mVolume); // get back to last volume
    }
    isMuted = false;
  }
};




int main() {
  std::shared_ptr<IAudioService> audio = std::make_shared<MockAudioServiceAdapter>();
  std::cout << std::to_string(audio->getVolume()) << std::endl;
  audio->setVolume(100.0f);
  std::cout << std::to_string(audio->getVolume()) << std::endl;
  audio->mute();
  std::cout << std::to_string(audio->getVolume()) << std::endl;
  audio->unmute();
  std::cout << std::to_string(audio->getVolume()) << std::endl;
}