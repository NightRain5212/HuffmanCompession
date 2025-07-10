//
// Created by 49818 on 25-7-1.
//

#include "header/AdaptiveHuffmanTree.h"

#include <iostream>
#include <algorithm>

Node::Node()
{
    data = INT_SYMBOL;
    weight = 0;
    number = 0;
    left = nullptr;
    right = nullptr;
    parent = nullptr;

}

bool Node::operator<(const Node& n) const
{
    if (weight == n.weight) return number < n.number;
    return weight < n.weight;
}

bool Node::isLeaf() const
{
    if (left == nullptr && right == nullptr) return true;
    return false;
}

// 从节点池获取新节点
Node* AHTree::newNode(int data, int weight, int number)
{
    Node* node = &nodePool[nodePoolNextIdx++];
    node->data = data;
    node->weight = weight;
    node->number = number;
    node->parent = nullptr;
    node->left = nullptr;
    node->right = nullptr;
    // 建立编号映射
    numberToNode[number] = node;
    return node;
}

// 构造函数
AHTree::AHTree():nodePool(),nodePoolNextIdx(0)
{
    symbolToNode.fill(nullptr);
    numberToNode.fill(nullptr);

    nextNumber = MAX_NODES;
    root = newNode(NYT_SYMBOL,0,nextNumber--);
    nyt = root;

}


// 交换两个节点信息及树上的位置
void AHTree::swapNodes(Node* node1, Node* node2)
{
    if (!node1 || !node2 || node1 == node2 ||node1->parent == nullptr || node2->parent == nullptr)
        return;

    // 交换节点的父母指针
    Node* p1 = node1->parent;
    Node* p2 = node2->parent;
    std::swap(node1->parent,node2->parent);

    // 交换两个节点在树中的实际位置
    if (p1)
    {
        if (node1 == p1->left) p1->left = node2;
        else p1->right = node2;
    }

    if (p2)
    {
        if (node2 == p2->left) p2->left = node1;
        else p2->right = node1;
    }

    // 交换编号对应表后交换编号。
    std::swap(numberToNode[node1->number],numberToNode[node2->number]);
    std::swap(node1->number,node2->number);

    // 如果节点是根，则更新根指针
    if (root == node1) root = node2;
    else if (root == node2) root = node1;

}

Node* AHTree::getRoot() const
{
    return root;
}

// 找到当前权重块的编号最大者
// 保证相同权重的节点拥有连续的编号
Node* AHTree::findLeaderInBlock(Node* node)
{
    int w = node->weight;
    for (int num = node->number+1; num <= MAX_NODES; num++)
    {
        Node* nextNode = numberToNode[num];
        if (nextNode == nullptr || nextNode->weight!= w)
        {
            // 到达权重块的末尾，即编号最大的节点
            return numberToNode[num-1];
        }
    }
    return numberToNode[MAX_NODES];
}

// 编码
void AHTree::encode(unsigned char sym,IOdevice& io)
{
    Node* leaf = symbolToNode[sym];
    if (leaf  != nullptr)
    {
        getCode(leaf,io);
        update(leaf);
    }
    else
    {
        getCode(nyt,io);
        for (int i=7;i>=0;i--)
        {
            io.writeBit((sym >> i) & 1);
        }
        // 分裂旧的nyt节点
        Node* oldnyt = nyt;
        Node* newLeaf = newNode(sym,1,oldnyt->number-1);
        Node* newnyt = newNode(NYT_SYMBOL,0,oldnyt->number-2);

        // 建立连接关系
        oldnyt->left = newnyt;
        oldnyt->right = newLeaf;
        newnyt->parent = oldnyt;
        newLeaf->parent = oldnyt;
        // 将旧的nyt作为中间节点
        oldnyt->data = INT_SYMBOL;

        // 更新nyt
        nyt = newnyt;

        // 更新映射表
        symbolToNode[sym] = newLeaf;
        // 从父结点开始向上更新
        update(oldnyt);
    }
}

void AHTree::update(Node* n)
{
    Node* p = n;
    while (p != nullptr)
    {
        Node* leader = findLeaderInBlock(p);
        if (leader != nullptr && leader != p && leader != p->parent)
        {
            swapNodes(p,leader);
        }
        //  增加 p 的权重
        p->weight += 1;

        // 往上维护性质
        p = p->parent;
    }
}

void AHTree::getCode(Node* node,IOdevice& io)
{
    bool codeBuffer[MAX_NODES];
    int count = 0;
    // 从叶子向上遍历到根节点，并将路上的位压入栈中
    Node* cur = node;

    while (cur != nullptr && cur->parent != nullptr)
    {
        if (cur == cur->parent->left)
        {
            codeBuffer[count++] = false;
        } else
        {
            codeBuffer[count++] = true;
        }
        cur = cur->parent;
    }

    // 弹栈写入
    for (int i=count-1;i>=0;i--)
    {
        io.writeBit(codeBuffer[i]);
    }
}

unsigned char AHTree::decode(IOdevice& io)
{
    Node* cur = root;
    // 不是叶子节点就随着bit流移动
    while (!cur->isLeaf())
    {
        bool bit;
        if (!io.readBit(bit))
        {
            std::cerr << "在解码树路径时比特流不完整。" << std::endl;
            return -1;
        }
        // 0向左，1向右
        cur = bit ? cur->right : cur->left;
    }

    unsigned char sym = 0;
    // 遇到nyt节点，读取接下来8位构建新字符
    if (cur == nyt)
    {
        for (int i=0;i<8;i++)
        {
            bool bit;
            if (!io.readBit(bit))
            {
                std::cerr << "在解码树路径时比特流不完整。" << std::endl;
                return -1;
            }
            sym = (sym<<1) | bit;
        }

        // 分裂nyt
        Node* oldnyt = nyt;
        Node* newLeaf = newNode(sym,1,oldnyt->number-1);
        Node* newnyt = newNode(NYT_SYMBOL,0,oldnyt->number-2);

        oldnyt->left = newnyt;
        oldnyt->right = newLeaf;
        newnyt->parent = oldnyt;
        newLeaf->parent = oldnyt;
        oldnyt->data = INT_SYMBOL;
        nyt = newnyt;
        symbolToNode[sym] = newLeaf;
        // 更新树
        update(oldnyt);
    } else
    {
        // 遇到已经出现的字符
        sym = cur->data;
        // 更新对应的节点
        update(cur);
    }
    return sym;
}