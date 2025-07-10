

#include <iostream>

#include "header/AdaptiveHuffmanTree.h"
#include "header/Caesar.h"
// 定义ANSI转义序列
const char* HIDE_CURSOR = "\x1b[?25l"; // 隐藏光标
const char* SHOW_CURSOR = "\x1b[?25h"; // 显示光标


void printhelp()
{
    std::cout<<"=== HELP ===\n";
    std::cout<<"1. 动态huffman压缩文件：huffman ah-z <outputFile> <inputFile1> <inputFile2> ...\n";
    std::cout<<"2. 动态huffman解压文件：huffman ah-d <inputFile>\n";
    std::cout<<"3. 静态huffman压缩文件：huffman h-z <outputFile> <inputFile1> <inputFile2> ...\n";
    std::cout<<"4. 静态huffman解压文件：huffman h-d <inputFile>\n";
    std::cout<<"5. 凯撒加密文件：huffman -en <inputFile> <key>\n";
    std::cout<<"6. 凯撒解密文件：huffman -de <inputFile> <key>\n";

    std::cout<<"7. 帮助：huffman -help";
}


int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout<<"ERROR: 参数错误！\n";
        printhelp();
        return 1;
    }
    AHTree tree;
    std::string command = argv[1];
    std::string inputFile;
    // 将全局C++ locale设置为操作系统的默认locale
    std::locale::global(std::locale(""));
    // 对于cout，还需要同步C风格的IO
    std::cout.imbue(std::locale());

    if (command == "ah-z"){
        if (argc < 4)
        {
            std::cout<<"Usage: ah-z <outputFile> <inputFiles> \n";
            return 1;
        }
        std::string outputFile = argv[2];
        std::vector<std::string> inputFiles;
        for (int i=3;i<argc;i++)
        {
            inputFiles.emplace_back(argv[i]);
        }
        std::cout <<HIDE_CURSOR<<"\n";
        zip(outputFile,inputFiles);
        std::cout <<SHOW_CURSOR<<"\n";
    }
    else if (command == "ah-d")
    {
        if (argc < 3)
        {
            std::cout<<"Usage: ah-d <inputFile>\n";
            return 1;
        }
        inputFile = argv[2];
        std::cout <<HIDE_CURSOR<<"\n";
        extract(inputFile);
        std::cout <<SHOW_CURSOR<<"\n";
    }
    else if (command == "-en")
    {
        if (argc != 4)
        {
            std::cout<<"Usage: -en <inputFile> <shift>\n";
            return 1;
        }
        inputFile = argv[2];
        int shift = atoi(argv[3]);
        std::cout <<HIDE_CURSOR<<"\n";
        encrypt(inputFile,shift);
        std::cout <<SHOW_CURSOR<<"\n";
    }
    else if (command == "-de")
    {
        if (argc != 4)
        {
            std::cout<<"Usage: -de <inputFile> <shift>\n";
            return 1;
        }
        inputFile = argv[2];
        int shift = atoi(argv[3]);
        std::cout <<HIDE_CURSOR<<"\n";
        decrypt(inputFile,shift);
        std::cout <<SHOW_CURSOR<<"\n";
    }
    else if (command == "h-z"){
        if (argc < 4)
        {
            std::cout<<"Usage: h-z <outputFile> <inputFiles> \n";
            return 1;
        }
        std::string outputFile = argv[2];
        std::vector<std::string> inputFiles;
        for (int i=3;i<argc;i++)
        {
            inputFiles.emplace_back(argv[i]);
        }
        std::cout <<HIDE_CURSOR<<"\n";
        zip_huffman(outputFile,inputFiles);
        std::cout <<SHOW_CURSOR<<"\n";
    }
    else if (command == "h-d")
    {
        if (argc < 3)
        {
            std::cout<<"Usage: h-d <inputFile>\n";
            return 1;
        }
        inputFile = argv[2];
        std::cout <<HIDE_CURSOR<<"\n";
        extract_huffman(inputFile);
        std::cout <<SHOW_CURSOR<<"\n";
    }


    else if (command == "help")
    {
        if (argc != 2)
        {
            std::cout<<"Usage: help\n";
            return 1;
        }
        printhelp();
    }
    else
    {
        std::cout<<"ERROR: 未知命令!\n";
        printhelp();
        return 1;
    }
    return 0;
}