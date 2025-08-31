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

// clang++ -std=c++20 ExampleService.cxx

#include <iostream>
#include <memory>
#include <string>
#include <map>


class ServiceBase
{
protected:
  bool mIsEnabled = false;

public:
  ServiceBase() = default;
  virtual ~ServiceBase() = default;
  virtual void setEnabled(bool enabled){
    // TODO: Actual registration & unregistration
    mIsEnabled = enabled;
  }
  virtual bool getEnabled(){ return mIsEnabled; };
};


class MyInterface
{
public:
  virtual std::string getValue(std::string key) = 0;
  virtual void setValue(std::string key, std::string value) = 0;
};
