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
#include <memory>

struct ListNode
{
  int value;
  std::shared_ptr<ListNode> next;
  ListNode(int val):value(val), next(nullptr){};
  ListNode(int val, std::shared_ptr<ListNode> next_ptr):value(val), next(next_ptr){};
};

class CLinkList
{
protected:
  std::shared_ptr<ListNode> head;

public:
  CLinkList() = default;
  virtual ~CLinkList() = default;

  void addValue(int value){
    std::shared_ptr<ListNode> newNode = std::make_shared<ListNode>(value);
    if(!head){
      head = newNode;
    } else {
      std::shared_ptr<ListNode> ptr = head;
      while(ptr->next){
        ptr = ptr->next;
      }
      ptr->next = newNode;
    }
  }

  std::shared_ptr<ListNode> getHead(){
    return head;
  }

  void dump_nodes(std::shared_ptr<ListNode> ptr=nullptr)
  {
    if(!ptr){
      ptr = head;
    }
    while(ptr){
      std::cout << ptr->value << std::endl;
      ptr = ptr->next;
    }
  }
};

class CSortedLinkList : public CLinkList
{
public:
  CSortedLinkList() = default;
  virtual ~CSortedLinkList() = default;

  void addValue(int value){
    std::shared_ptr<ListNode> newNode = std::make_shared<ListNode>(value);

    if(!head || head->value>=value){
      newNode->next = head;
      head = newNode;
    } else {
      std::shared_ptr<ListNode> ptr = head;
      while(ptr->next && ptr->next->value < value){
        ptr = ptr->next;
      }
      newNode->next = ptr->next;
      ptr->next = newNode;
    }
  }
};


std::shared_ptr<ListNode> mergeLists(std::shared_ptr<ListNode> l1, std::shared_ptr<ListNode> l2)
{

  if(!l1) return l2;
  if(!l2) return l1;

  if(l1->value < l2->value){
    l1->next = mergeLists(l1->next, l2);
    return l1;
  } else {
    l2->next = mergeLists(l1, l2->next);
    return l2;
  }

}



int main()
{
  std::cout << "LinkList" << std::endl;
  CLinkList linkList;
  linkList.addValue(10);
  linkList.addValue(11);
  linkList.addValue(9);
  linkList.addValue(12);
  linkList.dump_nodes();

  std::cout << "\nSortedLinkList" << std::endl;
  CSortedLinkList sortedLinkList;
  sortedLinkList.addValue(10);
  sortedLinkList.addValue(11);
  sortedLinkList.addValue(9);
  sortedLinkList.addValue(12);
  sortedLinkList.dump_nodes();

  CSortedLinkList sortedLinkList2;
  sortedLinkList2.addValue(20);
  sortedLinkList2.addValue(22);
  sortedLinkList2.addValue(18);
  sortedLinkList2.addValue(24);
  sortedLinkList2.addValue(3);

  std::cout << "\nMergedLinkList" << std::endl;
  std::shared_ptr<ListNode> ptr = mergeLists(sortedLinkList.getHead(), sortedLinkList2.getHead());
  linkList.dump_nodes(ptr);

  return 0;
}