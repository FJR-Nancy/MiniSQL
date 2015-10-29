/*
 * BPlusTreeIndex.h
 *
 *  Created on: 2013.10.13
 *      Author: Zhao Jingchen
 *
 */
#ifndef BPLUSTREEINDEX_H_
#define BPLUSTREEINDEX_H_

#include "MiniSQL.h"
#include "TreeNode.h"
#include "BufferManager.h"
#include <string>
#include <vector>
#include <list>
#include <stdlib.h>
#include <fstream>
using namespace std;
class bufferBlock
{
public:
    TreeNode *block;
    int blockAddress;
    bool readOnly;

    bufferBlock(TreeNode *block, int blockAddress, bool readOnly)
    {
        this->block = block;
        this->blockAddress = blockAddress;
        this->readOnly = readOnly;
    }
};

class BPlusTreeIndex
{
private:
    int path[10]; //path[9] is the path length
    vector<bufferBlock> secondaryBuffer;
    Index &index;

    //------------  insert key  -----------------
    void splitLeafNode(TreeNode *node);
    void insertInParent(TreeNode *node, string key, TreeNode *newNode);
    void insertInInternal(TreeNode *node, string key, TreeNode *newNode);
    void splitInternalNode(TreeNode *node);
    //------------  remove key  -----------------
    void deleteKey(TreeNode *node, string key);
    void redistribution(TreeNode *node, TreeNode *sibling, int dire,
                        string midKey, TreeNode *parent);
    bool reconstruct(TreeNode *node);
    void merge(TreeNode *node, TreeNode *lost, string key);
    //------------  assistants  -----------------
    TreeNode *findBlockFor(const string key, bool readOnly);
    int findParentAddr(TreeNode *node);
    //------------  block read-write  -----------------
    TreeNode *createNode(bool isLeaf, bool isRoot, int itemNum, Pointer lmptr =
                             NULL, Pointer rmptr = NULL); //create a node object in memory
    TreeNode *loadNode(Pointer blockAddr, bool readOnly = true);
    void writeNode(TreeNode *node);
    void removeNode(TreeNode *const node);
    //------------  secondary buffer  -----------------
    void addTobuffer(TreeNode *node, bool readOnly = true);
    TreeNode *findInBuffer(int address, bool readOnly = true);
    void removeFromBuffer(int address);
    
    void format(const string str,string& key);

public:
    //------------  constructors and destructor  -----------------
    BPlusTreeIndex(Index &indexInfo); 	//used to find,insert,and delete.
    virtual ~BPlusTreeIndex(); //write back tree nodes and delete all the node objects in memory

    //------------  public API  -----------------
    bool createIndex(vector<string> &keys, vector<Pointer> &ptrs);
    /*index is used to record information produced by creating index tree
     (indexName,keySize,freeList,blocknum and rootAddress)
     other information should be provided by API model*/
    static bool removeIndex(string indexName); //static method used to delete an index

    Pointer find(const string key); //string here is actually a binary area end with 0
    vector<Pointer> find(Condition con);
    bool insert(const string key, const Pointer keyPtr);
    bool remove(const string key);
    bool removeSome(const vector<string> &keys);
    void clear();

};

#endif /* BPLUSTREEINDEX_H_ */
