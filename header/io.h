//
// Created by 49818 on 25-7-5.
//

#ifndef IO_H
#define IO_H
#include <cstdint>
#include <fstream>
#include <vector>
#include "zip.h"

class AHTree;
constexpr size_t BUFFER_SIZE = 1024 * 128;
constexpr uint64_t UPDATE_INTERVAL = 1024 * 128;

class IOdevice
{
    public:
    IOdevice(std::istream& in, std::ostream& out);
    ~IOdevice();

    // 禁止拷贝构造和赋值，管理文件资源
    IOdevice(const IOdevice &) = delete;
    IOdevice &operator=(const IOdevice &) = delete;

    // 文件操作

    // 检查文件成功打开
    bool good() const;


    // 获取输入文件总大小
    uint64_t getInputFileSize();
    // 获取总共写入的位数
    uint64_t getTotalBitsWritten() const;
    // 将文件头的字节数计入总位数
    void accountForHeader(uint64_t headerBytes);
    // 写入一个64位整数
    void writeHeader(uint64_t v);
    // 读入一个64位整数
    bool readHeader(uint64_t &v);

    // 位操作
    void writeBit(bool bit);
    bool readBit(bool& bit);

    // 字节操作
    void writeByte(unsigned char byte);
    bool readByte(unsigned char &byte);

    // 刷新写缓冲区到文件
    void flushWriteBuffer();

    private:

    // 从文件读入缓冲区
    bool fillReadBuffer();

    std::istream& ifs;
    std::ostream& ofs;

    // 写操作
    // 主写入缓冲区
    std::vector<unsigned char> writeBuffer;
    // 指向当前写入缓冲区位置的指针
    size_t writeBufferPos;
    // 对位进行操作时的缓冲区(缓冲一个字节)，因为进行写入的最小单位是字节，要将写入的位拼成字节进行写入
    unsigned char writeBitBuffer;
    // 指向当前位缓冲区位置的指针
    int writeBitPos;
    // 统计有多少字节写入
    uint64_t totalBitsWritten;

    // 读操作
    // 主读取缓冲区
    std::vector<unsigned char> readBuffer;
    // 指向当前读取缓冲区位置的指针
    size_t readBufferPos;
    // 获取缓冲区中的字节数(有的时候读取到的字节数可能小于缓冲区大小)
    size_t bytesInReadBuffer;
    // 对位进行操作时的读缓冲区(将读取到的字节分解成8位进行处理)
    unsigned char readBitBuffer;
    // 记录位缓冲区剩余的未读取的位数
    int bitsLeftInReadBuffer;

};


void showProgress(const std::string& prefix,uint64_t cur,uint64_t total);

void clearLine();
#endif //IO_H
