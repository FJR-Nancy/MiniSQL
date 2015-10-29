#include "BPlusTreeIndex.h"
#include <stdio.h>
extern BufferManager buffer;

BPlusTreeIndex::BPlusTreeIndex(Index &indexInfo) :
    index(indexInfo)
{
    for (int i = 0; i < 10; i++)
        path[i] = 0;
    secondaryBuffer.clear();
}

BPlusTreeIndex::~BPlusTreeIndex()
{
    for (vector<bufferBlock>::iterator iter = secondaryBuffer.begin();
            iter != secondaryBuffer.end(); iter++)
    {
        if (!iter->readOnly)
        {
            writeNode(iter->block);
        }
    }

    secondaryBuffer.clear();

}

//------------  create and remove index  -----------------

bool BPlusTreeIndex::createIndex(vector<string> &keys, vector<Pointer> &ptrs)
{
    try
    {
        ofstream ouf;		//create index file
        string fileName = index.indexName + ".idx";
        ouf.open(fileName.data());
        ouf.close();

        TreeNode *root = createNode(true, true, 0);
        index.rootAddress = root->address;

        vector<string>::iterator iter1 = keys.begin();
        vector<Pointer>::iterator iter2 = ptrs.begin();

        for (; iter1 != keys.end(); iter1++, iter2++)
        {
            insert(*iter1, *iter2);
        }

        return true;

    }
    catch (...)
    {
        return false;
    }
}

bool BPlusTreeIndex::removeIndex(string indexName)		//done test
{
    try
    {

        string cmdstr = /*"del " + */indexName + ".idx";
        const char *cmd = cmdstr.data();
        //system(cmd);
        if(std::remove(cmd)==EOF)
            throw 1;
    }
    catch (...)
    {
        return false;
    }
    return true;
}

//------------  find address by key  -----------------

Pointer BPlusTreeIndex::find(const string key) //return -1 when the key is not found
{	
	string tmpKey;
	format(key,tmpKey);    
	TreeNode *node = findBlockFor(tmpKey, true);
    //cout << "address:" << node->address << endl;
    return node->findRecordPos(tmpKey);
}

vector<Pointer> BPlusTreeIndex::find(Condition con)
{	
    vector<Pointer> ptrs;
    bool end = false;
    TreeNode *node;
    list<NodeItem>::iterator iter;
    string tmpKey;
    
	format(con.value,tmpKey);    
    switch (con.op)
    {
    case Lt:
        node = findBlockFor("", true);        
        while (!end)
        {
            for (iter = node->itemList.begin(); iter != node->itemList.end(); iter++)
            {
                if (iter->key == tmpKey)
                {	
                    end = true;
                    break;
                }
                ptrs.push_back(iter->keyPtr);
            }
            node = loadNode(node->rightMostPtr);
        }
        break;
    case Le:
        node = findBlockFor("", true);
        
        while (!end)
        {
            for (iter = node->itemList.begin(); iter != node->itemList.end(); iter++)
            {
                if (iter->key > tmpKey)
                {
                    end = true;
                    break;
                }
                ptrs.push_back(iter->keyPtr);
            }
            node = loadNode(node->rightMostPtr);
        }
        break;
    case Ge:
        node = findBlockFor(tmpKey, true);
        for (iter = node->itemList.begin(); iter != node->itemList.end(); iter++)
        {	 
            if (iter->key == tmpKey)
            {	
                for (; iter != node->itemList.end(); iter++)
                {
                    ptrs.push_back(iter->keyPtr);
                }
                break;
            }
        }
        while (node->rightMostPtr)
        {
            node = loadNode(node->rightMostPtr);
            for (iter = node->itemList.begin(); iter != node->itemList.end();
                    iter++)
                ptrs.push_back(iter->keyPtr);
        }
        break;
    case Gt:
        node = findBlockFor(tmpKey, true);
        for (iter = node->itemList.begin(); iter != node->itemList.end(); iter++)
        {
            if (iter->key > tmpKey)
            {
                for (; iter != node->itemList.end(); iter++)
                {
                    ptrs.push_back(iter->keyPtr);
                }
                break;
            }
        }

        while (node->rightMostPtr)
        {
            node = loadNode(node->rightMostPtr);
            for (iter = node->itemList.begin(); iter != node->itemList.end();
                    iter++)
                ptrs.push_back(iter->keyPtr);
        }
        break;
    case Eq:
        ptrs.push_back(find(tmpKey));
        if(ptrs.at(0)==-1) ptrs.clear();
        break;
    case Ne:
        Condition c;
        c.op = Lt;
        c.value = tmpKey;
        ptrs = find(c);
        c.op = Gt;
        vector<int> tmpPtrs = find(c);
        while(tmpPtrs.size()!=0)
        {
            ptrs.push_back(tmpPtrs.back());
            tmpPtrs.pop_back();
        }
        break;
    }
    //less than, less equal, greater than, greater equal, equal, not equal
    return ptrs;
}

