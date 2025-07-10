//
// Created by 49818 on 25-7-7.
//

#ifndef ZIP_H
#define ZIP_H
#include <cstdint>
#include <string>
#include <vector>

class AHTree;

// 文件标识
const char SYMBOL[5] = "HUFF";

struct FileMetaData
{
    char fileName[256]; // 便于io
    uint64_t originalSize;
};

void zip(const std::string& target, const std::vector<std::string>& inputFiles);

void extract(const std::string& source);

void zip_huffman(const std::string& target, std::vector<std::string>& inputFiles);

void extract_huffman(const std::string& source);

#endif //ZIP_H
