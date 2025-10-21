#include <capnp/ez-rpc.h>
#include "example.capnp.h"
#include <iostream>
#include <unistd.h>

// clang++ -std=c++20 -I/opt/homebrew/include -L/opt/homebrew/lib server.cpp example.capnp.c++ -lcapnp -lcapnp-rpc -lkj-async -lkj -o server

class ServerImpl final : public Example::Server {
public:
  kj::Promise<void> sendMessage(SendMessageContext context) override {
    auto text = context.getParams().getText();
    std::cout << "[Server] Received: " << text.cStr() << std::endl;

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
