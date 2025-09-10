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
#include "ExampleClient.hpp"

using com::gmail::twitte::harold::ExampleService;
using com::gmail::twitte::harold::GetValueRequest;
using com::gmail::twitte::harold::GetValueReply;
using com::gmail::twitte::harold::SetValueRequest;
using com::gmail::twitte::harold::SetValueReply;



class MyServiceClient 
    : public ClientBase<MyServiceClient, ExampleService> {
public:
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
};


// ---- main ----
int main(int argc, char** argv) {
    MyServiceClient client;
    client.connect("localhost:50051");

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

    return 0;
}
