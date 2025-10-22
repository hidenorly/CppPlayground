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
}
