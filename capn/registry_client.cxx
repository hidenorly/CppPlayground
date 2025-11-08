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
// clang++ -std=c++23 -I/opt/homebrew/include -L/opt/homebrew/lib registry_client.cxx registry.capnp.c++ -lcapnp -lcapnp-rpc -lkj-async -lkj -o client

#include <capnp/ez-rpc.h>
#include "registry.capnp.h"
#include "../../OptParse/OptParse.hpp"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <functional>

#include "registry.hpp"

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



class RegistryClient : public ClientBase<Registry::Client>, public MyInterface
{
public:
  RegistryClient(std::string socketPath = "/tmp/capn_registry.sock"):ClientBase<Registry::Client>(socketPath){
  }

  std::string getValue(std::string key) override {
    bool isAvailable = mpClient && mpClientImpl;
    std::string result;

    if( isAvailable ){
      auto getReq = mpClientImpl->getRequest();
      getReq.setKey(key);
      auto reply = getReq.send().wait(mpClient->getWaitScope()).getReply();
      result = reply.cStr();
    }

    return result;
  }

  bool setValue(std::string key, std::string value) override {
    bool isAvailable = mpClient && mpClientImpl;

    if( isAvailable ){
      auto setReq = mpClientImpl->setRequest();
      setReq.setKey(key);
      setReq.setValue(value);
      setReq.send().wait(mpClient->getWaitScope());
    }

    return isAvailable;
  }

  uint32_t registerCallback(const std::string id, NOTIFIER notifier){
    auto lamdaCallback = kj::heap<LambdaCallbackHandler>(id, notifier);
    kj::Own<Callback::Server> basePtr = kj::mv(lamdaCallback);
    return _registerCallback(id, kj::mv(basePtr));
  }
};


void construct_benchmark_data(std::vector<std::string>& values, int count)
{
    for( int i=0; i<count; i++) {
        values.push_back( std::to_string(i) );
    }
}

void benchmark_invoke(int count = 1000)
{
  std::vector<std::string> values;
  construct_benchmark_data(values, count);

  RegistryClient reg;

  auto startTime = std::chrono::steady_clock::now();

  for( auto& value : values ){
    reg.setValue("key1", value);
  }

  auto endTime = std::chrono::steady_clock::now();

  auto latency = (endTime - startTime) / count;
  auto latencyMs = duration_cast<std::chrono::microseconds>(latency).count();
  std::cout << "latency setValue : " << latencyMs << std::endl;
}

void benchmark_callback(int count = 1000)
{
  std::vector<std::string> values;
  construct_benchmark_data(values, count);

  RegistryClient reg;
  reg.setValue("key1", "");

  using Clock = std::chrono::steady_clock;
  std::map<std::string, Clock::time_point> startTimes;
  std::map<std::string, Clock::duration> latencies;

  // setup callback handler
  NOTIFIER notifier = [&](const std::string& key, const std::string& value) {
      auto endTime = Clock::now();
      latencies[value] = endTime - startTimes[value];
  };
  auto id3 = reg.registerCallback("1", notifier);

  for( auto& value : values ){
      startTimes[value] = Clock::now();
      reg.setValue("key1", value);
  }

  std::this_thread::sleep_for(std::chrono::seconds(3));

  Clock::duration total_latency = Clock::duration::zero();
  int64_t received_count = 0;

  for (auto& entry : latencies) {
      total_latency += entry.second;
      received_count++;
  }

  double average_latency_us = -1;

  if (received_count > 0) {
      average_latency_us = std::chrono::duration_cast<std::chrono::microseconds>(total_latency).count() / static_cast<double>(received_count);
  }

  std::cout << "latency[uSec] callback of setValue : " << average_latency_us << std::endl;
}


int main(int argc, char** argv) {
  std::vector<OptParse::OptParseItem> options;

  options.push_back( OptParse::OptParseItem("-b", "--benchmark", true, "0", "Specify benchmark count if benchmark"));

  OptParse optParser( argc, argv, options );

  int benchCount = std::stoi( optParser.values["-b"]=="true" ? "1000" : optParser.values["-b"] );
  bool isBenchmark = optParser.values.contains("-b") && ( benchCount!=0 );
  std::cout << "benchmark : " << benchCount << std::endl;

  if( isBenchmark ){
    benchmark_invoke(benchCount);
    benchmark_callback(benchCount);
  } else {
    RegistryClient reg;
    NOTIFIER notifier4 = [&](const std::string& key, const std::string& value) {
      std::cout << "LAMBDA4: Key '" << key << "' = '" << value << "'" << std::endl;
    };
    auto id4 = reg.registerCallback("client4", notifier4);
    reg.setValue("hoge", "hoge_value");
    std::cout << reg.getValue("hoge") << std::endl;
    reg.unregisterCallback(id4);
  }

  return 0;
}
