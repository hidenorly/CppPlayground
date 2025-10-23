# Prepare

```
brew install capnp
```

# generate from idl

```
capnp compile -oc++ example.capnp
```

# build

```
% clang++ -std=c++20 -I/opt/homebrew/include -L/opt/homebrew/lib server.cpp example.capnp.c++ -lcapnp -lcapnp-rpc -lkj-async -lkj -o server
% clang++ -std=c++20 -I/opt/homebrew/include -L/opt/homebrew/lib client.cpp example.capnp.c++ -lcapnp -lcapnp-rpc -lkj-async -lkj -o client
```

# test

```
% ./server&; ./client 
Server listening on /tmp/capn_example.sock
[Server] Received: Hi server, from macOS client!
[Client] Server replied: Hello from server!
```