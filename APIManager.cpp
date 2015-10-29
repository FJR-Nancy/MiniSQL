#include "APIManager.h"
#include "BufferManager.h"
#include "CatalogManager.h"
#include "RecordManager.h"
#include "BPlusTreeIndex.h"


RecordManager record;
Catalog_manager catalogManager;
BufferManager buffer;

bool APIManager::ExistTable(string tableName)
{
    return catalogManager.exist_table(tableName);
}

Table APIManager::GetTableInformation(string tableName)
{
    return catalogManager.get_table_information(tableName);
}

Index APIManager::GetIndexInformation(string indexName)
{
    return catalogManager.get_index_information(indexName);
}

int APIManager::GetColumnNumber(Table &table, string columnName)  //返回是表的第几个属性
{
    return catalogManager.get_column_number(table.name, columnName);
}

bool APIManager::ExistIndex(string tableName, int testInt)
{
    return catalogManager.exist_index(tableName, testInt);
}

bool APIManager::ExistIndex(string indexName)
{
    return catalogManager.exist_index(indexName);
}

int APIManager::GetColumnAmount(Table &table)  //返回表的属性个数
{
    return catalogManager.get_column_amount(table.name);
}

void APIManager::createTable(Table table)
{
    Index indext;
    indext = catalogManager.create_table(table);
    record.createTable(table);

    BPlusTreeIndex tempIndex(indext);
    vector<int> ptrs;// = table.recordList;
    vector<string> keys;// = record.select(table, indext.attriNum);
    tempIndex.createIndex(keys, ptrs);
    catalogManager.update_index(indext);
    //cout << "Table " << table.name << " has been created successfully!" << endl;
}
//Table信息传给catalogManager  调用ctlg的建表函数（插入表的信息）

void APIManager::dropTable(Table table)
{
    vector<Index> indexes = catalogManager.indexInTable(table.name);//等小顾写函数
    for(unsigned int i = 0; i < indexes.size(); i++)
    {
        BPlusTreeIndex::removeIndex(indexes.at(i).indexName);
    }
    catalogManager.drop_table(table.name);
    record.dropTable(table);
    buffer.clear();

    //cout << "Table " << table.name << " has been dropped successfully!" << endl;
}
//告诉ctlg删除表的信息  rcd把表的内容清除  index把这个表上所有的索引清掉

void APIManager::insertValue(Table &tempTable, Row row)
{
    Table table = catalogManager.get_table_information(tempTable.name);
    int recordInsertedNum = record.insertValue(table, row);
    int addr = table.recordList.back();
    vector<Index> indexes = catalogManager.indexInTable(table.name);//等小顾写函数
    BPlusTreeIndex *tempIndex;
    for(unsigned int i = 0; i < indexes.size(); i++)
    {

        tempIndex = new BPlusTreeIndex(indexes.at(i));
        tempIndex->insert(row.columns.at(indexes.at(i).attriNum), addr);
        catalogManager.update_index(indexes.at(i));
        delete tempIndex;

    }

    catalogManager.update_table(table);
}
//rcd函数插入记录(ctlg里面改条数)  调ctlg，里面哪一列有索引，有索引的列要调用index插入索引里面

Data APIManager::select(Table table,  vector<Condition> condition)
{
    for(unsigned int i = 0; i < condition.size(); i++)
    {
        if(catalogManager.attr_index_exist(table.name, condition.at(i).columnNum))
        {
            Index  idx = catalogManager.find_index(table.name, condition.at(i).columnNum);
            BPlusTreeIndex tempIndex(idx);
            vector<int> addresses = tempIndex.find(condition.at(i));
            condition.erase(condition.begin() + i);
            return record.select_with_index(table, condition, addresses);

        }
        //else
            //cout << "There is no table named " << table.name << "!" << endl;
    }
    return record.select_without_index(table, condition);
    //return record.select(table, );
}
//ctlg里面查看有没有index  有就调index里的函数查找，找到的地址给rcd，让rcd返回记录（==）
//（<>）的话就返回一大坨地址给rcd，返回记录
//没有index的话，调用rcd的顺序查找，返回记录（而非地址）

Data APIManager::select(Table table)
{
    //int size = record.select(table).rows.size();
    //if(size == 0)
        //cout << "There is no table named " << table.name << "!" << endl;
    return record.select(table);
}
//无条件：直接调用rcd，返回表的数据

int APIManager::deleteValue(Table table)
{
    vector<Index> indexes = catalogManager.indexInTable(table.name);//等小顾写函数
    for(int i = 0; i < indexes.size(); i++)
    {
        BPlusTreeIndex tmpIndex(indexes.at(i));
        tmpIndex.clear();
        catalogManager.update_index(indexes.at(i));
    }

    int num = record.deleteValue(table);
    catalogManager.update_table(table);
    

    //cout << num << " record(s) deleted successfully!" << endl;
	return num;
}

int APIManager::deleteValue(Table table,  vector<Condition> condition)
{
    vector<int> address;
    for(unsigned int i = 0; i < condition.size(); i++)
    {
        if(catalogManager.attr_index_exist(table.name, condition.at(i).columnNum))
        {
            Index idx = catalogManager.find_index(table.name, condition.at(i).columnNum);
            BPlusTreeIndex tempIndex(idx);

            vector<int> addresses = tempIndex.find(condition.at(i));

            condition.erase(condition.begin() + i);
            address = record.deleteValue_with_index(table, condition, addresses);

            vector<Index> indexes = catalogManager.indexInTable(table.name);
            for(int j = 0; j < indexes.size(); j++)
            {
                BPlusTreeIndex tmpIndex(indexes.at(j));
                vector<string> indexValue = record.select(table, indexes.at(j).attriNum, address);
                tmpIndex.removeSome(indexValue);//record要改  不能传地址
                catalogManager.update_index(indexes.at(j));
            }
            catalogManager.update_table(table);
            return address.size();
        }
    }

    address = record.deleteValue_without_index(table, condition);
    vector<Index> indexes = catalogManager.indexInTable(table.name);
    for(int j = 0; j < indexes.size(); j++)
    {
        vector<string> indexValue = record.select(table, indexes.at(j).attriNum, address);
        BPlusTreeIndex tempIndex(indexes.at(j));
        tempIndex.removeSome(indexValue);//record要改  不能传地址
        catalogManager.update_index(indexes.at(j));
    }
    catalogManager.update_table(table);
    //cout << address.size() << " record(s) deleted successfully!" << endl;
    return address.size();

}
//insert反过来：rcd删除  ctlg该条数  有索引的话删index

void APIManager::createIndex(Index index)
{
    Table table = catalogManager.get_table_information(index.tableName);
    BPlusTreeIndex tempIndex(index);
    vector<int> ptrs = table.recordList;
    vector<string> keys = record.select(table, index.attriNum);
    tempIndex.createIndex(keys, ptrs);

    catalogManager.create_index(index);

    //cout << "Index " << index.indexName << " has been created successfully!" << endl;
}
//循环调用rcd，返回要被索引的列的所有键值，存到vector里面
//直接调index的createIndex
//index插入catlog

void APIManager::dropIndex(Index index)
{
    BPlusTreeIndex::removeIndex(index.indexName);
    catalogManager.drop_index(index.indexName, index.tableName);

    //cout << "Index " << index.indexName << " has been dropped successfully!" << endl;
}
//告诉CTG删除index信息
//调index
