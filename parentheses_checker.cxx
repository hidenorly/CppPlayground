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
#include <string>
#include <unordered_map>

bool isValidParentheses(const std::string& str)
{
  bool result = true;

  const static std::unordered_map<char, char> open_brackets = {
    {'(', ')'},
    {'[', ']'},
    {'{', '}'}
  };
  const static std::unordered_map<char, bool> close_brackets = {
   {')', true},
   {']', true},
   {'}', true}
 };

  std::stack<char> stack;

  for( auto ch : str ){
    if( open_brackets.contains(ch) ){
      // opening blacket
      stack.push(open_brackets.at(ch));
      //std::cout << "found.." << ch << std::endl;
    } else {
      if( close_brackets.contains(ch) ){
        // closing blacket
        if( !stack.empty() && stack.top() == ch ){
          //std::cout << "found matched.." << ch << std::endl;
          stack.pop();
        } else {
          result = false;
          break;
        }
      }
    }
  }
  result = result & stack.empty();

  return result;
}

std::string getTrueFalseString(bool flag)
{
  if(flag){
    return "True";
  } else {
    return "False";
  }
}

void printResult(const std::string& str)
{
  bool result = isValidParentheses(str);
  std::cout << "input:" << str << " : " << getTrueFalseString(result) << std::endl;
}

int main()
{
  printResult("");
  printResult("{([])}");
  printResult("{(()])}");
  printResult("{}");
  printResult("(aaa{})");
  return 0;
}