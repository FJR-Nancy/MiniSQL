#include "BufferManager.h"

BufferManager::BufferManager()
{
    for(int i = 0; i < MAXBLOCKNUM; i++)
    {
        buffer[i].initialize();
    }
}

BufferManager::~BufferManager()
{
    for(int i = 0; i < MAXBLOCKNUM; i++)
    {
        flashBack(i);
    }
}
void BufferManager::clear(){
    for(int i=0;i<MAXBLOCKNUM;i++)
        buffer[i].initialize();
}

void BufferManager::flashBack(int bufferNum)
{
    if(buffer[bufferNum].isWritten == false) return;//是否被修改过。true就重新写回文件，false不用管它
    string fileName = buffer[bufferNum].fileName;
    FILE *fp;
    fp = fopen(fileName.c_str(), "rb+");
    //fopen_s(&fp, fileName.c_str(), "rb+");

    fseek(fp, buffer[bufferNum].blockOffset, 0);
    fwrite(buffer[bufferNum].values, 4096, 1, fp);
    buffer[bufferNum].initialize();
    useBlock(bufferNum);//一次替换完毕，更新所有块的LRU
    fclose(fp);
}

//将文件内容读入bufferBlock
void BufferManager::readBlock(string fileName, int blockNumInBuffer, int blockOffsetInFile, int blockNum)
{
    buffer[blockNumInBuffer].isWritten = false;//// 以防万一
    buffer[blockNumInBuffer].isValid = false;//// 以防万一
    buffer[blockNumInBuffer].fileName = fileName;
    buffer[blockNumInBuffer].blockOffset = blockOffsetInFile;/// add: 本快的offset置为与file内offset相同

    fstream  fin(fileName.c_str(), ios::in);
    fin.seekp(BLOCK * blockNum, fin.beg); //读出含有目的数据内容的块（目的数据在中间）
    fin.read(buffer[blockNumInBuffer].values, BLOCK);//向buffer的block中读入4096B
    fin.close();
}

int BufferManager::getIfIsInBuffer(string fileName, int blockOffsetInFile)
{
    for(int i = 0; i < MAXBLOCKNUM; i++)
    {
        if((buffer[i].fileName == fileName) && (buffer[i].blockOffset == blockOffsetInFile))  return i;//block的offset是块头到文件头的距离
    }
    return -1;//不在buffer中
}

int BufferManager::getEmptyBuffer() //找到buffer数组中 第一个 可用的空block编号
{
    for(int i = 0; i < MAXBLOCKNUM; i++)
    {
        if(buffer[i].isValid == true)
        {
            buffer[i].isValid = false;
            buffer[i].initialize();
            return i;
        }
    }
    return -1;//buffer已满  没有空block
}

int BufferManager::getBufferLRU()
{
    int highestLRUValue = buffer[0].LRUValue;
    int bufferNum = 0;/// add:=0  LRU选取的block的数组下标，LRU全相等的时候就替换第一块

    for(int i = 0; i < MAXBLOCKNUM; i++) //找到最大的LRU块
    {
        if(buffer[i].LRUValue > highestLRUValue)
        {
            highestLRUValue = buffer[i].LRUValue;
            bufferNum = i;//最大LRU块的下标
        }
    }
    flashBack(bufferNum);//被替换的块回写入文件////////////////////////////////////cancel!
    buffer[bufferNum].isValid = false;/// ？这里需要吗
    return bufferNum;
}

//供调用的四个函数
void BufferManager::freePin(int blockNum) //供index调用的解锁函数
{
    buffer[blockNum].pin = false;
}


char *BufferManager::getBufferAddr(string fileName, int addr, bool readOrWrite) //read=0,write=1
{

    int blockNum = addr / BLOCK; //数据存在于file的第 blockNum+1 块内，从blockNum末尾开始读取
    //int blockOffset = addr % BLOCK; //所需数据在块内的偏移量
    int blockOffsetInFile = blockNum * BLOCK; /// 与文件头的距离
    int blockNumInBuffer;//block在buffer中的位置


    if(getIfIsInBuffer(fileName, blockOffsetInFile) != -1) //已经存在于buffer中
    {
        blockNumInBuffer = getIfIsInBuffer(fileName, blockOffsetInFile);//查找块在buffer中的编号
        useBlock(blockNumInBuffer);//// 更新所有buffer block的LRU值
        if(readOrWrite == 1)	buffer[blockNumInBuffer].isWritten = 1;//// 如果index要写，则置为1表示此块写过
        return buffer[blockNumInBuffer].values;// 返回数据地址
    }
    else //需要的内容不在buffer中
    {
        if(getEmptyBuffer() != -1) //buffer中有空block
        {
            blockNumInBuffer = getEmptyBuffer();//取一块空的出来
            //// file块中内容存入buffer块
            useBlock(blockNumInBuffer);//// 更新LRU
            buffer[blockNumInBuffer].isValid = false;/// add:本块标记为不能用（因为已经读入数据了）
        }
        else //满了则用LRU找到应被替换的块,并更新LRU值
        {
            blockNumInBuffer = getBufferLRU();
            //flashBack(blockNumInBuffer);//// 原来的块写回到文件中
            useBlock(blockNumInBuffer);  /// 更新LRU
            buffer[blockNumInBuffer].isWritten = false;//// add:新块标记为未更改
        }

        readBlock(fileName, blockNumInBuffer, blockOffsetInFile, blockNum);//将文件块读到buffer[blockNumInBuffer]，并设置isWritten isValid
        if(readOrWrite == 1)	buffer[blockNumInBuffer].isWritten = 1;//// 如果index要写，则置为1表示此块写过
        return buffer[blockNumInBuffer].values;
    }
}

//?????????如何修改pin值
//将buffer中的内容读到给定的地址中
void BufferManager::getBufferData(string fileName, int addr,  void *bufferAcceptData, int elemSize, int elemCount )
{
    int blockOffset = addr % BLOCK; //所需数据在块内的偏移量

    char *bufferAddr = getBufferAddr(fileName, addr, 0);//所需数据存储在buffer中的地址。values数组
    //(*bufferAddr).pin = true;
    memcpy(bufferAcceptData, bufferAddr + blockOffset, elemSize * elemCount);
    //(*bufferAddr).pin = false;
}

//?????????? 如何修改isWritten和pin值
//将record给的数据写到buffer的block中
void BufferManager::writeBufferData(string fileName, int addr,  void *bufferAcceptData, int elemSize, int elemCount)
{
    int blockOffset = addr % BLOCK; //所需数据在块内的偏移量

    char *bufferAddr = getBufferAddr(fileName, addr, 1);//所需数据存储在buffer中的地址（从头开始）
    //(*bufferAddr).pin = true;
    memcpy(bufferAddr + blockOffset, bufferAcceptData, elemSize * elemCount); /// 写到buffer里面去
    //(*bufferAddr).isWritten = true;
    //(*bufferAddr).pin = false;
}



void BufferManager::useBlock(int bufferNum) //仅仅用来重置所有的LRU值
{
    for(int i = 0; i < MAXBLOCKNUM; i++)
    {
        if(i == bufferNum )
        {
            buffer[bufferNum].LRUValue = 0;
        }
        else buffer[i].LRUValue++;/// 其他的block LRU值++
    }
}
