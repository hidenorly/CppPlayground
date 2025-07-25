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

// clang++ -std=c++20 ChainOfResponsibility.cxx

#include <iostream>
#include <memory>
#include <string>
#include <optional>

template <typename T>
class Handler {
protected:
  std::shared_ptr<Handler<T>> nextHandler;

public:
  void setNext(std::shared_ptr<Handler<T>> next) {
    nextHandler = next;
  }

  void handle(const T& request) {
    if (!process(request) && nextHandler) {
      nextHandler->handle(request);
    }
  }

  virtual ~Handler() = default;

protected:
  // @return true: processed / false : pass to next
  virtual bool process(const T& request) = 0;
};

typedef std::string Message;

class HandlerA : public Handler<Message>
{
public:
  virtual bool process(const Message& request){
    if( request.starts_with("HeaderA") ){
      std::cout << "HandlerA:" << request << std::endl;
      return true;
    }
    return false;
  }
};


class HandlerB : public Handler<Message>
{
public:
  virtual bool process(const Message& request){
    if( request.ends_with("HeaderB") ){
      std::cout << "HandlerB:" << request << std::endl;
      return true;
    }
    return false;
  }
};


class DefaultHandler : public Handler<Message>
{
public:
  virtual bool process(const Message& request){
    std::cout << "DefaultHandler:" << request << std::endl;
    return true;
  }
};


int main(int argc, char** argv) {
  auto head = std::make_shared<HandlerA>();
  auto handlerB = std::make_shared<HandlerB>();
  auto defaultHandler = std::make_shared<DefaultHandler>();

  head->setNext(handlerB);
  handlerB->setNext(defaultHandler);

  head->handle("HeaderA_MESSAGE"); // expect to handle by HandlerA
  head->handle("MESSAGE_HeaderB"); // expect to handle by HandlerB
  head->handle("HeaderB_MESSAGE"); // expect to handle by DefaultHandler
  head->handle("hoge"); // expect to handle by DefaultHandler

  return 0;
}