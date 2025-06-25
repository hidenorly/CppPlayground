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

enum class ExceptionSeverity {
    Recoverable,
    Critical
};

class BaseException : public std::exception {
protected:
    std::string mMsg;
    ExceptionSeverity mSeverity;
    std::source_location mLocation;


public:
    BaseException(std::string_view message,
                  ExceptionSeverity severity,
                  std::source_location location = std::source_location::current())
        : mMsg(message),
          mSeverity(severity),
          mLocation(location) {}

    const char* what() const noexcept override {
        return mMsg.c_str();
    }

    ExceptionSeverity severity() const noexcept {
        return mSeverity;
    }

    bool isRecoverable() const noexcept {
        return mSeverity == ExceptionSeverity::Recoverable;
    }

    const std::source_location& location() const noexcept {
        return mLocation;
    }

  std::string demangle(const char* name) const {
    int status = 0;
    char* demangled = abi::__cxa_demangle(name, nullptr, nullptr, &status);
    std::string result = (status == 0 && demangled) ? demangled : name;
    free(demangled);
    return result;
  }

    void dump(std::ostream& os = std::cerr) const {
    os << demangle(typeid(*this).name()) << ": "
      << what() << " ("
      << (isRecoverable() ? "RECOVERABLE" : "CRITICAL") << ")\n\t"
      << location().file_name() << ':' << location().line()
      << std::endl;
    }
};

class InvalidArgumentException final : public BaseException {
public:
    explicit InvalidArgumentException(std::string_view message,
        std::source_location location = std::source_location::current())
        : BaseException(message, ExceptionSeverity::Recoverable, location) {}
};

class IllegalStateException final : public BaseException {
public:
    explicit IllegalStateException(std::string_view message,
        std::source_location location = std::source_location::current())
        : BaseException(message, ExceptionSeverity::Critical, location) {}
};



class IUpdateInstallHal
{
public:
  // enumerate supported subsystem IDs
  virtual std::vector<std::string> getSupportedIds() = 0;

  const char* META_VERSION = "version";
  const char* META_HASH = "hash";

  // get the specified subsystem's attributes
  virtual std::map<std::string, std::string> getMetaDataById(std::string id) = 0;


  // async method completion
  typedef std::function<void(std::string id, bool isSuccessfullyDone)> COMPLETION_CALLBACK;

  // session to write the new image.
  // Note that B-side(next-side) is written if supported.
  //           A-side(current) is written if B-side isn't suppoted.
  class IUpdateSession {
  public:
    virtual bool write(std::vector<uint8_t> chunk) = 0;
    virtual float getProgressPercent() = 0;

    // cancel might be failed if B-side isn't supported
    virtual bool cancel() = 0;
  };

  // create session to write the new firmware image
  virtual std::shared_ptr<IUpdateSession> startUpdateSession(std::string id, COMPLETION_CALLBACK completion) = 0;

  // validate the written image
  virtual void validate(std::string id, COMPLETION_CALLBACK completion) = 0;

  // Set Active for next (the written firmware will be applied without invoking this if the subsystem doesn't support A/B)
  virtual void activateForNext(std::string id, COMPLETION_CALLBACK completion) = 0;

  // optional method. If you'd like to apply immediately and if the subsystem support runtime reboot.
  virtual void restartAndWaitToBoot(std::string id, COMPLETION_CALLBACK completion) = 0;
};


class UpdateInstallHalImpl : public IUpdateInstallHal
{
protected:
  class UpdateSessionImpl : public IUpdateInstallHal::IUpdateSession
  {
  protected:
    const int mMaxSize;
    int mWrittenSize;
    const std::string mId;
    const COMPLETION_CALLBACK mCompletion;

  public:
    UpdateSessionImpl(const std::string id, const int nSize, const COMPLETION_CALLBACK completion):mId(id),mMaxSize(nSize), mWrittenSize(0), mCompletion(completion)
    {
    }
    virtual ~UpdateSessionImpl(){};

    virtual bool write(std::vector<uint8_t> chunk){
      mWrittenSize += chunk.size();
      bool result = mWrittenSize < mMaxSize;
      if( !result && mCompletion ){
        mCompletion(mId, !result);
      }
      return result;
    }

    virtual float getProgressPercent(){
      float progressPercent = (float)mWrittenSize/(float)mMaxSize*100.0f;
      if( progressPercent> 100.0f ){
        return 100.0f;
      }
      return progressPercent;
    }

    virtual bool cancel(){
      mWrittenSize = 0;
      return true;
    }
  };

public:
  UpdateInstallHalImpl() = default;
  virtual ~UpdateInstallHalImpl() = default;

  virtual std::vector<std::string> getSupportedIds(){
    std::vector<std::string> ids;
    return ids;
  }

  virtual std::map<std::string, std::string> getMetaDataById(std::string id){
    return std::map<std::string, std::string>({});
  }

  virtual std::shared_ptr<IUpdateSession> startUpdateSession(std::string id, COMPLETION_CALLBACK completion){
    std::shared_ptr<IUpdateSession> result = std::make_shared<UpdateSessionImpl>( id, 0, completion );
    return result;
  }

  virtual void validate(std::string id, COMPLETION_CALLBACK completion){
    completion(id, true);
  }

  virtual void activateForNext(std::string id, COMPLETION_CALLBACK completion){
    completion(id, true);
  }

  virtual void restartAndWaitToBoot(std::string id, COMPLETION_CALLBACK completion){
    completion(id, true);
  }
};


class UpdateInstallHalMockImpl : public UpdateInstallHalImpl
{
protected:
  std::map<std::string, std::map<std::string, std::string>> mDummyData;

  void setUpDummyData(){
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
  };

public:
  constexpr static int DUMMY_SIZE = 1024*1024;

public:
  UpdateInstallHalMockImpl(){
    setUpDummyData();
  }
  virtual ~UpdateInstallHalMockImpl() = default;

  virtual std::vector<std::string> getSupportedIds(){
    std::vector<std::string> ids;
    for( auto& [key, val] : mDummyData ){
      ids.push_back(key);
    }
    return ids;
  }

  virtual std::map<std::string, std::string> getMetaDataById(std::string id){
    if( mDummyData.contains(id) ){
      return mDummyData[id];
    }
    return std::map<std::string, std::string>({});
  }

  virtual std::shared_ptr<IUpdateSession> startUpdateSession(std::string id, COMPLETION_CALLBACK completion){
    std::shared_ptr<IUpdateSession> result = std::make_shared<UpdateSessionImpl>( id, DUMMY_SIZE, completion );
    return result;
  }
};


class UpdateInstallHalFactory
{
protected:
  static std::shared_ptr<IUpdateInstallHal> mInstance;

public:
  static std::shared_ptr<IUpdateInstallHal> getInstance(){
    if(!mInstance){
      mInstance = std::make_shared<UpdateInstallHalMockImpl>();
    }
    return mInstance;
  }
};

std::shared_ptr<IUpdateInstallHal> UpdateInstallHalFactory::mInstance;


int main(int argc, char** argv) {
  std::shared_ptr<IUpdateInstallHal> hal = UpdateInstallHalFactory::getInstance();

  std::vector<std::string> updateTargets = hal->getSupportedIds();
  std::map<std::string, std::shared_ptr<IUpdateInstallHal::IUpdateSession>> sessions;

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
      std::vector<uint8_t> chunk(UpdateInstallHalMockImpl::DUMMY_SIZE/4);
      session->write(chunk);
      std::cout << "id=" << id << " progress=" << std::to_string(session->getProgressPercent()) << std::endl;
    }
  }

  return 0;
}