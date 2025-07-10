//
// Created by 49818 on 25-7-7.
//

#include "header/zip.h"

#include <chrono>

#include "header/AdaptiveHuffmanTree.h"
#include <cstring>
#include <iostream>

#include "header/Huffman.h"


void zip(const std::string& target, const std::vector<std::string>& inputFiles)
{
    std::cout<<"正在压缩..." << std::endl;

    std::ofstream targetFile(target,std::ios::binary);
    if (!targetFile)
    {
        std::cerr << "错误：无法创建压缩文件"<< target << std::endl;
        return;
    }
    // 预处理：元数据
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<FileMetaData> metadataList;
    uint64_t totalOriginalSize = 0;

    for (const auto& inputFile : inputFiles)
    {
        std::ifstream tempFile(inputFile,std::ios::binary);
        if (!tempFile)
        {
            std::cerr<<"警告：无法打开 "<< inputFile << "，已跳过"<<std::endl;
            continue;
        }
        FileMetaData metadata = {};
        // 记录文件名
        std::string basename = inputFile.substr(inputFile.find_last_of("/\\")+1);
        strncpy(metadata.fileName,basename.c_str(),sizeof(metadata.fileName)-1);
        metadata.fileName[sizeof(metadata.fileName) - 1] = '\0';
        // 记录文件大小
        tempFile.seekg(0,std::ios::end);
        metadata.originalSize = tempFile.tellg();
        totalOriginalSize += metadata.originalSize;

        metadataList.push_back(metadata);
    }

    // 写入元数据
    uint64_t Filenums = metadataList.size();
    targetFile.write(SYMBOL,sizeof(SYMBOL)-1);
    targetFile.write(reinterpret_cast<char*>(&Filenums),sizeof(Filenums));
    targetFile.write(reinterpret_cast<char*>(metadataList.data()),Filenums*sizeof(FileMetaData));

    AHTree tree;
    std::ifstream dummy; // 用于构造IO
    IOdevice io(dummy,targetFile);

    uint64_t totalBytesProcessed = 0;
    uint64_t totalCompressedSize = 0;
    // 顺序压缩所有文件
    for (const auto& metadata : metadataList)
    {

        std::string inputFile = metadata.fileName;
        std::string prefix = "-> 正在压缩 " + inputFile;

        std::ifstream tempFile(inputFile,std::ios::binary);
        unsigned char byte;
        uint64_t currentFileBytesProcessed = 0;
        const uint64_t currentFileSize = metadata.originalSize;
        uint64_t initialBitsWritten = io.getTotalBitsWritten();
        showProgress(prefix, 0, currentFileSize);
        while (tempFile.get(reinterpret_cast<char&>(byte)))
        {
            // 同一个树和io对象持续编码
            tree.encode(byte,io);
            currentFileBytesProcessed++;
            if (currentFileBytesProcessed % UPDATE_INTERVAL == 0)
            {
                showProgress(prefix,currentFileBytesProcessed,currentFileSize);
            }
        }
        uint64_t finalBitsWritten = io.getTotalBitsWritten();
        totalCompressedSize += (finalBitsWritten - initialBitsWritten+7)/8;
        showProgress(prefix,currentFileSize, currentFileSize);
        //  该文件处理完毕，换行
        std::cout << std::endl;

    }
    io.flushWriteBuffer();

    targetFile.close();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end-start);

    std::cout<<"压缩完成！" << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    std::cout << "总计:" << std::endl;
    std::cout << "  原始大小: " << totalOriginalSize << " 字节" << std::endl;
    std::cout << "  压缩大小: " << totalCompressedSize << " 字节" << std::endl;
    if (totalOriginalSize > 0) {
        double compressionRatio = static_cast<double>(totalCompressedSize) / totalOriginalSize;
        std::cout << "  压缩率: " << std::fixed << std::setprecision(2) << compressionRatio * 100 << "%" << std::endl;
    } else {
        std::cout << "  压缩率: N/A (无数据)" << std::endl;
    }
    std::cout << "  总耗时: " << duration.count() << " 秒" << std::endl;
}

