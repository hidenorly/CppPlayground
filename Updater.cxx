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

// clang++ -std=c++20 Updater.cxx 

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


class MockConstants
{
public:
  constexpr static int DUMMY_SIZE = 1024*1024;
};


#if USE_PLUGIN
#else // USE_PLUGIN
// --- Mock impl. of primitive updater (impl. of IConcreteUpdateHal)
class ConcreteUpdateHalMockImpl : public IConcreteUpdateHal
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

class ConcreteUpdateHalMockImpl2 : public ConcreteUpdateHalMockImpl
{
protected:
  virtual void setUpDummyData(){
    std::map<std::string, std::string> dummyMeta;
    dummyMeta[META_VERSION] = "1111";
    dummyMeta[META_HASH] = "deadbeafdeadbeaf";

    mDummyData["adc_system"] = dummyMeta;
    mDummyData["adc_vendor"] = dummyMeta;
    mDummyData["adc_product"] = dummyMeta;
    mDummyData["adc_oem"] = dummyMeta;
    mDummyData["adcodm"] = dummyMeta;
  }

public:
  ConcreteUpdateHalMockImpl2() {
    setUpDummyData();
  }
  virtual ~ConcreteUpdateHalMockImpl2() = default;
};
#endif // USE_PLUGIN

// --- Mock impl. of HAL (based on UpdateInstallHalImpl)
class UpdateInstallHalMockImpl : public UpdateInstallHalImpl
{
protected:
  std::vector<std::shared_ptr<IConcreteUpdateHal>> mMockImpls;
  std::map<std::string, std::shared_ptr<IConcreteUpdateHal>> mMockIdImpls;

public:
  UpdateInstallHalMockImpl(){
#if USE_PLUGIN
#else // USE_PLUGIN
    mMockImpls.push_back( std::make_shared<ConcreteUpdateHalMockImpl>() );
    mMockImpls.push_back( std::make_shared<ConcreteUpdateHalMockImpl2>() );
#endif // USE_PLUGIN

    for( auto& impl : mMockImpls ){
      auto ids = impl->getSupportedIds();
      for( auto& id : ids ){
        mMockIdImpls[ id ] = impl;
      }
    }
  }

  virtual ~UpdateInstallHalMockImpl() = default;

  virtual std::vector<std::string> getSupportedIds(){
    std::vector<std::string> ids;
    for( auto& [id, impl] : mMockIdImpls ){
      ids.push_back( id );
    }
    return ids;
  }

  virtual std::map<std::string, std::string> getMetaDataById(std::string id){
    std::map<std::string, std::string> metadata;
    if( mMockIdImpls.contains(id) ){
      return mMockIdImpls[id]->getMetaDataById(id);
    } else {
      throwBadId(id);
    }
    return metadata;
  }

  // validate the written image
  virtual void validate(std::string id, COMPLETION_CALLBACK completion){
    if( mMockIdImpls.contains(id) ){
      return mMockIdImpls[id]->validate(id, completion);
    } else {
      throwBadId(id);
    }
  }

  // Set Active for next (the written firmware will be applied without invoking this if the subsystem doesn't support A/B)
  virtual void activateForNext(std::string id, COMPLETION_CALLBACK completion){
    if( mMockIdImpls.contains(id) ){
      return mMockIdImpls[id]->activateForNext(id, completion);
    } else {
      throwBadId(id);
    }
  }

  // optional method. If you'd like to apply immediately and if the subsystem support runtime reboot.
  virtual void restartAndWaitToBoot(std::string id, COMPLETION_CALLBACK completion){
    if( mMockIdImpls.contains(id) ){
      return mMockIdImpls[id]->restartAndWaitToBoot(id, completion);
    } else {
      throwBadId(id);
    }
  }


  virtual std::shared_ptr<IUpdateSession> startUpdateSession(std::string id, COMPLETION_CALLBACK completion, IUpdateSession::UpdateType type = IUpdateSession::UpdateType::FULL){
    if( mMockIdImpls.contains(id) ){
      return mMockIdImpls[id]->startUpdateSession(id, completion, type);
    } else {
      throwBadId(id);
    }
    return nullptr;
  }
};



std::shared_ptr<IUpdateInstallHal> UpdateInstallHalFactory::getInstance()
{
  if(!mInstance){
    mInstance = std::make_shared<UpdateInstallHalMockImpl>();
  }
  return mInstance;
}

std::shared_ptr<IUpdateInstallHal> UpdateInstallHalFactory::mInstance;


int main(int argc, char** argv) {
  std::shared_ptr<IUpdateInstallHal> hal = UpdateInstallHalFactory::getInstance();

  std::vector<std::string> updateTargets = hal->getSupportedIds();
  for(auto& id : updateTargets){
    std::cout << id << std::endl;
  }

  std::map<std::string, std::shared_ptr<IUpdateSession>> sessions;

  // completion handler for restart the subsystem to apply immediately
  IUpdateInstallHal::COMPLETION_CALLBACK restartCompletion = [&](std::string id, bool isSuccessfullyDone){
    std::cout << "RestartCompletion::id=" << id << " : " << (isSuccessfullyDone ? "Completed" : "Not Completed") << std::endl;
  };

  // completion handler for validation for written image
  IUpdateInstallHal::COMPLETION_CALLBACK activateCompletion = [&](std::string id, bool isSuccessfullyDone){
    std::cout << "ActivateCompletion::id=" << id << " : " << (isSuccessfullyDone ? "Completed" : "Not Completed") << std::endl;

    std::cout << "Request restart : " << id << std::endl;
    hal->restartAndWaitToBoot(id, restartCompletion);
  };

  // completion handler for validation for written image
  IUpdateInstallHal::COMPLETION_CALLBACK validateCompletion = [&](std::string id, bool isSuccessfullyDone){
    std::cout << "ValidateCompletion::id=" << id << " : " << (isSuccessfullyDone ? "Completed" : "Not Completed") << std::endl;

    std::cout << "Request Activate : " << id << std::endl;
    hal->activateForNext(id, activateCompletion);
  };

  // Session for writing new image
  IUpdateInstallHal::COMPLETION_CALLBACK writeCompletion = [&](std::string id, bool isSuccessfullyDone){
    std::cout << "WriteCompletion::id=" << id << " : " << (isSuccessfullyDone ? "Completed" : "Not Completed") << std::endl;

    std::cout << "Request Validate : " << id << std::endl;
    hal->validate(id, validateCompletion);
  };

  for(auto& id : updateTargets){
    std::cout << id << std::endl;
    std::map<std::string, std::string> theMeta = hal->getMetaDataById(id);
    for( auto& [key, value] : theMeta ){
      std::cout << "\t" << key << ":" << value << std::endl;
    }
    sessions[id] = hal->startUpdateSession(id, writeCompletion);
  }

  for(int i=0; i<4; i++){
    for(auto& [id, session] : sessions){
      std::vector<uint8_t> chunk(MockConstants::DUMMY_SIZE/4);
      session->write(chunk);
      std::cout << "id=" << id << " progress=" << std::to_string(session->getProgressPercent()) << std::endl;
    }
  }

  // test for over-written case. expect to catch IllegalInvocationException
  std::string id_for_test; 
  for(auto& [id, session] : sessions){
    id_for_test = id; // for test
    break;
  }
  try{
    if( !id_for_test.empty() ){
      std::vector<uint8_t> chunk(MockConstants::DUMMY_SIZE/4);
      sessions[id_for_test]->write(chunk);
    }
  } catch (BaseException& ex){
    ex.dump();
  }

  return 0;
}