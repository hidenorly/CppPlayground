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

// clang++ -std=c++20 ExampleService.cxx

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


class MyService : public ServiceBase, public MyInterface, public ExampleService::Service
{
protected:
  std::map<std::string, std::string> mRegistry;
  std::unique_ptr<Server> mServer;

public:
  MyService(){
    mRegistry["ro.serialno"] = "dummy";
  }
  virtual ~MyService() = default;

//protected:
  virtual std::string getValue(std::string key) override{
    return mRegistry.contains(key) ? mRegistry[key] : "";
  }

  virtual void setValue(std::string key, std::string value) override{
    if( key.starts_with("ro.") && mRegistry.contains(key) ){
    } else {
      mRegistry[key] = value;
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

  virtual void setEnabled(bool enabled) override {
    if(!mIsEnabled && enabled){
      // enabling
      std::string server_address("0.0.0.0:50051");

      ServerBuilder builder;
      builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
      builder.RegisterService(this);

      mServer = builder.BuildAndStart();
      std::cout << "Server listening on " << server_address << std::endl;

      // Run in the thread
      std::thread([this]() {
          mServer->Wait();
      }).detach();
    } else if ( mIsEnabled && !enabled ){
      // disabling
      mServer->Shutdown();
      mServer = nullptr;
    }
    mIsEnabled = enabled;
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
  std::cout << "Disable the server\n";
  service.setEnabled(false);

  return 0;
}