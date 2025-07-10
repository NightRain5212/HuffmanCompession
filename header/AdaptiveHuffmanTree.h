//
// Created by 49818 on 25-7-1.
//

#ifndef TREE_H
#define TREE_H

#include <array>

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


class AHTree
{
private:
    Node *nyt;  // 外部节点
    Node *root;

    // 节点池优化
    std::array<Node,MAX_NODES> nodePool;
    int nodePoolNextIdx;
    Node* newNode(int data,int weight,int number);

    std::array<Node*,BYTESIZE> symbolToNode;
    std::array<Node*,MAX_NODES+1> numberToNode;

    int nextNumber;
    void swapNodes(Node* node1, Node* node2);
    void update(Node* p);
    void getCode(Node* node,IOdevice& io);
    Node* findLeaderInBlock(Node* node);

public:
    AHTree();
    ~AHTree() = default;

    void encode(unsigned char sym,IOdevice& io);
    unsigned char decode(IOdevice& io);
    Node* getRoot() const;
};


#endif //TREE_H