//------------  insert key  -----------------

bool BPlusTreeIndex::insert(const string key, const Pointer keyPtr)
{
    try
    {
    	string tmpKey;
    	format(key,tmpKey);
        TreeNode *node = findBlockFor(tmpKey, false);

        if (node->findRecordPos(tmpKey) != -1) //check unique
            throw new exception();

        if (node->info.itemNum < node->maxItemNum) //the node has free space
            node->insertAsLeaf(tmpKey, keyPtr);

        else		//the node is full
        {
            node->insertAsLeaf(tmpKey, keyPtr);
            splitLeafNode(node);
        }
        return true;

    }
    catch (...)
    {
        return false;
    }
}

void BPlusTreeIndex::splitLeafNode(TreeNode *node)
{

    TreeNode *newNode = createNode(node->info.isLeaf, false,
                                   (int) floor(node->info.itemNum / 2.0), 0, node->rightMostPtr);

    node->info.itemNum = (int) ceil(node->info.itemNum / 2.0);
    node->rightMostPtr = newNode->address;

    NodeItem *item = NULL;
    for (int i = 0; i < newNode->info.itemNum; i++)
    {
        item = &(node->itemList.back());
        newNode->itemList.push_front(*item);
        node->itemList.pop_back();
    }


    insertInParent(node, newNode->itemList.front().key, newNode);

}

void BPlusTreeIndex::insertInParent(TreeNode *node, string key,
                                    TreeNode *newNode)
{	
    if (node->info.isRoot)
    {
        node->info.isRoot = false;

        TreeNode *newRoot = createNode(false, true, 1, node->address,
                                       0);

        index.rootAddress = newRoot->address;
        newRoot->itemList.push_back(NodeItem(newNode->address, key));
    }
    else
    {
        TreeNode *parent = loadNode(findParentAddr(node), false);

        if (parent->info.itemNum < parent->maxItemNum)
        {
            insertInInternal(parent, key, newNode);
        }

        else
        {
            insertInInternal(parent, key, newNode);
            splitInternalNode(parent);
        }
    }
}

void BPlusTreeIndex::insertInInternal(TreeNode *node, string key,
                                      TreeNode *newNode)
{
    Pointer keyPtr = newNode->address;

    for (list<NodeItem>::iterator iter = node->itemList.begin();
            iter != node->itemList.end(); iter++)
    {
        if (key < iter->key)
        {
            node->info.itemNum++;
            node->itemList.insert(iter, NodeItem(keyPtr, key));
            return;
        }
    }
    node->info.itemNum++;
    node->itemList.push_back(NodeItem(keyPtr, key));

}

void BPlusTreeIndex::splitInternalNode(TreeNode *node)
{
    TreeNode *newNode = createNode(node->info.isLeaf, false,
                                   (int) floor(ceil(node->info.itemNum / 2)), 0, 0);

    node->info.itemNum = (int) ceil(node->info.itemNum / 2) - 1;

    NodeItem *item = NULL;
    for (int i = 0; i < newNode->info.itemNum + 1; i++)
    {
        item = &(node->itemList.back());
        newNode->itemList.push_front(*item);
        node->itemList.pop_back();
    }
    item = &(newNode->itemList.front());
    newNode->leftMostPtr = item->keyPtr;

    insertInParent(node, item->key, newNode);

    newNode->itemList.pop_front();
}

//------------  remove key  -----------------

