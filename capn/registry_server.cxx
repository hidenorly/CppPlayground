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

// capnp compile -oc++ registry.capnp
// clang++ -std=c++20 -I/opt/homebrew/include -L/opt/homebrew/lib registry_server.cxx registry.capnp.c++ -lcapnp -lcapnp-rpc -lkj-async -lkj -o server 


#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <thread>

#include <capnp/ez-rpc.h>
#include "registry.capnp.h"
#include <kj/debug.h>
#include <kj/common.h>
#include <unistd.h>

# include "registry.hpp"

class RegistryServer final : public Registry::Server, public MyInterface
{
protected:
  std::map<std::string, std::string> mRegistry;
  std::mutex mRegistryMutex;

  std::unordered_map<uint32_t, Callback::Client> mCallbacks;
  std::mutex mRegisterMutex;
  uint32_t mNextId = 1;

public:
  kj::Promise<void> registerCallback(RegisterCallbackContext context) override {
    std::lock_guard<std::mutex> lock(mRegisterMutex);
    uint32_t id = mNextId++;
    Callback::Client cb = context.getParams().getCb();

    mCallbacks.emplace(id, cb);
    printf("[Server] Callback registered. id=%u total=%zu\n", id, mCallbacks.size());

    context.getResults().setId(id);
    return kj::READY_NOW;
  }

  kj::Promise<void> unregisterCallback(UnregisterCallbackContext context) override {
    std::lock_guard<std::mutex> lock(mRegisterMutex);
    uint32_t id = context.getParams().getId();
    size_t erased = mCallbacks.erase(id);
    printf("[Server] Callback unregistered. id=%u (removed=%zu) total=%zu\n", id, erased, mCallbacks.size());
    return kj::READY_NOW;
  }

  kj::Promise<void> set(SetContext context) override{
    auto key = context.getParams().getKey();
    auto value = context.getParams().getValue();
    if( setValue(key, value) ){
      // changed
      for( auto& [id, cb] : mCallbacks ){
        auto req = cb.onUpdateRequest();
        req.setKey(key);
        req.setValue(value);
        auto promise = req.send().then(
          [id](auto&&) {
            std::cout << "[Server] Notified callback id=" << id << std::endl;
          },
          [id](kj::Exception&& e) {
            std::cerr << "[Server] Callback failed (id=" << id << "): " << e.getDescription().cStr() << std::endl;
        });
        std::ignore = promise;
      }
    }
    return kj::READY_NOW;
  }

  kj::Promise<void> get(GetContext context) override {
    auto key = context.getParams().getKey();
    auto value = getValue(key);
    context.getResults().setReply(kj::StringPtr(value));//context.getResults().setReply(kj::StringPtr(value));//context.getResults().setReply(kj::str(value));

    std::cout << "get(key=" << key.cStr() << ") returns " << value << std::endl;
    return kj::READY_NOW;
  }



public:
  std::string getValue(std::string key) override {
    std::lock_guard<std::mutex> lock(mRegistryMutex);

    return mRegistry.contains(key) ? mRegistry[key] : "";
  }

  bool setValue(std::string key, std::string value) override {
    std::lock_guard<std::mutex> lock(mRegistryMutex);

    std::string oldValue;
    bool valueChanged = false;

    {
      if (mRegistry.contains(key)) {
        oldValue = mRegistry[key];
      }
      if (oldValue != value || !mRegistry.contains(key)) {
        mRegistry[key] = value;
        std::cout << "key=" << key << ", value=" << value << std::endl;
        valueChanged = true;
      }
    }

    return valueChanged;
  }
};


int main()
{
  RegistryServer registry;
  registry.setValue("key1", "value1");
  std::cout << "key1=" << registry.getValue("key1") << std::endl;

  std::string socketPath = "/tmp/capn_registry.sock";
  std::string unixsocketPath = "unix:"+socketPath;
  unlink(socketPath.c_str());

  capnp::EzRpcServer server(kj::heap<RegistryServer>(), unixsocketPath);
  auto& waitScope = server.getWaitScope();

  std::cout << "Server listening on " << socketPath << std::endl;
  kj::NEVER_DONE.wait(waitScope);


  return 0;
}