#include <unordered_map>
#include <iostream>

using namespace std;

// LRU缓存类 Least Recent Used

class LRUCache 
{
private:
    // 双向链表节点结构
    struct DListNode 
    {
        int key;
        int value;
        DListNode* prev;
        DListNode* next;

        DListNode() : key(0), value(0), prev(nullptr), next(nullptr) { }

        DListNode(int k, int v) : key(k), value(v), prev(nullptr), next(nullptr) { }
    };

    unordered_map<int, DListNode*> cache;  // 哈希表，通过key直接定位链表元素
    int size;           // 当前缓存大小
    int capacity;       // 缓存最大容量
    DListNode* head;    // 虚拟头节点, 始终位于链表头部
    DListNode* tail;    // 虚拟尾节点，始终位于链表尾部

    // 将节点添加到头节点之后(最新访问/新添加 节点)
    void addToHead(DListNode* node)
    {
        node->next = head->next;
        node->prev = head;
        head->next->prev = node;
        head->next = node;
    }

    // 从链表中移除指定节点
    void removeNode(DListNode* node)
    {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    // 将节点移动到链表头部(表示最近使用 Most Recent Used)
    void moveToHead(DListNode* node) 
    {
        removeNode(node);  // 先移除
        addToHead(node);   // 再添加到头部
    }

    DListNode* removeTail()
    {
        DListNode* removed = tail->prev;    // 实际末尾节点是tail的前一个
        removeNode(removed);               // 从链表中移除
        return removed;                     // 返回被移除节点
    }
public:
    // 构造函数，初始化容量并创建虚拟头尾节点
    LRUCache(int c) : size(0), capacity(c) 
    {
        head = new DListNode();
        tail = new DListNode();
        head->next = tail;
        tail->prev = head;
    }

    // 获取键值，不存在返回-1，存在则移动节点到头部
    int get(int key)
    {
        if (!cache.count(key))
            return -1;
        
        DListNode* node = cache[key];
        moveToHead(node);
        return node->value;
    }

    // 插入键值对，存在则更新键值并移动到头部(最新访问)，不存在则创建新节点
    void put(int key, int value)
    {
        if (!cache.count(key))
        {
            DListNode *node = new DListNode(key, value);
            cache[key] = node;      // 存入哈希表
            addToHead(node);        // 添加到链表头部
            ++ size;

            // 超过容量时移除尾节点
            if (size > capacity)
            {
                DListNode* removed = removeTail();
                cache.erase(removed->key);  // 从哈希表中删除
                delete removed;
                -- size;
            }
        }
        else 
        {
            DListNode *node = cache[key];
            node->value = value;
            moveToHead(node);
        }
    }

    // 析构函数，释放所有内存
    ~LRUCache()
    {
        DListNode* cur = head;
        while (cur)
        {
            DListNode* next = cur->next;
            delete cur;
            cur = next;
        }
    }
};

// 示例测试代码
int main() 
{
    LRUCache cache(2);
    cache.put(1, 10);
    cache.put(2, 20);
    cout << cache.get(1) << endl; // 输出10
    cache.put(3, 3);              // 挤掉键2
    cout << cache.get(2) << endl; // 输出-1
    return 0;
}