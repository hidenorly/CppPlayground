/*
  Copyright (C) 2024 hidenorly

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

#include <iostream>
#include <stack>
#include <vector>
#include <stdexcept>

class QueueWithStacks
{
protected:
  std::stack<int> mInputStack, mOutputStack;
public:
  QueueWithStacks() = default;
  virtual ~QueueWithStacks() = default;

  void enqueue(int value){
    mInputStack.push(value);
  }

  int dequeue(){
    if( mOutputStack.empty() ){
      while( !mInputStack.empty() ){
        mOutputStack.push( mInputStack.top() );
        mInputStack.pop();
      }
    }
    if(mOutputStack.empty()){
      throw std::runtime_error("queue is empty");
    }

    int retValue = mOutputStack.top();
    mOutputStack.pop();
    return retValue;
  }

  bool isEmpty(){
    return mInputStack.empty() && mOutputStack.empty();
  }
};

int main()
{
  std::vector<int> values = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  QueueWithStacks queue;

  for(auto& value : values){
    queue.enqueue(value);
  }

  while( !queue.isEmpty() ){
    std::cout << queue.dequeue() << std::endl;
  }

  return 0;
}
