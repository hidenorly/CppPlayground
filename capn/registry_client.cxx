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
// clang++ -std=c++20 -I/opt/homebrew/include -L/opt/homebrew/lib registry_client.cxx registry.capnp.c++ -lcapnp -lcapnp-rpc -lkj-async -lkj -o client

#include <capnp/ez-rpc.h>
#include "registry.capnp.h"
#include <iostream>

class CallbackHandler final : public Callback::Server {
public:
  CallbackHandler(std::string name) : mHandlerName(std::move(name)) {}

  kj::Promise<void> onUpdate(OnUpdateContext context) override {
    auto key = context.getParams().getKey();
    auto value = context.getParams().getValue();
    std::cout << "[" << mHandlerName << "] onUpdate: " << key.cStr() << " = " << value.cStr() << std::endl;
    return kj::READY_NOW;
  }

private:
  std::string mHandlerName;
};

int main() {
  std::string socketPath = "/tmp/capn_registry.sock";
  std::string unixsocketPath = "unix:"+socketPath;
  capnp::EzRpcClient client(unixsocketPath);
  auto registry = client.getMain<Registry>();

  auto cb = kj::heap<CallbackHandler>("Client1");
  auto regReq = registry.registerCallbackRequest();
  regReq.setCb(kj::mv(cb));
  auto id = regReq.send().wait(client.getWaitScope()).getId();
  std::cout << "[Client] Registered callback id=" << id << std::endl;

  auto cb2 = kj::heap<CallbackHandler>("Client2");
  auto regReq2 = registry.registerCallbackRequest();
  regReq2.setCb(kj::mv(cb2));
  auto id2 = regReq2.send().wait(client.getWaitScope()).getId();
  std::cout << "[Client] Registered callback id=" << id2 << std::endl;

  auto setReq = registry.setRequest();
  setReq.setKey("foo");
  setReq.setValue("bar");
  setReq.send().wait(client.getWaitScope());

  auto getReq = registry.getRequest();
  getReq.setKey("foo");
  auto reply = getReq.send().wait(client.getWaitScope()).getReply();
  std::cout << "[Client] get(foo) = " << reply.cStr() << std::endl;

  // Unregister
  auto unregReq = registry.unregisterCallbackRequest();
  unregReq.setId(id);
  unregReq.send().wait(client.getWaitScope());
  std::cout << "[Client] Unregistered callback id=" << id << std::endl;
}
