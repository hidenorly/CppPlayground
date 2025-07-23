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

// clang++ -std=c++20 StateMachine.cxx 

#include <iostream>
#include <memory>
#include <string>


class StateManager;


class StateContext {
public:
  std::shared_ptr<StateManager> stateManager;
};

class IState {
protected:
  virtual void onEnter(std::shared_ptr<StateContext> context){};
  virtual void onLeave(std::shared_ptr<StateContext> context){};

public:
  virtual ~IState() = default;

  virtual void enter(std::shared_ptr<StateContext> context){ onEnter(context); };
  virtual void exit(std::shared_ptr<StateContext> context){ onLeave(context); };
};

class StateManager
{
protected:
  std::shared_ptr<IState> mCurrentState;
  std::shared_ptr<StateContext> mContext;

public:
  StateManager(std::shared_ptr<StateContext> context){
    mContext = context;
  };

  virtual ~StateManager() = default;

  void changeState(std::shared_ptr<IState> nextState){
    if( mCurrentState ){
      mCurrentState->exit(mContext);
    }
    if( nextState ){
      mCurrentState = nextState;
      nextState->enter(mContext);
    }
  }
};


class State3 : public IState
{
protected:
    virtual void onEnter(std::shared_ptr<StateContext> context){
      std::cout << "State3::enter" << std::endl;
    };
    virtual void onLeave(std::shared_ptr<StateContext> context){
      std::cout << "State3::leave" << std::endl;
    };

public:
  State3(){

  };
  virtual ~State3() = default;
};


class State2 : public IState
{
protected:
    virtual void onEnter(std::shared_ptr<StateContext> context){
      std::cout << "State2::enter" << std::endl;
      context->stateManager->changeState(std::make_shared<State3>());
    };
    virtual void onLeave(std::shared_ptr<StateContext> context){
      std::cout << "State2::leave" << std::endl;
    };

public:
  State2(){

  };
  virtual ~State2() = default;
};


class State1 : public IState
{
protected:
    virtual void onEnter(std::shared_ptr<StateContext> context){
      std::cout << "State1::enter" << std::endl;
      context->stateManager->changeState(std::make_shared<State2>());
    };
    virtual void onLeave(std::shared_ptr<StateContext> context){
      std::cout << "State1::leave" << std::endl;
    };

public:
  State1(){

  };
  virtual ~State1() = default;
};






int main() {
  std::shared_ptr<StateContext> context = std::make_shared<StateContext>();
  std::shared_ptr<StateManager> manager = std::make_shared<StateManager>(context);
  context->stateManager = manager;
  manager->changeState(std::make_shared<State1>());
}

