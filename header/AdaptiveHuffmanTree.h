//
// Created by 49818 on 25-7-1.
//

#ifndef TREE_H
#define TREE_H

#include <array>
#include <vector>
#include <unordered_map>
#include <set>

#include "io.h"

constexpr int BYTESIZE = 256;  // 一个字节（0~255）的大小为256；（256种符号）
constexpr int NYT_SYMBOL = 256;        // 表示NYT节点的符号码
constexpr int INT_SYMBOL = -1;
constexpr int MAX_NODES = 257*2-1;

// 定义节点
struct Node
{
    int data;        // 存储字节数据
    int weight;      // 节点权重
    int number;

    Node* left;
    Node* right;
    Node* parent;


    Node( int _data,int _weight,int _number, Node* _left = nullptr,Node* _right = nullptr,Node* _parent = nullptr):
    data(_data),weight(_weight),number(_number),left(_left),right(_right),parent(_parent){};

    Node();
    bool operator < (const Node& n) const;

    bool isLeaf() const;
};


struct NodeComparator {
    bool operator()(const Node* a, const Node* b) const {
        // 在set中，我们只关心number，因为同一set内weight相同
        // 按编号降序排序，这样第一个元素就是leader
        return a->number > b->number;
    }
};

class AHTree
{
private:
    Node *nyt;  // 外部节点
    Node *root;

    // 节点池优化
    std::array<Node,MAX_NODES> nodePool;
    int nodePoolNextIdx;
    Node* newNode(int data,int weight,int number);

    std::unordered_map<int,Node*> nodeMap;
    std::unordered_map<int, std::set<Node*, NodeComparator>> blocks;

    int nextNumber;


public:
    AHTree();
    ~AHTree();

    void encode(unsigned char sym,IOdevice& io);
    void getCode(int symbol,IOdevice& io);
    void print();
    Node* getRoot() const;
    Node* splitNyt(unsigned char sym);
    void update(Node* p);
    void swapNodes(Node* node1, Node* node2);
    void preOrder(Node* p);
    Node* findBlockLeader(int w);
};


#endif //TREE_H