bool BPlusTreeIndex::remove(string key)
{
    TreeNode *node = findBlockFor(key, false);

    try
    {
    	string tmpKey;
    	format(key,tmpKey);
        deleteKey(node, tmpKey);
        return true;

    }
    catch (...)
    {
        return false;
    }
}
bool BPlusTreeIndex::removeSome(const vector<string> &keys)
{
    try
    {
        string key;
        for (int i = 0; i < keys.size(); i++)
        {
            key = keys.at(i);
            remove(key);

        }
        return true;
    }
    catch (...)
    {
        return false;
    }
}

void BPlusTreeIndex::deleteKey(TreeNode *node, string key)
{

    if (!node->eraseItem(key))
        throw new exception;
    if (node->info.itemNum < node->minItemNum)
    {
        if (node->info.isRoot)		//level - 1
        {
            if (node->info.itemNum < 2)
            {
                TreeNode *newRoot = loadNode(node->leftMostPtr, false);
                newRoot->info.isRoot = true;
                index.rootAddress = newRoot->address;
                removeNode(node);
            }
        }
        else
        {
            reconstruct(node);
        }
    }
}

bool BPlusTreeIndex::reconstruct(TreeNode *node)
{
    TreeNode *parent = loadNode(findParentAddr(node), false);
    TreeNode *nextSibling = NULL;
    TreeNode *previousSibling = NULL;
    string midKey;
    list<NodeItem>::iterator iter;
    int itemNumAfterMergeLeft;
    int itemNumAfterMergeRight;

    enum
    {
        mergeLEFT = -2,
        mergeRIGHT = 2,
        redistributionLEFT = -1,
        redistributionRIGHT = 1,
        END = 0
    } direction = END;

    if (parent->leftMostPtr == node->address) //first child
    {
        NodeItem item = parent->itemList.front();
        nextSibling = loadNode(item.keyPtr, false);
        midKey = item.key;
        if (node->info.isLeaf)
        {
            itemNumAfterMergeRight = nextSibling->info.itemNum
                                     + node->info.itemNum;
        }
        else
        {
            itemNumAfterMergeRight = nextSibling->info.itemNum
                                     + node->info.itemNum + 1;
        }
        if (itemNumAfterMergeRight <= node->maxItemNum)
            direction = mergeRIGHT;
        else if (nextSibling->info.itemNum - 1 >= node->minItemNum)
            direction = redistributionRIGHT;
    }
    else
    {
        bool rightMost = true;
        string key = node->itemList.front().key;
        list<NodeItem>::iterator iter = parent->itemList.begin();
        for (; iter != parent->itemList.end(); iter++)
        {
            if (key < iter->key)
            {
                rightMost = false;
                break;
            }
        }
        if (rightMost) //last child
        {
            iter--;
            iter--;
            NodeItem item = *iter;
            previousSibling = loadNode(item.keyPtr, false);
            midKey = node->itemList.back().key;
            if (node->info.isLeaf)
            {
                itemNumAfterMergeLeft = previousSibling->info.itemNum
                                        + node->info.itemNum;

            }
            else
            {
                itemNumAfterMergeLeft = previousSibling->info.itemNum
                                        + node->info.itemNum + 1;

            }
            if (itemNumAfterMergeLeft <= node->maxItemNum)
                direction = mergeLEFT;
            else if (previousSibling->info.itemNum - 1 >= node->minItemNum)
                direction = redistributionLEFT;
        }
        else //middle child
        {
            NodeItem nextItem = *iter;
            NodeItem item = *(--iter);
            NodeItem lastItem = *(--iter);
            nextSibling = loadNode(nextItem.keyPtr, false);
            previousSibling = loadNode(lastItem.keyPtr, false);
            if (node->info.isLeaf)
            {
                itemNumAfterMergeLeft = previousSibling->info.itemNum
                                        + node->info.itemNum;
                itemNumAfterMergeRight = nextSibling->info.itemNum
                                         + node->info.itemNum;
            }
            else
            {
                itemNumAfterMergeLeft = previousSibling->info.itemNum
                                        + node->info.itemNum + 1;
                itemNumAfterMergeRight = nextSibling->info.itemNum
                                         + node->info.itemNum + 1;
            }
            if (itemNumAfterMergeLeft <= node->maxItemNum)
            {
                midKey = item.key;
                direction = mergeLEFT;
            }

            else if (itemNumAfterMergeRight <= node->maxItemNum)
            {
                midKey = nextItem.key;
                direction = mergeRIGHT;
            }
            else if (previousSibling->info.itemNum - 1 >= node->minItemNum)
            {
                midKey = item.key;
                direction = redistributionLEFT;
            }
            else if (nextSibling->info.itemNum - 1 >= node->minItemNum)
            {
                midKey = nextItem.key;
                direction = redistributionRIGHT;
            }
        }
    }

    switch (direction)
    {
    case mergeLEFT:
        deleteKey(parent, midKey);
        merge(previousSibling, node, midKey);
        break;
    case mergeRIGHT:
        deleteKey(parent, midKey);
        merge(node, nextSibling, midKey);
        break;
    case redistributionLEFT:
        redistribution(node, previousSibling, direction, midKey, parent);
        break;
    case redistributionRIGHT:
        redistribution(node, nextSibling, direction, midKey, parent);
        break;
    case END:
        return false;
    }
    return true;

}

