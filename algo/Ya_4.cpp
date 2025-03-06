#include <iostream>

// 带有随机指针的链表

class Node 
{
public:
    Node(int val_) : val{val_}, next{nullptr}, random{nullptr}
    { }

    int val;
    Node* next;
    Node* random;
};

Node* copyRandomList(Node* head)
{
    // 对指针类型的常规判断: 是否为空指针
    if (head == nullptr)
        return nullptr;
    
    // 复制原链表的每个节点，将新节点加载旧节点后面
    Node* cur = head;
    while (cur)
    {
        Node* node = new Node(cur->val);
        node->next = cur->next;
        cur->next = node;
        cur = node->next;
    }

    // 为新节点设置 random 指针
    cur = head;
    while (cur)
    {
        if (cur->random != nullptr)
            cur->next->random = cur->random->next;
        cur = cur->next->next;
    }


}