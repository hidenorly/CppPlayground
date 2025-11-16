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


#include <capnp/ez-rpc.h>
#include "registry.capnp.h"
#include <functional>

template <typename T> class ClientBase
{
protected:
  std::shared_ptr<capnp::EzRpcClient> mpClient;
  std::shared_ptr<T> mpClientImpl;
  std::string mSocketPath;
  std::string mUnixsocketPath;
  std::vector<uint32_t> mCallbackIds;

public:
  ClientBase(std::string socketPath):mSocketPath(socketPath){
    mUnixsocketPath = "unix:"+socketPath;

    mpClient = std::make_shared<capnp::EzRpcClient>(mUnixsocketPath);
    if(mpClient){
      mpClientImpl = std::make_shared<T>(mpClient->getMain<Registry>());
    }
  }

  virtual ~ClientBase(){
    bool isAvailable = mpClient && mpClientImpl;

    if( isAvailable ){
      for(auto& id : mCallbackIds){
        unregisterCallback(id);
      }
      mCallbackIds.clear();
    }
  }

  virtual uint32_t _registerCallback(const std::string id, kj::Own<Callback::Server> callback){
    uint32_t resultId = -1;
    bool isAvailable = mpClient && mpClientImpl;

    if( isAvailable ){
      auto regReq = mpClientImpl->registerCallbackRequest();
      regReq.setCb(kj::mv(callback));
      resultId = regReq.send().wait(mpClient->getWaitScope()).getId();
      mCallbackIds.push_back(resultId);
    }

    return resultId;
  }

  virtual void unregisterCallback(const uint32_t id){
    bool isAvailable = mpClient && mpClientImpl;
    if( isAvailable ){
      auto unregReq = mpClientImpl->unregisterCallbackRequest();
      unregReq.setId(id);
      unregReq.send().wait(mpClient->getWaitScope());
    }
    std::erase_if(mCallbackIds, [&](uint32_t theId){ return theId == id; });
  }
};


typedef std::function<void(const std::string&, const std::string&)> NOTIFIER;

class LambdaCallbackHandler final : public Callback::Server {
protected:
  NOTIFIER mNotifier;

public:
  LambdaCallbackHandler(std::string name, NOTIFIER notifier) : mHandlerName(std::move(name)), mNotifier(std::move(notifier)) {}

  kj::Promise<void> onUpdate(OnUpdateContext context) override {
    auto key = context.getParams().getKey();
    auto value = context.getParams().getValue();
    mNotifier(key, value);
    return kj::READY_NOW;
  }

private:
  std::string mHandlerName;
};
