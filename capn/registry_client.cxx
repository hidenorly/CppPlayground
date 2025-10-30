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

// cd ~/work; git clone https://github.com/hidenorly/OptParse.git
// capnp compile -oc++ registry.capnp
// clang++ -std=c++20 -I/opt/homebrew/include -L/opt/homebrew/lib registry_client.cxx registry.capnp.c++ -lcapnp -lcapnp-rpc -lkj-async -lkj -o client

#include <capnp/ez-rpc.h>
#include "registry.capnp.h"
#include "../../OptParse/OptParse.hpp"
#include <iostream>
#include <unistd.h>


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


void construct_benchmark_data(std::vector<std::string>& values, int count)
{
    for( int i=0; i<count; i++) {
        values.push_back( std::to_string(i) );
    }
}

void benchmark_invoke( capnp::EzRpcClient& client, Registry::Client& registry, int count = 1000)
{
    std::vector<std::string> values;
    construct_benchmark_data(values, count);


    auto startTime = std::chrono::steady_clock::now();

    for( auto& value : values ){
      auto setReq = registry.setRequest();
      setReq.setKey("key1");
      setReq.setValue(value);
      setReq.send().wait(client.getWaitScope());
    }

    auto endTime = std::chrono::steady_clock::now();

    auto latency = (endTime - startTime) / count;
    auto latencyMs = duration_cast<std::chrono::microseconds>(latency).count();
    std::cout << "latency setValue : " << latencyMs << std::endl;
}



int main(int argc, char** argv) {
  std::vector<OptParse::OptParseItem> options;

  options.push_back( OptParse::OptParseItem("-b", "--benchmark", true, "0", "Specify benchmark count if benchmark"));

  OptParse optParser( argc, argv, options );

  int benchCount = std::stoi( optParser.values["-b"]=="true" ? "1000" : optParser.values["-b"] );
  bool isBenchmark = optParser.values.contains("-b") && ( benchCount!=0 );
  std::cout << "benchmark : " << benchCount << std::endl;

  std::string socketPath = "/tmp/capn_registry.sock";
  std::string unixsocketPath = "unix:"+socketPath;
  capnp::EzRpcClient client(unixsocketPath);
  auto registry = client.getMain<Registry>();

  if( isBenchmark ){
    benchmark_invoke(client, registry);
  } else {
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


  return 0;
}
