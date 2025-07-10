//
// Created by 49818 on 25-7-6.
//

#ifndef CAESAR_H
#define CAESAR_H
#include <cstdint>
#include <string>


// 处理偏移量
uint64_t caesar(const std::string& input,const std::string& output ,int shift);

// 加密
void encrypt(const std::string& input,int shift);

// 解密
void decrypt(const std::string& input,int shift);

#endif //CAESAR_H
