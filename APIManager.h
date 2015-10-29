#ifndef APIMANAGER_H_
#define APIMANAGER_H_
#include "MiniSQL.h"


#include <iostream>

using namespace std;

////////////void welcome();


class APIManager
{
public:
    bool ExistTable(string tableName);
    Table GetTableInformation(string tableName);
    Index GetIndexInformation(string indexName);
    int GetColumnNumber(Table &table, string columnName);  //返回是表的第几个属性
    bool ExistIndex(string tableName, int testInt);
    bool ExistIndex(string indexName);
    int GetColumnAmount(Table &table);  //返回表的属性个数

    void createTable(Table table);
    void dropTable(Table table);
    void insertValue(Table &table, Row row);
    Data select(Table table, vector<Condition> condition);
    Data select(Table table);
    int  deleteValue(Table table, vector<Condition> condition); //返回删除了多少条记录
    int  deleteValue(Table table);
    void createIndex(Index index);
    void dropIndex(Index index);

};


#endif