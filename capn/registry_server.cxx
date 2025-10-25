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
#include <unistd.h>


class RegistryServer final : public Registry::Server
{
protected:
  std::map<std::string, std::string> mRegistry;
  std::mutex mRegistryMutex;

public:
  std::unordered_map<uint32_t, Callback::Client> mCallbacks;
  std::mutex mRegisterMutex;
  uint32_t mNextId = 1;

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



public:
  std::string getValue(std::string key) {
    std::lock_guard<std::mutex> lock(mRegistryMutex);
    return mRegistry.contains(key) ? mRegistry[key] : "";
  }

  void setValue(std::string key, std::string value) {
    std::string oldValue;
    bool valueChanged = false;

    {
      if (mRegistry.contains(key)) {
        oldValue = mRegistry[key];
      }
      if (oldValue != value || !mRegistry.contains(key)) {
        mRegistry[key] = value;
        valueChanged = true;
      }
    }
    if (valueChanged) {
      // TODO: should callback here
    }
  }
};


int main()
{
  RegistryServer registry;
  registry.setValue("key1", "value1");
  std::cout << "key1=" << registry.getValue("key1") << std::endl;

  return 0;
}