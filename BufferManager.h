#ifndef _BUFFERMANAGER_H
#define _BUFFERMANAGER_H
#define MAXBLOCKNUM 40//buffer中的block数量

#include "MiniSQL.h"
#include <math.h>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
using namespace std;


class BufferBlock //描述了block的信息
{
public:
    bool isWritten;
    bool isValid;
    string fileName;
    int blockOffset;
    int LRUValue;
    bool pin;
    char values[BLOCK];

    BufferBlock()
    {
        initialize();
    }

    void initialize()
    {
        isWritten = false;//没被更改
        isValid = true;
        fileName = "NULL";
        blockOffset = 0;
        LRUValue = 0;
        pin = false;//没被锁住

        for(int i = 0; i < BLOCK; i++)  values[i] = '.'; /// 测试的时候把'\0'改为了'.'
    }

};

class BufferManager
{

public:

    BufferBlock buffer[MAXBLOCKNUM];//数组代表buffer，内含若干个block（buffer类型）

    //constructor & destructor
    BufferManager();//bufferBlock中每个元素调用initialize()
    ~BufferManager();//清空这个buffer的内容时，bufferBlock中每个元素调用flashback()回存入file.

    void flashBack(int bufferNum);	//编号为bufferNum的块的内容输入到文件里
    void useBlock(int bufferNum);   /// 使用编号为bufferNum的块【替换】的时候调用这个函数,重置所有LRU值

    //三个函数：获取buffer中编号
    int getIfIsInBuffer(string fileName, int blockOffset);
    int getEmptyBuffer();
    int getBufferLRU();

    //从文件中取块存入buffer
    void readBlock(string fileName, int blockNumInBuffer, int blockOffset, int blockNum);

    //提供给其他模块的函数
    char *getBufferAddr(string fileName, int addr, bool readOrWrite);//提供给Index：给出buffer中block的头指针;read = 0; write = 1
    void freePin(int blockNum);//提供给Index：为buffer中block解锁
    void getBufferData(string fileName, int addr,  void *bufferAcceptData, int elemSize, int elemCount);//提供给Record：给出指定记录内容
    void writeBufferData(string fileName, int addr,  void *bufferAcceptData, int elemSize, int elemCount);//提供给Record:将record给的数据写到buffer的block中
    void clear();
};

#endif
