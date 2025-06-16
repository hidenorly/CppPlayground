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

// clang++ -std=c++20 Updater.cxx 

#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>

class IUpdateInstallHal
{
public:
  virtual std::vector<std::string> getSupportedIds() = 0;
  virtual std::map<std::string, std::string> getMetaDataById(std::string id) = 0;
};


class UpdateInstallHalImpl : public IUpdateInstallHal
{
protected:
  std::map<std::string, std::map<std::string, std::string>> mDummyData;

  void setUpDummyData(){
    std::map<std::string, std::string> dummyMeta;
    dummyMeta["version"] = "0000";
    dummyMeta["hash"] = "0123456789abcdef";

    mDummyData["system"] = dummyMeta;
    mDummyData["vendor"] = dummyMeta;
    mDummyData["product"] = dummyMeta;
    mDummyData["oem"] = dummyMeta;
    mDummyData["odm"] = dummyMeta;

    std::map<std::string, std::string> dummyMeta2;
    dummyMeta2["version"] = "1234";
    dummyMeta2["hash"] = "abcdef0123456789";
    mDummyData["mcu_1"] = dummyMeta2;
  };

public:
  UpdateInstallHalImpl(){
    setUpDummyData();
  }
  virtual ~UpdateInstallHalImpl() = default;

  virtual std::vector<std::string> getSupportedIds(){
    std::vector<std::string> ids;
    for( auto& [key, val] : mDummyData ){
      ids.push_back(key);
    }
    return ids;
  }

  virtual std::map<std::string, std::string> getMetaDataById(std::string id){
    if( mDummyData.contains(id) ){
      return mDummyData[id];
    }
    return std::map<std::string, std::string>({});
  }
};



int main(int argc, char** argv) {
  std::shared_ptr<IUpdateInstallHal> hal = std::make_shared<UpdateInstallHalImpl>();

  std::vector<std::string> updateTargets = hal->getSupportedIds();
  for(auto& id : updateTargets){
    std::cout << id << std::endl;
    std::map<std::string, std::string> theMeta = hal->getMetaDataById(id);
    for( auto& [key, value] : theMeta ){
      std::cout << "\t" << key << ":" << value << std::endl;
    }
  }

  return 0;
}