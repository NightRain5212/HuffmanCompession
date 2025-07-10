//
// Created by 49818 on 25-7-6.
//

#include "header/Caesar.h"

#include <chrono>
#include <iostream>

#include "header/AdaptiveHuffmanTree.h"
#include "header/io.h"


uint64_t caesar(const std::string& input, const std::string& output, int shift)
{

    unsigned char byte;
    std::ifstream ifs(input,std::ios::binary);
    std::ofstream ofs(output,std::ios::binary);
    IOdevice io(ifs,ofs);
    uint64_t totalReadBytes = 0;
    uint64_t currentReadBytes = 0;

    if (!io.good())
    {
        std::cerr << "\n错误: 无法打开文件 " << input << " 或 " << output << std::endl;

        return 0;
    }
    totalReadBytes = io.getInputFileSize();
    while (io.readByte(byte))
    {
        byte = (byte + shift + BYTESIZE) % BYTESIZE;
        currentReadBytes ++;
        io.writeByte(byte);
        if (currentReadBytes % UPDATE_INTERVAL == 0)
        {
            showProgress("",currentReadBytes,totalReadBytes);
        }
    }

    showProgress("",totalReadBytes,totalReadBytes);

    return totalReadBytes;

}

void encrypt(const std::string& input, int shift)
{
    std::cout<<"加密开始..."<<std::endl;
    std::string output = input+".enc";
    auto start = std::chrono::high_resolution_clock::now();

    uint64_t totalReadBytes = 0;
    totalReadBytes = caesar(input,output,shift);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end-start;
    std::cout << std::endl;
    std::cout << "加密完成!" << std::endl;
    std::cout << "文件大小: " << totalReadBytes << " 字节" << std::endl;
    std::cout << "加密耗时: " << std::fixed << std::setprecision(3) << duration.count() << " 秒" << std::endl;
}

void decrypt(const std::string& input, int shift)
{
    std::cout<<"解密开始..."<<std::endl;

    int dot_pos = input.rfind(".enc");
    std::string output ;
    if (dot_pos != std::string::npos && dot_pos == input.length() - 4) {
        // 如果找到了，就移除 .enc 后缀作为输出文件名
        output = input.substr(0, dot_pos);
    } else {
        // 如果没有找到 .enc 后缀，可以自定义一个默认行为，比如添加 .dec
        std::cout<<"文件格式错误！\n";
        return;
    }


    auto start = std::chrono::high_resolution_clock::now();

    uint64_t totalReadBytes = 0;
    totalReadBytes = caesar(input,output,-shift);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end-start;
    std::cout << std::endl;
    std::cout << "解密完成!" << std::endl;
    std::cout << "文件大小: " << totalReadBytes << " 字节" << std::endl;
    std::cout << "解密耗时: " << std::fixed << std::setprecision(3) << duration.count() << " 秒" << std::endl;
}
