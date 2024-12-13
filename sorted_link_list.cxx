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


std::shared_ptr<ListNode> add_value_to_list(int value, std::shared_ptr<ListNode> head=nullptr)
{
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

  return head;
}

void dump_nodes(std::shared_ptr<ListNode> ptr)
{
  while(ptr){
    std::cout << ptr->value << std::endl;
    ptr = ptr->next;
  }
}

int main()
{
  std::shared_ptr<ListNode> ptr = add_value_to_list(10);
  ptr = add_value_to_list(11, ptr);
  ptr = add_value_to_list(9, ptr);
  ptr = add_value_to_list(12, ptr);

  dump_nodes(ptr);

  return 0;
}