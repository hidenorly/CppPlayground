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

// --- exception definitions ---
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

class IllegalInvocationException final : public BaseException {
public:
    explicit IllegalInvocationException(std::string_view message,
        std::source_location location = std::source_location::current())
        : BaseException(message, ExceptionSeverity::Recoverable, location) {}
};

class IllegalStateException final : public BaseException {
public:
    explicit IllegalStateException(std::string_view message,
        std::source_location location = std::source_location::current())
        : BaseException(message, ExceptionSeverity::Critical, location) {}
};


// --- interface definition ---
class IUpdateCore
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

  // validate the written image
  virtual void validate(std::string id, COMPLETION_CALLBACK completion) = 0;

  // Set Active for next (the written firmware will be applied without invoking this if the subsystem doesn't support A/B)
  virtual void activateForNext(std::string id, COMPLETION_CALLBACK completion) = 0;

  // optional method. If you'd like to apply immediately and if the subsystem support runtime reboot.
  virtual void restartAndWaitToBoot(std::string id, COMPLETION_CALLBACK completion) = 0;
};

// session to write the new image.
// Note that B-side(next-side) is written if supported.
//           A-side(current) is written if B-side isn't suppoted.
class IUpdateSession {
public:
  enum UpdateType {
    FULL,
    DELTA,
  };

public:
  virtual bool write(std::vector<uint8_t> chunk) = 0;
  virtual float getProgressPercent() = 0;

  // cancel might be failed if B-side isn't supported
  virtual bool cancel() = 0;
};

// Interface for Create session
class IUpdateStartSession
{
public:
  // create session to write the new firmware image
  virtual std::shared_ptr<IUpdateSession> startUpdateSession(std::string id, IUpdateCore::COMPLETION_CALLBACK completion, IUpdateSession::UpdateType type = IUpdateSession::UpdateType::FULL) = 0;
};


// --- interface definition : IUpdateInstallHal with IUpdateSession for the hal user ---
class IUpdateInstallHal : public IUpdateCore, public IUpdateStartSession
{
};

// --- interface definition : IConcreteUpdateHal for primitive updater impl. ---
class IConcreteUpdateHal : public IUpdateCore, public IUpdateStartSession
{
public:
  virtual bool write(std::string id, std::vector<uint8_t> chunk) = 0;
  virtual float getProgressPercent(std::string id) = 0;

  // cancel might be failed if B-side isn't supported
  virtual bool cancel(std::string id) = 0;
};


// UpdateSession Impl
class UpdateSessionImpl : public IUpdateSession
{
protected:
  const int mMaxSize;
  int mWrittenSize;
  const std::string mId;
  const IUpdateCore::COMPLETION_CALLBACK mCompletion;
  bool mIsCompleted;
  std::shared_ptr<IConcreteUpdateHal> mConcreteHal;
  UpdateType mType;

public:
  UpdateSessionImpl(
    const std::string id, 
    const int nSize, 
    const IUpdateCore::COMPLETION_CALLBACK completion, 
    std::shared_ptr<IConcreteUpdateHal> pConcreteHal = nullptr, 
    IUpdateSession::UpdateType type = IUpdateSession::UpdateType::FULL )
    : 
    mId(id),
    mMaxSize(nSize), 
    mWrittenSize(0), 
    mCompletion(completion), 
    mIsCompleted(false), 
    mConcreteHal(pConcreteHal),
    mType(type)
  {
  }
  virtual ~UpdateSessionImpl(){};

  virtual bool write(std::vector<uint8_t> chunk){
    mWrittenSize += chunk.size();
    bool result = mWrittenSize < mMaxSize;
    if( !result && mCompletion ){
      if( !mIsCompleted ){
        mCompletion(mId, !result);
        mIsCompleted = true;
      } else {
        throw IllegalInvocationException("Already done to update");
      }
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
    if( mIsCompleted ) return false;
    mWrittenSize = 0;
    return true;
  }
};


// --- default impl. of IUpdateInstallHal ---
//     Note that this delegates to instance of IConcreteUpdateHal
class UpdateInstallHalImpl : public IUpdateInstallHal
{
protected:
    std::map<std::string, std::shared_ptr<IConcreteUpdateHal>> mConcreteHals;
    void throwBadId(std::string id){
      std::string msg = "The id ";
      msg += id;
      msg += " isn't supported";
      throw InvalidArgumentException(msg);
    }

public:
  UpdateInstallHalImpl() = default;
  virtual ~UpdateInstallHalImpl() = default;

  virtual std::vector<std::string> getSupportedIds(){
    std::vector<std::string> ids;
    for( auto& [id, hal] : mConcreteHals ){
      ids.push_back(id);
    }
    return ids;
  }

  virtual std::map<std::string, std::string> getMetaDataById(std::string id){
    if( mConcreteHals.contains(id) && mConcreteHals[id] ){
      return mConcreteHals[id]->getMetaDataById(id);
    }
    return std::map<std::string, std::string>({});
  }

  virtual std::shared_ptr<IUpdateSession> startUpdateSession(std::string id, IUpdateCore::COMPLETION_CALLBACK completion, IUpdateSession::UpdateType type = IUpdateSession::UpdateType::FULL){
    std::shared_ptr<IUpdateSession> result;
    if( mConcreteHals.contains(id) && mConcreteHals[id] ){
      result = std::make_shared<UpdateSessionImpl>( id, 0, completion, mConcreteHals[id], type );
    } else {
      throwBadId(id);
    }
    return result;
  }

  virtual void validate(std::string id, IUpdateCore::COMPLETION_CALLBACK completion){
    if( mConcreteHals.contains(id) && mConcreteHals[id] ){
      mConcreteHals[id]->validate(id, completion);
    } else {
      completion(id, true);
      throwBadId(id);
    }
  }

  virtual void activateForNext(std::string id, IUpdateCore::COMPLETION_CALLBACK completion){
    if( mConcreteHals.contains(id) && mConcreteHals[id] ){
      mConcreteHals[id]->activateForNext(id, completion);
    } else {
      completion(id, true);
      throwBadId(id);
    }
  }

  virtual void restartAndWaitToBoot(std::string id, IUpdateCore::COMPLETION_CALLBACK completion){
    if( mConcreteHals.contains(id) && mConcreteHals[id] ){
      mConcreteHals[id]->restartAndWaitToBoot(id, completion);
    } else {
      completion(id, true);
      throwBadId(id);
    }
  }
};


class UpdateInstallHalFactory
{
protected:
  static std::shared_ptr<IUpdateInstallHal> mInstance;

public:
  static std::shared_ptr<IUpdateInstallHal> getInstance();
};



