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

// cd build; cmake ..; make; ./ExampleClient


#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <thread>
#include <algorithm>
#include "build/generated/example.grpc.pb.h"
#include "GrpcUtil.hpp"

using grpc::ClientContext;
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

class MyServiceClient : public ClientBase<MyServiceClient, ExampleService> {
public:
    typedef std::function<void(const std::string&, const std::string&)> NOTIFIER;

protected:
    std::unique_ptr<ClientContext> mSubscriberContext;
    std::unique_ptr<grpc::ClientReaderWriter<SubscriptionRequest, ChangeNotification>> mSubscriberStream;
    std::mutex mMutexSubscriber;

    std::map<std::string, NOTIFIER> mCallbacks;
    std::mutex mCallbackMutex;
    std::unique_ptr<std::thread> mSubscriberThread;


public:
    MyServiceClient() = default;
    virtual ~MyServiceClient(){
        terminateSubscriber();
    }

    std::string getValue(const std::string& key) {
        GetValueRequest request;
        request.set_key(key);

        GetValueReply reply;
        ClientContext context;

        Status status = getStub()->GetValue(&context, request, &reply);

        if (status.ok()) {
            return reply.value();
        } else {
            return "RPC failed: " + status.error_message();
        }
    }

    bool setValue(const std::string& key, const std::string& value) {
        SetValueRequest request;
        request.set_key(key);
        request.set_value(value);

        SetValueReply reply;
        ClientContext context;

        Status status = getStub()->SetValue(&context, request, &reply);
        return status.ok();
    }

    bool shutdown(void) {
        ShutdownRequest request;
        ShutdownReply reply;
        ClientContext context;

        Status status = getStub()->Shutdown(&context, request, &reply);
        return status.ok();
    }

    void registerCallback(const std::string& id, const NOTIFIER& callback) {
        std::lock_guard<std::mutex> lock(mCallbackMutex);
        mCallbacks[id] = callback;
        if( !mSubscriberThread ){
            mSubscriberThread = std::make_unique<std::thread>([&]{
                subscribeToChanges();
            });
        }
    }

    void unregisterCallback(const std::string& id) {
        std::lock_guard<std::mutex> lock(mCallbackMutex);
        if( mCallbacks.contains(id) ){
            mCallbacks.erase(id);
        }
        if( mCallbacks.empty() ){
            terminateSubscriber();
        }
    }

protected:
    void subscribeToChanges() {
        {
            std::lock_guard<std::mutex> lock(mMutexSubscriber);
            mSubscriberContext = std::make_unique<ClientContext>();
            mSubscriberStream = std::unique_ptr<grpc::ClientReaderWriter<SubscriptionRequest, ChangeNotification>>(mStub->SubscribeToChanges(mSubscriberContext.get()));
        }

        ChangeNotification notification;
        std::cout << "Subscribed to change notifications. Waiting for updates..." << std::endl;

        while (mSubscriberContext && mSubscriberStream->Read(&notification)) {
            std::lock_guard<std::mutex> lock(mCallbackMutex);
            for( auto& [id, callback] : mCallbacks ){
                callback(notification.key(), notification.new_value());
            }
        }

        if(mSubscriberContext){
            Status status = mSubscriberStream->Finish();
            if (!status.ok()) {
                std::cerr << "SubscribeToChanges stream failed: " << status.error_message() << std::endl;
            }
        }
        std::cout << "Subscription stream terminated." << std::endl;

        mSubscriberStream = nullptr;
    }

    void cancelSubscription() {
        std::lock_guard<std::mutex> lock(mMutexSubscriber);
        if (mSubscriberContext) {
            mSubscriberContext->TryCancel();
        }
    }

    void terminateSubscriber(){
        std::cout << "terminateSubscriber\n";
        cancelSubscription();
        if( mSubscriberThread ){
            mSubscriberThread->join();
            mSubscriberThread = nullptr;
        }
    }
};


// ---- main ----
int main(int argc, char** argv) {
    std::string server_address("localhost:50051");
    MyServiceClient client;
    client.connect(server_address);

    if (client.isConnected()) {
        auto callback = [&](const std::string& key, const std::string& value) {
            std::cout << "Notified via callback: Key '" << key << "' = '" << value << "'" << std::endl;
        };
        const std::string id_1 = "1";
        const std::string id_2 = "2";
        client.registerCallback(id_1, callback);
        client.registerCallback(id_2, callback);

        std::thread changer_thread([&]() {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            std::cout << "Setting key1=value1..." << std::endl;
            if (client.setValue("key1", "value1")) {
                std::cout << "Set succeeded" << std::endl;
            } else {
                std::cout << "Set failed" << std::endl;
            }

            std::cout << "Getting key1..." << std::endl;
            std::string value = client.getValue("key1");
            std::cout << "Got value: " << value << std::endl;

            std::cout << "Request Shutdown()" << std::endl;
            client.shutdown();
        });

        changer_thread.join();

        client.unregisterCallback(id_1);
        client.unregisterCallback(id_2);
    } else {
        std::cerr << "Failed to connect to the server within the timeout period." << std::endl;
        return -1;
    }

    return 0;
}
