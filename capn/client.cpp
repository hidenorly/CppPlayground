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
#include <string>

// clang++ -std=c++20 -I/opt/homebrew/include -L/opt/homebrew/lib client.cpp example.capnp.c++ -lcapnp -lcapnp-rpc -lkj-async -lkj -o client

int main() {
  std::string socketPath = "/tmp/capn_example.sock";
  std::string unixsocketPath = "unix:"+socketPath;
  capnp::EzRpcClient client(unixsocketPath);
  auto cap = client.getMain<Example>();
  auto& waitScope = client.getWaitScope();

  auto request = cap.sendMessageRequest();
  request.setText("Hi server, from macOS client!");

  auto response = request.send().wait(waitScope);
  std::cout << "[Client] Server replied: " << response.getReply().cStr() << std::endl;

  // exception test
  try {
    auto req2 = cap.sendMessageRequest();
    req2.setText("throw");
    req2.send().wait(client.getWaitScope());
  } catch (const kj::Exception& e) {
    printf("[Client] Caught server exception: %s\n", e.getDescription().cStr());
  }

  return 0;
}
