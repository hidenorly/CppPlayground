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


#include <memory>
#include <grpcpp/grpcpp.h>

using grpc::Channel;

template <typename Derived, typename ServiceType>
class ClientBase {
protected:
    std::shared_ptr<Channel> mChannel;
    std::unique_ptr<typename ServiceType::Stub> mStub;

public:
    ClientBase() = default;
    virtual ~ClientBase() = default;

    void connect(const std::string& server_address) {
        mChannel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
        mStub = ServiceType::NewStub(mChannel);
    }

    bool isConnected() const {
        return (mStub != nullptr);
    }

    typename ServiceType::Stub* getStub() {
        return mStub.get();
    }
};
