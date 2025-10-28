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
#include <capnp/ez-rpc.h>
#include "example.capnp.h"
#include <iostream>
#include <unistd.h>

// capnp compile -oc++ example.capnp
// clang++ -std=c++20 -I/opt/homebrew/include -L/opt/homebrew/lib server.cpp example.capnp.c++ -lcapnp -lcapnp-rpc -lkj-async -lkj -o server

class ServerImpl final : public Example::Server {
public:
  kj::Promise<void> sendMessage(SendMessageContext context) override {
    auto _text = context.getParams().getText();
    std::string text(_text.cStr());
    std::cout << "[Server] Received: " << text << std::endl;

    if( text == "throw" ){
      KJ_FAIL_REQUIRE("Server simulated exception");
    }

    context.getResults().setReply("Hello from server!");
    return kj::READY_NOW;
  }
};

int main() {
  std::string socketPath = "/tmp/capn_example.sock";
  std::string unixsocketPath = "unix:"+socketPath;
  unlink(socketPath.c_str());

  capnp::EzRpcServer server(kj::heap<ServerImpl>(), unixsocketPath);
  auto& waitScope = server.getWaitScope();

  std::cout << "Server listening on " << socketPath << std::endl;
  kj::NEVER_DONE.wait(waitScope);
}
