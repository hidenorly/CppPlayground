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

// cd build; cmake ..; make; ./ExampleServer

#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <thread>

#include <grpcpp/grpcpp.h>
#include "build/generated/example.grpc.pb.h"
#include "ExampleService.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using com::gmail::twitte::harold::ExampleService;
using com::gmail::twitte::harold::GetValueRequest;
using com::gmail::twitte::harold::GetValueReply;
using com::gmail::twitte::harold::SetValueRequest;
using com::gmail::twitte::harold::SetValueReply;
using com::gmail::twitte::harold::ShutdownRequest;
using com::gmail::twitte::harold::ShutdownReply;
using com::gmail::twitte::harold::ChangeNotification;
using com::gmail::twitte::harold::SubscriptionRequest;

class SubscriptionManager {
public:
  using Stream = grpc::ServerReaderWriter<ChangeNotification, SubscriptionRequest>;
  using Subscription = std::pair<ServerContext*, Stream*>;

protected:
  std::list<Subscription> mSubscriptions;
  std::mutex mMutex;

public:
  void addSubscription(ServerContext* context, Stream* stream) {
    std::lock_guard<std::mutex> lock(mMutex);
    mSubscriptions.emplace_back(context, stream);
  }

  void removeSubscription(ServerContext* context) {
    std::lock_guard<std::mutex> lock(mMutex);
    mSubscriptions.remove_if([context](const Subscription& s){
      return s.first == context;
    });
  }

  void notifyAll(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mMutex);
    ChangeNotification notification;
    notification.set_key(key);
    notification.set_new_value(value);

    for (const auto& subscription : mSubscriptions) {
      // 接続が有効か確認し、通知を送信
      if (!subscription.second->Write(notification)) {
        std::cerr << "Failed to write to client, assuming disconnect." << std::endl;
      }
    }
  }
};




class MyService : public ServiceBase<MyService>, public MyInterface, public ExampleService::Service
{
protected:
  std::unique_ptr<Server> mServer;
  std::map<std::string, std::string> mRegistry;
  std::mutex mRegistryMutex;
  SubscriptionManager mSubscriptionManager;

public:
  MyService(){
    mRegistry["ro.serialno"] = "dummy";
  }
  virtual ~MyService() = default;

//protected:
  virtual std::string getValue(std::string key) override {
    std::lock_guard<std::mutex> lock(mRegistryMutex);
    return mRegistry.contains(key) ? mRegistry[key] : "";
  }

  virtual void setValue(std::string key, std::string value) override {
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
        mSubscriptionManager.notifyAll(key, value);
    }
  }

public:
  Status GetValue(ServerContext* context, const GetValueRequest* request, GetValueReply* reply) override {
    reply->set_value( getValue(request->key()) );
    return Status::OK;
  }

  Status SetValue(ServerContext* context, const SetValueRequest* request, SetValueReply* reply) override {
    setValue( request->key(), request->value() );
    reply->set_success(true);
    return Status::OK;
  }

  Status SubscribeToChanges(ServerContext* context, grpc::ServerReaderWriter<ChangeNotification, SubscriptionRequest>* stream) override {
    mSubscriptionManager.addSubscription(context, stream);
    std::cout << "Client subscribed to changes." << std::endl;

    SubscriptionRequest request;
    while (stream->Read(&request)) {
    }

    mSubscriptionManager.removeSubscription(context);
    std::cout << "Client unsubscribed from changes." << std::endl;
    return Status::OK;
  }

  Status Shutdown(ServerContext* context, const ShutdownRequest* request, ShutdownReply* reply) override {
    std::cout << "Shutdown() requested\n";
    requestShutdownAsync();
    std::cout << "Done:requestShutdownAsync()\n";
    reply->set_success(true);
    return Status::OK;
  }
};

int main()
{
  MyService service;
  std::cout << "Enable gRPC server\n";
  service.setEnabled(true);
  if( service.getEnabled() ){
    std::cout << "service enabled" << std::endl;
    std::cout << "ro.serialno=" << service.getValue("ro.serialno") << std::endl;
    service.setValue("ro.serialno", "override");
    std::cout << "ro.serialno=" << service.getValue("ro.serialno") << std::endl;
    std::cout << "ro.build.fingerprint=" << service.getValue("ro.build.fingerprint") << std::endl;
    service.setValue("ro.build.fingerprint", "writeonece");
    std::cout << "ro.build.fingerprint=" << service.getValue("ro.build.fingerprint") << std::endl;
    service.setValue("ro.build.fingerprint", "writeonece2");
    std::cout << "ro.build.fingerprint=" << service.getValue("ro.build.fingerprint") << std::endl;
  }


  std::cout << "Enable sever in 10 seconds or invoke Shutdon() to turn off immediately\n";
  const int MIN_RESPONSE_MILLISECOND = 100;
  for(int i=0; i<10*(1000/MIN_RESPONSE_MILLISECOND); i++){
    std::this_thread::sleep_for(std::chrono::milliseconds(MIN_RESPONSE_MILLISECOND));
    if( !service.getEnabled() ) break;
  }
  service.setEnabled(false);

  return 0;
}