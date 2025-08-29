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

// clang++ -std=c++20 -flat_namespace -dynamiclib MockUpdaterPlugIn.cxx -ldl -o libmockupdater_plugin.dylib -I ../../plugin-manager/include 

#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include <memory>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <source_location>
#include <sstream>
#include <cxxabi.h>

#define USE_PLUGIN 1
#include "Updater.hpp"
#if USE_PLUGIN
// git clone https://github.com/hidenorly/plugin-manager.git
#include "../../plugin-manager/src/PlugInManager.cpp"
#endif // USE_PLUGIN



class MockConstants
{
public:
  constexpr static int DUMMY_SIZE = 1024*1024;
};


// --- Mock impl. of primitive updater (impl. of IConcreteUpdateHal)
class ConcreteUpdateHalMockImpl : public UpdaterPlugInBase
{
protected:
  std::map<std::string, std::map<std::string, std::string>> mDummyData;
  std::map<std::string, std::shared_ptr<IUpdateSession>> mSessions;

  virtual void setUpDummyData(){
    std::map<std::string, std::string> dummyMeta;
    dummyMeta[META_VERSION] = "0000";
    dummyMeta[META_HASH] = "0123456789abcdef";

    mDummyData["system"] = dummyMeta;
    mDummyData["vendor"] = dummyMeta;
    mDummyData["product"] = dummyMeta;
    mDummyData["oem"] = dummyMeta;
    mDummyData["odm"] = dummyMeta;

    std::map<std::string, std::string> dummyMeta2;
    dummyMeta2[META_VERSION] = "1234";
    dummyMeta2[META_HASH] = "abcdef0123456789";
    mDummyData["mcu_1"] = dummyMeta2;
  }

public:
  ConcreteUpdateHalMockImpl(){
    setUpDummyData();
  }
  virtual ~ConcreteUpdateHalMockImpl() = default;


  virtual std::string toString(void){ return "ConcreteUpdateHalMockImpl"; };

  /* @desc initialize at loading the  plug-in shared object such as .so */
  virtual void onLoad(void){
    std::cout << "ConcreteUpdateHalMockImpl::onLoad" << std::endl;
  }
  /* @desc uninitialize at unloading the  plug-in shared object such as .so */
  virtual void onUnload(void){
    std::cout << "ConcreteUpdateHalMockImpl::onUnload" << std::endl;
  }
  /* @desc report your plug-in's unique id
     @return unique plug-in id. may use uuid. */
  virtual std::string getId(void){
    return std::string("ConcreteUpdateHalMockImpl");
  }
  /* @desc this is expected to use by strategy
     @return new YourPlugIn's result */
  virtual IPlugIn* newInstance(void){
    std::cout << "ConcreteUpdateHalMockImpl::newInstance" << std::endl;
    return new ConcreteUpdateHalMockImpl();
  }

  // enumerate supported subsystem IDs
  virtual std::vector<std::string> getSupportedIds(){
    std::vector<std::string> ids;
    for( auto& [key, val] : mDummyData ){
      ids.push_back(key);
    }
    return ids;
  }

  // get the specified subsystem's attributes
  virtual std::map<std::string, std::string> getMetaDataById(std::string id){
    if( mDummyData.contains(id) ){
      return mDummyData[id];
    } else {
      throwBadId(id);
    }
    return std::map<std::string, std::string>({});
  }

  // validate the written image
  virtual void validate(std::string id, COMPLETION_CALLBACK completion){
    if( mDummyData.contains(id) ){
      completion(id, true);
    } else {
      throwBadId(id);
    }
  }

  // Set Active for next (the written firmware will be applied without invoking this if the subsystem doesn't support A/B)
  virtual void activateForNext(std::string id, COMPLETION_CALLBACK completion){
    if( mDummyData.contains(id) ){
      completion(id, true);
    } else {
      throwBadId(id);
    }
  }

  // optional method. If you'd like to apply immediately and if the subsystem support runtime reboot.
  virtual void restartAndWaitToBoot(std::string id, COMPLETION_CALLBACK completion){
    if( mDummyData.contains(id) ){
      completion(id, true);
    } else {
      throwBadId(id);
    }
  }

  // --- for session
  virtual bool canStartUpdateSession(std::string id, IUpdateSession::UpdateType type = IUpdateSession::UpdateType::FULL){

    // this "mock" only allows FULL update
    return (type == IUpdateSession::UpdateType::FULL);
  }

  virtual bool write(std::string id, std::vector<uint8_t> chunk){
    bool result = false;
    if( mSessions.contains(id) && mSessions[id] ){
      result = mSessions[id]->write(chunk);
    }
    return result;
  }

  virtual float getProgressPercent(std::string id){
    float result = 100.0f;
    if( mSessions.contains(id) && mSessions[id] ){
      result = mSessions[id]->getProgressPercent();
    }
    return result;
  }

  // cancel might be failed if B-side isn't supported
  virtual bool cancel(std::string id){
    bool result = false;
    if( mSessions.contains(id) && mSessions[id] ){
      result = mSessions[id]->cancel();
    }
    return result;
  }

  virtual std::shared_ptr<IUpdateSession> startUpdateSession(std::string id, IUpdateCore::COMPLETION_CALLBACK completion = nullptr, IUpdateSession::UpdateType type = IUpdateSession::UpdateType::FULL){
    if( mDummyData.contains(id) ){
      return std::make_shared<UpdateSessionImpl>( id, MockConstants::DUMMY_SIZE, completion, nullptr, type );
    } else {
      throwBadId(id);
    }
    return nullptr;
  }
};



extern "C"
{
__attribute__ ((visibility ("default"))) void* getPlugInInstance(void)
{
  ConcreteUpdateHalMockImpl* pInstance = new ConcreteUpdateHalMockImpl();
  return reinterpret_cast<void*>(dynamic_cast<IPlugIn*>(pInstance));
}
};