void BPlusTreeIndex::redistribution(TreeNode *node, TreeNode *sibling,
                                    int direction, string midKey, TreeNode *parent)
{
    if (node->info.isLeaf)
    {
        if (direction == -1)
        {
            NodeItem item = sibling->itemList.back();
            node->itemList.push_front(item);
            sibling->itemList.pop_back();
            parent->replaceKey(midKey, item.key);
        }
        else
        {
            NodeItem item = sibling->itemList.front();
            node->itemList.push_back(item);
            sibling->itemList.pop_front();
            item = sibling->itemList.front();
            parent->replaceKey(midKey, item.key);
        }
    }
    else
    {
        if (direction == -1)
        {
            NodeItem item = sibling->itemList.back();
            NodeItem itemToInsert(node->leftMostPtr, midKey);
            node->itemList.push_front(itemToInsert);
            node->leftMostPtr = item.keyPtr;
            sibling->itemList.pop_back();
            parent->replaceKey(midKey, item.key);

        }
        else
        {
            NodeItem item = sibling->itemList.front();
            NodeItem itemToInsert(sibling->leftMostPtr, midKey);
            node->itemList.push_back(itemToInsert);
            sibling->leftMostPtr = item.keyPtr;
            sibling->itemList.pop_front();
            parent->replaceKey(midKey, item.key);
        }
    }

}

void BPlusTreeIndex::merge(TreeNode *node, TreeNode *lost, string key)
{
    if (node->info.isLeaf)
    {
        node->info.itemNum += lost->info.itemNum;
        node->itemList.splice(node->itemList.end(), lost->itemList);
        node->rightMostPtr = lost->rightMostPtr;
        removeNode(lost);
    }
    else
    {
        node->info.itemNum += (lost->info.itemNum + 1);
        node->itemList.push_back(NodeItem(lost->leftMostPtr, key));
        node->itemList.splice(node->itemList.end(), lost->itemList);
        removeNode(lost);
    }
}

//------------  assistants  -----------------

TreeNode *BPlusTreeIndex::findBlockFor(const string key, bool readOnly)
{	
    int blockAddr = index.rootAddress;
    path[0] = blockAddr;
    path[9] = 1;

    TreeNode *node = loadNode(blockAddr, true);
    int i = 1;
    while (!node->info.isLeaf)
    {
	
		 blockAddr = node->findWay(key);  
        path[i] = blockAddr;
        path[9]++;
        node = loadNode(blockAddr, true);
        i++;
    }
    if (!readOnly)
        node = loadNode(blockAddr, readOnly);
    return node;
}

int BPlusTreeIndex::findParentAddr(TreeNode *node)
{
    int address = node->address;
    for (int i = 0; i < path[9]; i++)
        if (path[i] == address)
        {
            return path[i - 1];
        }
    return -1;
}

//------------  block read-write  -----------------

TreeNode *BPlusTreeIndex::createNode(bool isLeaf, bool isRoot, int itemNum,
                                     Pointer lmptr, Pointer rmptr)
{
    TreeNode *node = new TreeNode(isLeaf, isRoot, itemNum, index.attriLength,
                                  index.type, lmptr, rmptr);
    node->address = index.freeList;
    addTobuffer(node, false);
    //cout << "BLOCKNUM" << index.freeList << ' ' << isRoot << endl;
    if (index.freeList == index.blockNum)
    {
        //No free node in the file
        index.freeList++;
    }
    else
    {
        char *block = buffer.getBufferAddr(index.indexName + ".idx", index.freeList * 4096, true); /////////////////////read-only buffer
        memcpy(&index.freeList, block, 8);
        buffer.freePin(index.freeList);
    }
    index.blockNum++;
    return node;
}

