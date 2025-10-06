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
// cd ~/work/CppPlayground/grpc; mkdir build; cd build; cmake ..; make; ./ExampleClient

#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <thread>
#include <algorithm>
#include "build/generated/example.grpc.pb.h"
#include "GrpcUtil.hpp"
#include "../../OptParse/OptParse.hpp"

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


void benchmark_invoke( MyServiceClient& client, int count = 1000)
{
    client.setValue("key1", "" );

    std::vector<std::string> values;
    for( int i=0; i<count; i++) {
        values.push_back( std::to_string(i) );
    }

    auto startTime = std::chrono::steady_clock::now();

    for( auto& value : values ){
        client.setValue("key1", value );
    }

    auto endTime = std::chrono::steady_clock::now();

    auto latency = (endTime - startTime) / count;
    std::cout << "latency setValue : " << latency << std::endl;
}

void benchmark_callback(MyServiceClient& client, int count = 1000)
{
    client.setValue("key1", "" );

    std::vector<std::string> values;
    for( int i=0; i<count; i++) {
        values.push_back( std::to_string(i) );
    }

    using Clock = std::chrono::steady_clock;
    std::map<std::string, Clock::time_point> startTimes;
    std::map<std::string, Clock::duration> latencies;

    auto callback = [&](const std::string& key, const std::string& value) {
        auto endTime = Clock::now();
        latencies[value] = endTime - startTimes[value];
    };

    const std::string id_1 = "1";
    client.registerCallback(id_1, callback);

    for( auto& value : values ){
        startTimes[value] = Clock::now();
        client.setValue("key1", value );
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

    std::cout << "latency[uSec] setValue : " << average_latency_us << std::endl;
}



// ---- main ----
int main(int argc, char** argv) {
    std::vector<OptParse::OptParseItem> options;

    options.push_back( OptParse::OptParseItem("-b", "--benchmark", true, "false", "Specify if benchmark"));

    OptParse optParser( argc, argv, options );

    bool isBenchmark = optParser.values.contains("-b") && ( optParser.values["-b"] == "true" );
    std::cout << "benchmark : " << (isBenchmark ? "true" : "false") << std::endl;

    std::string server_address("localhost:50051");
    MyServiceClient client;
    client.connect(server_address);

    if (client.isConnected()) {
        if( isBenchmark ){
            benchmark_invoke( client );
            benchmark_callback( client );
        }

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
