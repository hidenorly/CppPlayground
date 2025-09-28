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

#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <thread>

#include <grpcpp/grpcpp.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

template <typename Derived>
class ServiceBase
{
protected:
  std::atomic<bool> mIsEnabled = false;
  std::unique_ptr<Server> mServer;

public:
  ServiceBase() = default;
  virtual ~ServiceBase() = default;
  virtual ::grpc::Service* getGrpcService(){
    return static_cast<Derived*>(this);
  };
  void requestShutdownAsync() {
      std::thread([this]() {
          this->setEnabled(false);
      }).detach();
  }

  virtual void setEnabled(bool enabled) {
    if(!mIsEnabled && enabled){
      // enabling
      std::string server_address("0.0.0.0:50051");

      ServerBuilder builder;
      builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
      builder.RegisterService(getGrpcService());

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
  virtual bool getEnabled(){ return mIsEnabled; };
};

// TContext: The type of the ServerContext.
// TStream: The type of the ServerReaderWriter stream.
// TContent: the notifying content
template <typename TContext, typename TStream, typename TContent>
class TSubscriptionManager {
public:
    using Subscription = std::pair<TContext*, TStream*>;

protected:
    std::list<Subscription> mSubscriptions;
    std::mutex mMutex;

public:
    void addSubscription(TContext* context, TStream* stream) {
        std::lock_guard<std::mutex> lock(mMutex);
        mSubscriptions.emplace_back(context, stream);
    }

    void removeSubscription(TContext* context) {
        std::lock_guard<std::mutex> lock(mMutex);
        mSubscriptions.remove_if([context](const Subscription& s) {
            return s.first == context;
        });
    }

    virtual void notifyAll(const TContent& content) {
        std::lock_guard<std::mutex> lock(mMutex);
        for (const auto& subscription : mSubscriptions) {
            // Check the connection then send the notify
            if (!subscription.second->Write(content)) {
                std::cerr << "Failed to write to client, assuming disconnect." << std::endl;
            }
        }
    }
};