TreeNode *BPlusTreeIndex::loadNode(Pointer blockAddr, bool readOnly)
{
    TreeNode *node = findInBuffer(blockAddr, readOnly);

    if (node != NULL)
        return node;

    char *block = buffer.getBufferAddr(index.indexName + ".idx", blockAddr * 4096, !readOnly); ///////////////read block from disk
    char *offset = block;
    bool isLeaf, isRoot;
    int itemNum;

    memcpy(&isLeaf, offset, 1);
    offset++;
    memcpy(&isRoot, offset, 1);
    offset++;
    memcpy(&itemNum, offset, 4);
    offset += 4;

    node = new TreeNode(isLeaf, isRoot, itemNum, index.attriLength, index.type);
    node->address = blockAddr;
    addTobuffer(node, readOnly);

    int keyPtr;
    string key;
    int intKey;
    float floatKey;
    char charKey[300];
    char tmp[100];

    if (isLeaf)
    {
        for (int i = 0; i < node->info.itemNum; i++)
        {
            memcpy(&keyPtr, offset, sizeof(Pointer));
            offset += sizeof(Pointer);
            switch (index.type)
            {
            case INT:
                memcpy(&intKey, offset, index.attriLength);
                sprintf(tmp, "%011d", intKey);
                key = tmp;
                break;
            case FLOAT:
                memcpy(&floatKey, offset, index.attriLength);
                sprintf(tmp, "%017.5f", floatKey);
                key = tmp;
                break;
            case CHAR:
                memcpy(&charKey, offset, index.attriLength);
                key = charKey;
                break;
            }

            offset += index.attriLength;

            node->itemList.push_back(NodeItem(keyPtr, key));;
        }

        memcpy(&(node->rightMostPtr), offset, sizeof(Pointer));

    }
    else
    {
        string key;
        int keyPtr;
        memcpy(&(node->leftMostPtr), offset, sizeof(Pointer));
        offset+=sizeof(Pointer);
        
        for (int i = 0; i < node->info.itemNum; i++)
        {
        	
            switch (index.type)
            {
            case INT:
                memcpy(&intKey, offset, index.attriLength);
                sprintf(tmp, "%011d", intKey);
                key = tmp;
                break;
            case FLOAT:
                memcpy(&floatKey, offset, index.attriLength);
                sprintf(tmp, "%017.5f", floatKey);
                key = tmp;
                break;
            case CHAR:
                memcpy(&charKey, offset, index.attriLength);
                key = charKey;
                break;
            }
            offset += index.attriLength;
            memcpy(&keyPtr, offset, sizeof(Pointer));
            offset += sizeof(Pointer);
            node->itemList.push_back(NodeItem(keyPtr, key));
        }
    }
    buffer.freePin(blockAddr);
    return node;
}

