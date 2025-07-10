//
// Created by 49818 on 25-7-5.
//


#include <chrono>
#include <iomanip>
#include <iostream>

#include "header/AdaptiveHuffmanTree.h"


void showProgress(const std::string&prefix,uint64_t cur,uint64_t total)
{
    if (total == 0) return; // 避免除以零
    double percent = static_cast<double>(cur)/total;
    int width = 40;
    int pos =static_cast<int>( width * percent);
    std::cout << "\r";
    std::cout << prefix << ": ";
    std::cout<<"[";
    for (int i=0;i<width;i++)
    {
        if (i<pos) std::cout<<"=";
        else if (i==pos) std::cout<<">";
        else std::cout<<" ";
    }
    std::cout<<"]"<< std::fixed << std::setprecision(2) << percent * 100.0 << " %\r";
    std::cout.flush();
}

void clearLine() {
    std::cout << "\r" << std::string(80, ' ') << "\r";
    std::cout.flush();
}

IOdevice::IOdevice(std::istream& input, std::ostream& output):
    ifs(input),ofs(output),
    writeBuffer(BUFFER_SIZE),writeBitPos(0),writeBitBuffer(0),writeBufferPos(0),totalBitsWritten(0),
    readBuffer(BUFFER_SIZE),readBitBuffer(0),readBufferPos(0),bitsLeftInReadBuffer(0),bytesInReadBuffer(0)
{};


IOdevice::~IOdevice()
{
    // 先刷入所有待写的数据再销毁
    flushWriteBuffer();
}

bool IOdevice::good() const
{
    return ifs.good() && ofs.good();
}


uint64_t IOdevice::getInputFileSize()
{
    if (!ifs.good()) return 0;
    // 将读取指针移动至文件末尾
    ifs.seekg(0,std::ios::end);
    // 获取当前读取指针的位置，即文件大小
    uint64_t size = ifs.tellg();
    // 将读取指针移动至文件开头
    ifs.seekg(0,std::ios::beg);
    return size;
}

uint64_t IOdevice::getTotalBitsWritten() const {
    return totalBitsWritten;
}

// 统计头部大小进入总写入位数
void IOdevice::accountForHeader(uint64_t headerBytes) {
    totalBitsWritten += headerBytes * 8;
}

// 用于写入文件头，(64位无符号整数)
void IOdevice::writeHeader(uint64_t v)
{
    if (ofs.good())
    {
        // 刷新缓冲区后立即写入
        flushWriteBuffer();
        ofs.write(reinterpret_cast<const char*>(&v), sizeof(v));
    }
}

// 用于读取文件头
bool IOdevice::readHeader(uint64_t& v)
{
    if (ifs.good())
    {
        ifs.read(reinterpret_cast<char*>(&v), sizeof(v));
        return ifs.gcount() == sizeof(v);
    }
    return false;
}

// 写入一位
void IOdevice::writeBit(bool bit)
{
    // 将新位添加到位缓冲区
    writeBitBuffer = (writeBitBuffer << 1) | bit;
    writeBitPos++;      // 更新指针
    totalBitsWritten++; // 更新统计

    // 满一个字节就写入写缓冲区
    if (writeBitPos == 8)
    {
        writeByte(writeBitBuffer);
        // 重置计数器和位缓冲区
        writeBitBuffer = 0;
        writeBitPos = 0;
    }
}

// 写入字节
void IOdevice::writeByte(unsigned char byte)
{
    if (!ofs.good()) return;

    writeBuffer[writeBufferPos++] = byte;
    // 缓冲区满了，写入文件
    if (writeBufferPos == BUFFER_SIZE)
    {
        ofs.write(reinterpret_cast<const char*>(writeBuffer.data()), BUFFER_SIZE);
        writeBufferPos = 0;
    }
}

// 强制刷新写缓冲区(全部写入文件后重置)
void IOdevice::flushWriteBuffer()
{
    if (!ofs.good()) return;

    // 处理位缓冲区剩下部分
    if (writeBitPos > 0)
    {
        // 用0填充右边的部分，形成一个字节
        writeBitBuffer <<= (8-writeBitPos);
        writeByte(writeBitBuffer); // 将最后一个字节写入缓冲区
    }

    // 处理缓冲区剩余部分
    if (writeBufferPos > 0)
    {
        ofs.write(reinterpret_cast<const char*>(writeBuffer.data()), writeBufferPos);
        writeBufferPos = 0;
    }
    ofs.flush();
}

// 填充读缓冲区
bool IOdevice::fillReadBuffer()
{
    if (!ifs.good())
        return false;

    uint64_t bytesToRead = BUFFER_SIZE;

    // 读入读缓冲区
    ifs.read(reinterpret_cast<char*>(readBuffer.data()), bytesToRead);

    bytesInReadBuffer = ifs.gcount(); // 获取读取到的字节数。
    readBufferPos = 0;                // 重置指针

    return bytesInReadBuffer > 0;
}

bool IOdevice::readBit(bool& bit)
{
    // 当前字节的位已经读完
    if (bitsLeftInReadBuffer == 0)
    {
        unsigned char byteBuffer;
        // 尝试读取新字节
        if (!readByte(byteBuffer))
        {
            return false; // 读不到新字节，文件结束
        }
        readBitBuffer = byteBuffer;
        // 重置剩余未读取位数
        bitsLeftInReadBuffer = 8;
    }

    // 从位缓冲区的高位取出1位
    bitsLeftInReadBuffer--;
    bit = (readBitBuffer >> bitsLeftInReadBuffer) & 1;
    return true;
}

bool IOdevice::readByte(unsigned char& byte)
{

    // 读缓冲区耗尽，从文件中补充
    if (readBufferPos >= bytesInReadBuffer)
    {
        if (!fillReadBuffer())
        {
            return false; // 无法填充文件结束
        }
    }

    // 从缓冲区读取一个字节
    byte = readBuffer[readBufferPos++];
    return true;
}
