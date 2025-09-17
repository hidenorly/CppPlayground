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
#include <memory>
#include <string>
#include <thread>
#include "ExampleClient.hpp"

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
protected:
  std::unique_ptr<ExampleService::Stub> mStub;

public:
    MyServiceClient(std::shared_ptr<Channel> channel) : mStub(ExampleService::NewStub(channel)) {}
    virtual ~MyServiceClient() = default;

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

    void subscribeToChanges() {
        ClientContext context;
        std::shared_ptr<grpc::ClientReaderWriter<SubscriptionRequest, ChangeNotification>> stream(
        mStub->SubscribeToChanges(&context));

        ChangeNotification notification;
        std::cout << "Subscribed to change notifications. Waiting for updates..." << std::endl;

        while (stream->Read(&notification)) {
            std::cout << "Notification received: Key '" << notification.key()
            << "' changed to '" << notification.new_value() << "'" << std::endl;
        }

        Status status = stream->Finish();
        if (!status.ok()) {
            std::cerr << "SubscribeToChanges stream failed: " << status.error_message() << std::endl;
        }
        std::cout << "Subscription stream terminated." << std::endl;
    }

    bool shutdown(void) {
        ShutdownRequest request;
        ShutdownReply reply;
        ClientContext context;

        Status status = getStub()->Shutdown(&context, request, &reply);
        return status.ok();
    }
};


// ---- main ----
int main(int argc, char** argv) {
    std::string server_address("localhost:50051");
    MyServiceClient client(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));

    std::thread subscriber_thread([&]() {
        client.subscribeToChanges();
    });

    std::this_thread::sleep_for(std::chrono::seconds(3));

    if (!client.isConnected()) {
        std::cerr << "Failed to connect to server" << std::endl;
        return 1;
    }

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

    subscriber_thread.join();

    return 0;
}
