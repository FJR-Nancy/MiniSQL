//========================================================================================================
// Name		: Record Manager模块的 public API
// Author	: 范佳柔
// Version	: final 1.0
// Date		: 2013-11-01
// Note		: 包含这个头文件就可以了
// name		: recordmanager.h
//========================================================================================================

#ifndef RECORDMANAGER_H
#define RECORDMANAGER_H

#include "MiniSQL.h"
#include <vector>
#include "BufferManager.h"
#include "MiniSQL.h"
#include <stdio.h>
//#include <fstream>


using namespace std;

class RecordManager
{
public:
    RecordManager();

    int createTable(Table table);
    int dropTable(Table table);

    Data select(Table table);
    Data select_without_index(Table table, vector<Condition> conditions);
    Data select_with_index(Table table, vector<Condition> conditions, vector<int> addresses);

    int insertValue(Table &table, Row row);

    int deleteValue(Table &table);
    vector<int> deleteValue_without_index(Table &table, vector<Condition> conditions);
    vector<int> deleteValue_with_index(Table &table, vector<Condition> conditions, vector<int> addresses);

    vector<string> select(Table table, int columnNum);

    vector<string> select(Table table, int columnNum, vector<int> addresses);
};

#endif // RECORDMANAGER_H
