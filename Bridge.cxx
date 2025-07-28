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

// clang++ -std=c++20 Bridge.cxx

#include <iostream>
#include <memory>
#include <string>


class IAudioHal
{
public:
  virtual float getVolume(void) = 0;
  virtual void setVolume(float volume) = 0;
};

class AudioHalMock : public IAudioHal
{
protected:
  float mVolume = 0.0f;

public:
  AudioHalMock(){};
  virtual ~AudioHalMock() = default;

  virtual float getVolume(void){
    return mVolume;
  }

  virtual void setVolume(float volume){
    if( volume < 0.0f ) volume = 0.0f;
    if( volume > 1.0f ) volume = 1.0f;
    mVolume = volume;
  }
};

class AudioHalFactory
{
public:
  static std::shared_ptr<IAudioHal> getAudioHalInstance(){
    return std::make_shared<AudioHalMock>();
  }
};

class AudioFramework
{
protected:
  std::shared_ptr<IAudioHal> mHal;
  float mVolume;

  void volumeDelta(float delta){
    if( mHal ){
      mVolume = mHal->getVolume();
      mHal->setVolume(mVolume + delta );
      mVolume = mHal->getVolume();
    }
  }

public:
  AudioFramework():mVolume(0.0f){
    mHal = AudioHalFactory::getAudioHalInstance();
  };

  virtual ~AudioFramework() = default;

  float getVolume(void){
    if( mHal ){
      return mHal->getVolume();
    }
    return mVolume;
  }


  void volumeUp(void){
    volumeDelta(0.1f);
  }

  void volumeDown(void){
    volumeDelta(-0.1f);
  }

  bool isMuted(void){
    if( mHal ){
      float volume = mHal->getVolume(); // don't set it to mVolume
      return volume <= 0.0f;
    }
    return true;
  }

  void mute(void){
    if( mHal ){
      mHal->setVolume(0.0f); // don't set it to mVolume
    }
  }

  void unmute(void){
    if( mHal ){
      mHal->setVolume(mVolume); // don't set it to mVolume
    }
  }
};

int main() {
  std::shared_ptr<AudioFramework> audio = std::make_shared<AudioFramework>();
  std::cout << std::to_string(audio->getVolume()) << (audio->isMuted() ? " muted" : " not muted") << std::endl;
  audio->volumeUp();
  std::cout << std::to_string(audio->getVolume()) << (audio->isMuted() ? " muted" : " not muted") << std::endl;
  audio->volumeUp();
  audio->volumeUp();
  audio->volumeUp();
  audio->volumeUp();
  audio->volumeUp();
  audio->volumeUp();
  audio->volumeUp();
  audio->volumeUp();
  audio->volumeUp();
  audio->volumeUp();
  std::cout << std::to_string(audio->getVolume()) << (audio->isMuted() ? " muted" : " not muted") << std::endl;
  audio->volumeDown();
  std::cout << std::to_string(audio->getVolume()) << (audio->isMuted() ? " muted" : " not muted") << std::endl;
  audio->mute();
  std::cout << std::to_string(audio->getVolume()) << (audio->isMuted() ? " muted" : " not muted") << std::endl;
  audio->unmute();
  std::cout << std::to_string(audio->getVolume()) << (audio->isMuted() ? " muted" : " not muted") << std::endl;
}