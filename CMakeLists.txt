cmake_minimum_required(VERSION 3.31)
project(huffman)

set(CMAKE_CXX_STANDARD 20)

add_executable(huffman main.cpp
        AdaptiveHuffmanTree.cpp
        header/AdaptiveHuffmanTree.h
        io.cpp
        header/io.h
        Huffman.cpp
        header/Huffman.h
        Caesar.cpp
        header/Caesar.h
        zip.cpp
        header/zip.h
)