void extract(const std::string& source)
{
    std::cout<<"正在解压..." << std::endl;
    std::ifstream sourceFile(source,std::ios::binary);
    if (!sourceFile)
    {
        std::cerr << "错误：无法打开文件："<< source << std::endl;
        return;
    }

    auto start = std::chrono::high_resolution_clock::now();
    // 读取头部，验证
    char symbolBuffer[sizeof(SYMBOL)-1];
    sourceFile.read(symbolBuffer,sizeof(symbolBuffer));
    if (strncmp(symbolBuffer,SYMBOL,sizeof(SYMBOL)-1)!=0)
    {
        std::cerr<<"错误！文件格式不正确！" << std::endl;
        return;
    }

    uint64_t Filenums = 0;
    sourceFile.read(reinterpret_cast<char*>(&Filenums),sizeof(Filenums));

    uint64_t totalOriginalSize = 0;
    std::vector<FileMetaData> metadataList(Filenums);
    sourceFile.read(reinterpret_cast<char*>(metadataList.data()),Filenums*sizeof(FileMetaData));
    for(const auto& meta : metadataList) {
        totalOriginalSize += meta.originalSize;
    }

    AHTree tree;

    std::ofstream dummy;
    IOdevice io(sourceFile,dummy);
    uint64_t totalBytesDecompressed = 0;
    // 逐个解压
    for (const auto& metadata : metadataList)
    {
        std::string prefix = "->正在解压:"+std::string(metadata.fileName);

        std::ofstream tempFile(metadata.fileName,std::ios::binary);
        if (!tempFile)
        {
            std::cerr << "警告！无法创建 " << metadata.fileName << "，已跳过。\n";
            return;
        }

        uint64_t decompressedBytes = 0;
        const uint64_t currentFileSize = metadata.originalSize;
        showProgress(prefix,decompressedBytes,currentFileSize);
        while (decompressedBytes < currentFileSize)
        {
            unsigned char sym = tree.decode(io);
            tempFile.put(sym);
            decompressedBytes++;

            if (decompressedBytes % UPDATE_INTERVAL == 0) {
                showProgress(prefix, decompressedBytes, currentFileSize);
            }
        }
        tempFile.close();
        showProgress(prefix,currentFileSize, currentFileSize);
        std::cout << std::endl;
    }
end_loop:;
    sourceFile.close();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end-start);

    std::cout << std::endl;

    std::cout <<"解压完成！"<< std::endl;
    std::cout << "总耗时: " << duration.count() << " 秒" << std::endl;

}

// 静态哈夫曼压缩
void zip_huffman(const std::string& target, std::vector<std::string>& inputFiles)
{
    std::cout<<"正在压缩..." << std::endl;

    std::ofstream targetFile(target,std::ios::binary);
    if (!targetFile)
    {
        std::cerr << "错误：无法创建压缩文件"<< target << std::endl;
        return;
    }

    // 预处理：元数据
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<FileMetaData> metadataList;
    uint64_t totalOriginalSize = 0;

    for (const auto& inputFile : inputFiles)
    {
        std::ifstream tempFile(inputFile,std::ios::binary);
        if (!tempFile)
        {
            std::cerr<<"警告：无法打开 "<< inputFile << "，已跳过"<<std::endl;
            continue;
        }
        FileMetaData metadata = {};

        // 文件名
        std::string basename = inputFile.substr(inputFile.find_last_of("/\\") + 1);
        strncpy(metadata.fileName,basename.c_str(),sizeof(metadata.fileName)-1);
        metadata.fileName[sizeof(metadata.fileName)-1] = '\0';

        // 文件大小
        tempFile.seekg(0,std::ios::end);
        metadata.originalSize = tempFile.tellg();
        tempFile.seekg(0,std::ios::beg);
        totalOriginalSize += metadata.originalSize;

        metadataList.push_back(metadata);
    }

    // 写入元数据区
    uint64_t Filenums = metadataList.size();
    targetFile.write(SYMBOL,sizeof(SYMBOL)-1);
    targetFile.write(reinterpret_cast<char*>(&Filenums),sizeof(Filenums));
    targetFile.write(reinterpret_cast<char*>(metadataList.data()),Filenums*sizeof(FileMetaData));

    // 构建哈夫曼树
    Huffman tree;
    tree.buildFromFiles(inputFiles);
    // 构建输入输出流
    std::ifstream dummy;
    IOdevice io(dummy,targetFile);

    // 序列化树到文件
    tree.serializeTree(io);

    uint64_t totalCompressedSize = 0;

    // 逐个压缩文件
    for (const auto& metadata : metadataList)
    {
        std::string inputFile = metadata.fileName;
        std::ifstream tempFile(inputFile,std::ios::binary);
        std::string prefix = "-> 正在压缩 " + inputFile;
        unsigned char byte;
        uint64_t currentFileSize = metadata.originalSize;
        uint64_t currentFileBytesProcessed = 0;
        uint64_t initalBitsWritten = io.getTotalBitsWritten();
        showProgress(prefix,currentFileBytesProcessed,currentFileSize);
        while (tempFile.get(reinterpret_cast<char&>(byte)))
        {
            tree.getCode(byte,io);
            currentFileBytesProcessed++;
            if (currentFileBytesProcessed % UPDATE_INTERVAL == 0)
            {
                showProgress(prefix,currentFileBytesProcessed,currentFileSize);
            }
        }
        uint64_t finalBitsWritten = io.getTotalBitsWritten();
        totalCompressedSize += (finalBitsWritten - initalBitsWritten+7)/8;
        showProgress(prefix,currentFileSize,currentFileSize);
        std::cout << std::endl;
    }
    io.flushWriteBuffer();

    targetFile.close();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end-start);

    std::cout << "压缩完成！" << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    std::cout << "总计:" << std::endl;
    std::cout << "  原始大小: " << totalOriginalSize << " 字节" << std::endl;
    std::cout << "  压缩大小: " << totalCompressedSize << " 字节" << std::endl;
    if (totalOriginalSize > 0) {
        double compressionRatio = static_cast<double>(totalCompressedSize) / totalOriginalSize;
        std::cout << "  压缩率: " << std::fixed << std::setprecision(2) << compressionRatio * 100 << "%" << std::endl;
    } else {
        std::cout << "  压缩率: N/A (无数据)" << std::endl;
    }
    std::cout << "  总耗时: " << duration.count() << " 秒" << std::endl;
}

