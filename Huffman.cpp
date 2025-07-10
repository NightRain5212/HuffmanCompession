//
// Created by 49818 on 25-7-6.
//
#include "header/Huffman.h"

#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stack>
#include <vector>

#include "header/AdaptiveHuffmanTree.h"
#include "header/io.h"

bool HNode::isLeaf()
{
    if (left == nullptr && right == nullptr)
    {
        return true;
    }
    return false;
}

// 后序遍历释放树的节点
void Huffman::freeTree(HNode* node)
{
    if (node == nullptr)
        return;
    freeTree(node->left);
    freeTree(node->right);
    delete node;
}

// 从文件构造哈夫曼树
Huffman::Huffman()
{
    pq = std::priority_queue<HNode*,std::vector<HNode*>,CompareHNode> ();
    nodeMap = std::unordered_map<int,HNode*>();
    countMap = std::unordered_map<int,int>();
}

HNode* Huffman::getRoot()
{
    return root;
}

void Huffman::serializeNode(HNode* node, IOdevice& io)
{
    // 先序序列化
    if (node->isLeaf())
    {
        // 叶子：1+原始编码
        io.writeBit(true);
        unsigned char sym = node->data;
        for (int i=7;i>=0;i--)
        {
            io.writeBit((sym >> i) & 1);
        }
    } else
    {
        // 中间节点：0
        io.writeBit(false);
        // 先序递归序列化
        serializeNode(node->left,io);
        serializeNode(node->right,io);
    }
}

void Huffman::serializeTree(IOdevice& io)
{
    if (root == nullptr)
        return;
    serializeNode(root,io);
}

HNode* Huffman::deserializeNode(IOdevice& io)
{
    // 判断是中间节点还是叶子
    bool bit;
    if (!io.readBit(bit))
    {
        return nullptr;
    }

    if (bit)
    {
        // 叶子节点，读入接下来8位构建一个字节
        unsigned char sym = 0;
        for (int i=7;i>=0;i--)
        {
            bool charBit;
            if (io.readBit(charBit))
            {
                if (charBit)
                {
                    sym = sym | (1<<i);
                }
            }
            else
            {
                return nullptr;
            }
        }
        // 返回新节点
        return new HNode(sym,0);
    }
    else
    {
        // 构建中间节点
        HNode* internal = new HNode(0,0);
        // 先序反序列化递归
        internal->left = deserializeNode(io);
        internal->right = deserializeNode(io);
        return internal;
    }
}

void Huffman::deserializeTree(IOdevice& io)
{
    this->root = deserializeNode(io);
}



Huffman::~Huffman()
{
    freeTree(root);
}

void Huffman::processSingleFile(const std::string& input)
{
    std::ifstream inputFile(input,std::ios::binary);
    std::ofstream dummy;
    if (!inputFile.is_open())
    {
        std::cout<< "Error opening file"<<input<<std::endl;
        return;
    }
    IOdevice io(inputFile,dummy);
    unsigned char byte;
    while (io.readByte(byte))
    {
        countMap[byte]++;
    }

    inputFile.close();
    dummy.close();
}

void Huffman::buildFromFiles(std::vector<std::string>& inputFiles)
{
    for (const auto& inputFile : inputFiles)
    {
        processSingleFile(inputFile);
    }
    for (auto& p: countMap)
    {
        HNode* newnode = new HNode(p.first,p.second);
        pq.push(newnode);
        nodeMap[p.first] = newnode;
    }

    while (pq.size() > 1)
    {
        HNode* left = pq.top();  pq.pop();
        HNode* right = pq.top();  pq.pop();
        HNode* internal = new HNode(-1,left->weight+right->weight);
        internal->left = left;
        internal->right = right;
        left->parent = internal;
        right->parent = internal;
        pq.push(internal);
    }
    root = pq.top();
    pq.pop();
}

void Huffman::getCode(int sym,IOdevice& io)
{
    if (codeMap.contains(sym))
    {
        for (const bool& bit: codeMap[sym])
        {
           io.writeBit(bit);
        }
    }
    else
    {
        HNode* p = nodeMap[sym];
        std::stack<bool> s;
        while (p!=nullptr && p->parent!=nullptr)
        {
            if (p->parent->left == p) s.push(false);
            else s.push(true);
            p = p->parent;
        }
        while (!s.empty())
        {
            bool bit = s.top(); s.pop();
            io.writeBit(bit);
            codeMap[sym].push_back(bit);
        }
    }
}

