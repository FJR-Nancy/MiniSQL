/*
 * TreeNode.h
 *
 *  Created on: 2013.10.13
 *      Author: Zhao Jingchen
 */

#ifndef TREENODE_H_
#define TREENODE_H_
#include "MiniSQL.h"
#include <string>
#include <cstring>
#include <list>
#include <cmath>
#include <iostream>
//using namespace std;
typedef int Pointer;
// the length of string contains the end null
class NodeInfo 	//node information,this part should be stored on the disk
{
public:
    bool isLeaf;
    bool isRoot;
    int itemNum;

    NodeInfo()
    {
        isLeaf = isRoot = false;
        itemNum = 0;
    }
    NodeInfo(bool isLeaf, bool isRoot, int itemNum)
    {
        this->isLeaf = isLeaf;
        this->isRoot = isRoot;
        this->itemNum = itemNum;
    }
};

class NodeItem	//the atom of a node,a pair of key and pointer.
{
public:
    Pointer keyPtr;
    string key;

    //in leaves, keyPtr is the record pointer of the key
    //in internal nodes, keyPtr is the next pointer of key

    NodeItem(Pointer ptr, string value)
    {
        keyPtr = ptr;
        key = value;
    }
    NodeItem()
    {
        keyPtr = -1;
        key = "";
    }
};

class TreeNode
{
public:

    NodeInfo info;
    list<NodeItem> itemList;
    Pointer rightMostPtr;		//used in leaf node only
    Pointer leftMostPtr;		//used in non-leaf node only

    //above four members should be stored on the disk

    int minItemNum;
    int maxItemNum;
    int address;		//the block number of the node in index file
    int type;
    //above three members are calculated in memory, and don't need to be record in disk

    TreeNode() :
        info()
    {
        leftMostPtr = 0;
        itemList.clear();
        rightMostPtr = 0;

        minItemNum = 0;
        maxItemNum = 0;
        address = -1;
        type = -1;
    }
    TreeNode(bool isLeaf, bool isRoot, int itemNum, int keySize, int type , Pointer lmptr =
                 0, Pointer rmptr = 0) :
        info(isLeaf, isRoot, itemNum)
        //used in creating a new node in memory
    {
        itemList.clear();

        rightMostPtr = rmptr;
        leftMostPtr = lmptr;

        maxItemNum = (4096 - sizeof(NodeInfo) - keySize)
                     / (keySize + sizeof(Pointer));
        minItemNum = (int) ceil(maxItemNum / 2.0);
        address = -1;
        this->type = type;
    }

    virtual ~TreeNode()
    {
        itemList.clear();
    }

    bool eraseItem(string key)

    {
        for (list<NodeItem>::iterator iter = itemList.begin();
                iter != itemList.end(); iter++)
        {
            if (key == iter->key)
            {
                info.itemNum--;
                itemList.erase(iter);
                return true;
            }
        }
        return false;
    }
    bool eraseItem(int address)
    {
        for (list<NodeItem>::iterator iter = itemList.begin();
                iter != itemList.end(); iter++)
        {
            if (address == iter->keyPtr)
            {
                info.itemNum--;
                itemList.erase(iter);
                return true;
            }
        }
        return false;
    }

    Pointer findRecordPos(string key)//get the address of the record including the key
    //only used in leaves
    //return -1 when the key is not found
    {
        for (list<NodeItem>::iterator iter = itemList.begin();
                iter != itemList.end(); iter++)
        {
            if (key == iter->key)
                return iter->keyPtr;
        }
        return -1;
    }

    Pointer findWay(const string key) //find the address may contain the key in the next level of the tree
    //only used in internal nodes
    {
        //cout<<"addr:"<<address<<"  front:"<<itemList.front().key<<endl;

        if (key < itemList.front().key)
            return leftMostPtr;

        list<NodeItem>::iterator iter = itemList.begin();
        for (; iter != itemList.end(); iter++)
        {
            if (key < iter->key)
                break;
        }
        iter--;

        return iter->keyPtr;
    }

    bool replaceKey(string oldKey, string newKey)	//repalce old key by new key
    {
        for (list<NodeItem>::iterator iter = itemList.begin();
                iter != itemList.end(); iter++)
        {
            if (iter->key == oldKey)
            {
                iter->key = newKey;
                return true;
            }
        }
        return false;
    }

    void insertAsLeaf(string key, Pointer keyPtr)
    {

        list<NodeItem>::iterator iter = itemList.begin();
        if (iter == itemList.end())	// leaf is empty
        {
            info.itemNum++;
            itemList.push_back(NodeItem(keyPtr, key));
        }
        else
        {
            for (; iter != itemList.end(); iter++)
            {
                if (key < iter->key)
                {
                    info.itemNum++;
                    itemList.insert(iter, NodeItem(keyPtr, key));
                    return;
                }
            }
            info.itemNum++;
            itemList.push_back(NodeItem(keyPtr, key));
            return;
        }
    }
};

#endif /* TREENODE_H_ */