// 静态哈夫曼解压
void extract_huffman(const std::string& source)
{
    std::ifstream sourceFile(source,std::ios::binary);
    if (!sourceFile)
    {
        std::cout<< "无法打开文件" << source << std::endl;
        return;
    }
    auto start = std::chrono::high_resolution_clock::now();
    char symbol[sizeof(SYMBOL)-1];
    // 读取文件标志
    sourceFile.read(symbol,sizeof(symbol));
    if (strncmp(symbol,SYMBOL,sizeof(SYMBOL)-1)!= 0)
    {
        std::cout << "文件格式错误！" << std::endl;
        return;
    }
    // 读取文件总数
    uint64_t Filenums = 0;
    sourceFile.read(reinterpret_cast<char*>(&Filenums),sizeof(Filenums));

    // 读取文件元数据列表
    std::vector<FileMetaData> metadataList(Filenums);
    sourceFile.read(reinterpret_cast<char*>(metadataList.data()),Filenums*sizeof(FileMetaData));

    Huffman tree;
    std::ofstream dummy;
    IOdevice io(sourceFile,dummy);
    uint64_t totalBytesDecompressed = 0;

    // 反序列化构建树
    tree.deserializeTree(io);

    // 逐个解压
    for (const auto& metadata : metadataList)
    {
        std::string prefix = "->正在解压:"+std::string(metadata.fileName);
        std::ofstream tempFile(metadata.fileName,std::ios::binary);
        if (!tempFile)
        {
            std::cerr << "警告！无法创建 " << metadata.fileName << "，已跳过。\n";
            continue;
        }

        HNode* cur = tree.getRoot();
        uint64_t decompressedBytes = 0;
        uint64_t currentFileSize = metadata.originalSize;
        showProgress(prefix,decompressedBytes,currentFileSize);
        while (decompressedBytes < currentFileSize)
        {
            if (cur->isLeaf())
            {
                unsigned char sym = cur->data;
                tempFile.put(sym);
                decompressedBytes++;
                totalBytesDecompressed ++;
                cur = tree.getRoot();
                if (decompressedBytes % UPDATE_INTERVAL == 0)
                {
                    showProgress(prefix,decompressedBytes,currentFileSize);
                }
            }
            else
            {
                bool bit;
                if (!io.readBit(bit)) break;
                cur = bit ? cur->right : cur->left;
            }
        }
        tempFile.close();
        showProgress(prefix,currentFileSize,currentFileSize);
        std::cout << std::endl;

    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end-start);
    std::cout << std::endl;

    std::cout <<"解压完成！"<< std::endl;
    std::cout << "总耗时: " << duration.count() << " 秒" << std::endl;


}




