//
// Created by 49818 on 25-7-6.
//

#ifndef HUFFMAN_H
#define HUFFMAN_H
#include <bitset>
#include <map>
#include <queue>
#include <string>
#include <unordered_map>

#include "io.h"

struct HNode
{
    int data;
    int weight;
    HNode* left;
    HNode* right;
    HNode* parent;
    HNode(int _data,int _weight,HNode* left = nullptr,HNode* right = nullptr,HNode* parent = nullptr):
    data(_data),weight(_weight),left(left),right(right),parent(parent){};

    bool isLeaf();

};

struct CompareHNode
{
    bool operator()(const HNode* n1, const HNode* n2) const
    {
        return n1->weight > n2->weight;
    }
};

class Huffman {
private:
    HNode* root;
    std::priority_queue<HNode*,std::vector<HNode*>,CompareHNode> pq;
    std::unordered_map<int,HNode*> nodeMap;
    std::unordered_map<int,int> countMap;
    std::unordered_map<int, std::vector<bool>> codeMap;
    void freeTree(HNode* node);
    // 序列化与反序列化
    void serializeNode(HNode* node,IOdevice& io);
    HNode* deserializeNode(IOdevice& io);

public:
    explicit Huffman();
    void processSingleFile(const std::string& inputFile);
    void buildFromFiles(std::vector<std::string>& inputFiles);
    void getCode(int sym,IOdevice& io);
    HNode* getRoot();

    void serializeTree(IOdevice& io);
    void deserializeTree(IOdevice& io);

    ~Huffman();
};



#endif //HUFFMAN_H
