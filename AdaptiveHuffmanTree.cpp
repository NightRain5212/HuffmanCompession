//
// Created by 49818 on 25-7-1.
//

#include "header/AdaptiveHuffmanTree.h"

#include <iostream>
#include <stack>

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
    return node;
}

// 构造函数
AHTree::AHTree():nodePool(),nodePoolNextIdx(0)
{
    nextNumber = MAX_NODES;
    root = newNode(NYT_SYMBOL,0,nextNumber--);
    nyt = root;

    blocks[root->weight].insert(root);


}


// 析构函数
AHTree::~AHTree()
{
    // 节点池中的节点会自动释放，无需手动释放

}

// 交换两个节点信息及树上的位置
void AHTree::swapNodes(Node* node1, Node* node2)
{
    if (!node1 || !node2 || node1 == node2) return;

    // 交换与树中位置绑定的节点编号
    std::swap(node1->number, node2->number);

    // 识别每个节点的父节点以及它是左子节点还是右子节点
    Node* parent1 = node1->parent;
    bool is_left1 = (parent1->left == node1);

    Node* parent2 = node2->parent;
    bool is_left2 = (parent2->left == node2);

    // 交换父节点指向子节点的指针
    if (is_left1) {
        parent1->left = node2;
    } else {
        parent1->right = node2;
    }

    if (is_left2) {
        parent2->left = node1;
    } else {
        parent2->right = node1;
    }

    // 交换子节点指向父节点的指针
    std::swap(node1->parent, node2->parent);
}

void AHTree::preOrder(Node* p)
{
    if (p == nullptr) return;
    std::cout<<p->data<<" ";
    preOrder(p->left);
    preOrder(p->right);
}

Node* AHTree::splitNyt(unsigned char sym)
{
    Node* oldnyt = nyt;
    Node* newRightLeaf = newNode(sym,0,nextNumber--);

    Node* newnyt = newNode(NYT_SYMBOL,0,nextNumber--);

    nodeMap[sym] = newRightLeaf;

    blocks[newnyt->weight].insert(newnyt);
    blocks[newRightLeaf->weight].insert(newRightLeaf);

    newRightLeaf->parent = oldnyt;
    newnyt->parent = oldnyt;

    oldnyt->data = INT_SYMBOL;
    oldnyt->left = newnyt;
    oldnyt->right = newRightLeaf;

    nyt = newnyt;

    return newRightLeaf;
}

void AHTree::print()
{
    preOrder(root);
}

Node* AHTree::getRoot() const
{
    return root;
}

// 找到当前权重块的编号最大者
Node* AHTree::findBlockLeader(int w)
{

    if (blocks.count(w) && !blocks[w].empty()) {
        // set的第一个元素就是leader
        return *blocks[w].begin();
    }
    return nullptr;
}

// 编码
void AHTree::encode(unsigned char sym,IOdevice& io)
{

    Node* toUpdate = nullptr;
    if (!nodeMap.contains(sym))
    {
        // 写入nyt的编码
        getCode(NYT_SYMBOL,io);
        // 写入新符号的原始表示
        for (int i=7;i>=0;i--)
        {
            io.writeBit((sym>>i)&1);
        }
        // 分裂nyt
        toUpdate = splitNyt(sym);
    }
    else
    {
        getCode(sym,io);
        toUpdate = nodeMap[sym];
    }
    update(toUpdate);
}

void AHTree::update(Node* n)
{
    Node* p = n;
    while (p != nullptr)
    {
        int current_weight = p->weight;

        Node* leader = findBlockLeader(p->weight);
        if (leader != nullptr && leader != p && leader != p->parent)
        {
            blocks[current_weight].erase(p);
            blocks[current_weight].erase(leader);

            swapNodes(p,leader);

            blocks[current_weight].insert(p);
            blocks[current_weight].insert(leader);
        }

        blocks[current_weight].erase(p);
        if (blocks[current_weight].empty()) {
            blocks.erase(current_weight); // 如果 block 空了，最好将其从 map 中移除
        }

        //  增加 p 的权重
        p->weight += 1;

        //  将 p 加入新的权重 block
        blocks[p->weight].insert(p);
        // 往上维护性质
        p = p->parent;
    }
}

void AHTree::getCode(int sym,IOdevice& io)
{
    // 从叶子向上遍历到根节点，并将路上的位压入栈中
    std::stack<bool> codeStack;
    Node* cur = nullptr;
    if (sym == NYT_SYMBOL) cur = nyt;
    else cur = nodeMap[sym];

    while (cur != nullptr && cur->parent != nullptr)
    {
        if (cur == cur->parent->left)
        {
            codeStack.push(false);
        } else
        {
            codeStack.push(true);
        }
        cur = cur->parent;
    }

    // 弹栈写入
    while (!codeStack.empty())
    {
        io.writeBit(codeStack.top());
        codeStack.pop();
    }
}