void BPlusTreeIndex::writeNode(TreeNode *node) //write back and remove it in memory
{

    char *block = buffer.getBufferAddr(index.indexName + ".idx", node->address * 4096, true); //////////////////write
    //char* block = new char[4096];
    block[0] = node->info.isLeaf;
    block[1] = node->info.isRoot;
    char *offset = block + 2;

    memcpy(offset, &(node->info.itemNum), 4);
    offset += 4;

    int i = 0;
    string j;
    int intKey;
    float floatKey;
    const char *charKey;

    if (node->info.isLeaf)
    {
        list<NodeItem>::iterator iter = node->itemList.begin();
        for (; iter != node->itemList.end(); iter++)
        {
            i = iter->keyPtr;
            j = iter->key;
            memcpy(offset, &(i), sizeof(Pointer));
            offset += sizeof(Pointer);

            switch (index.type)
            {
            case INT:
                intKey = atoi(j.c_str());
                memcpy(offset, &intKey, index.attriLength);
                offset += index.attriLength;
                break;
            case FLOAT:
                floatKey = (float) atof(j.c_str());
                memcpy(offset, &floatKey, index.attriLength);
                offset += index.attriLength;
                break;
            case CHAR:
                charKey = j.c_str();
                memcpy(offset, charKey, index.attriLength);
                offset += index.attriLength;
                break;
            }
        }

        memcpy(offset, &(node->rightMostPtr), sizeof(Pointer));
        offset += sizeof(Pointer);
    }
    else
    {
        memcpy(offset, &(node->leftMostPtr), sizeof(Pointer));
        offset += sizeof(Pointer);

        list<NodeItem>::iterator iter = node->itemList.begin();
        for (; iter != node->itemList.end(); iter++)
        {
            i = iter->keyPtr;
            j = iter->key;
            switch (index.type)
            {
            case INT:
                intKey = atoi(j.c_str());
                memcpy(offset, &intKey, index.attriLength);
                offset += index.attriLength;
                break;
            case FLOAT:
                floatKey = (float) atof(j.c_str());
                memcpy(offset, &floatKey, index.attriLength);
                offset += index.attriLength;
                break;
            case CHAR:
                charKey = j.c_str();
                memcpy(offset, charKey, index.attriLength);
                offset += index.attriLength;
                break;
            }

            memcpy(offset, &(i), sizeof(Pointer));
            offset += sizeof(Pointer);
        }
    }
    while (offset != block + 4096)
    {
        *offset = 0;
        offset++;
    }

    buffer.freePin(node->address);
    delete node;
}

void BPlusTreeIndex::removeNode(TreeNode *const node)
{
    int a = index.freeList;
    index.freeList = node->address;
    char *block = buffer.getBufferAddr(index.indexName + ".idx", node->address * 4096, true); //////////////////write
    //char* block = new char[4096];
    memset(block, 0, 4096);
    memcpy(block, &a, 4);
    buffer.freePin(node->address);
    removeFromBuffer(node->address);
    delete node;
    index.blockNum--;
}

//------------  secondary buffer  -----------------

void BPlusTreeIndex::addTobuffer(TreeNode *node, bool readOnly)
{
    if (secondaryBuffer.size() > 10) //bounded buffer of size 10
    {
        writeNode(secondaryBuffer.at(0).block);
        secondaryBuffer.erase(secondaryBuffer.begin());
    }

    secondaryBuffer.push_back(bufferBlock(node, node->address, readOnly));
}

TreeNode *BPlusTreeIndex::findInBuffer(int address, bool readOnly) //LRU
{
    for (vector<bufferBlock>::iterator it = secondaryBuffer.begin(); it != secondaryBuffer.end();
            it++)
    {
        if (it->blockAddress == address)
        {
            TreeNode *node = (secondaryBuffer.at(it - secondaryBuffer.begin())).block;
            if (!readOnly)
                secondaryBuffer.at(it - secondaryBuffer.begin()).readOnly = false;

            bufferBlock bf = secondaryBuffer.at(it - secondaryBuffer.begin()); //LRU
            secondaryBuffer.erase(it);
            secondaryBuffer.push_back(bf);

            return node;
        }
    }
    return NULL;
}

void BPlusTreeIndex::removeFromBuffer(int address)
{
    for (vector<bufferBlock>::iterator it = secondaryBuffer.begin(); it != secondaryBuffer.end();
            it++)
    {
        if (it->blockAddress == address)
        {
            secondaryBuffer.erase(it);
            break;
        }
    }
}
void BPlusTreeIndex::format(const string str,string& key){
	char tmp[100];
	int intString;
	float floatString;
	switch(index.type){
	case INT:
		intString=atoi(str.c_str());	
		sprintf(tmp, "%011d", intString);
        key=tmp;
		break;
	case FLOAT:
		intString=atoi(str.c_str());	
		sprintf(tmp, "%017.5f", floatString);
        key=tmp;
        break;
    case CHAR:
        key=str;
        break;
	}

}
void BPlusTreeIndex::clear(){
    FILE* fp;
    char tmp[4096];
    memset(tmp,0,4096);
    tmp[0]=1;
    tmp[1]=1;
    string name=index.indexName+".idx";
    fp=fopen(name.c_str(),"w");
    fwrite(&tmp,4096,1,fp);
    fclose(fp);
}
