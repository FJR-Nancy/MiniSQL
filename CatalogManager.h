#ifndef CATALOG_MANAGER_H
#define CATALOG_MANAGER_H

#include "MiniSQL.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

class Catalog_manager
{

private:
    vector<Table> ttable;
    vector<Index> iindex;
    int tabnum;
    int indnum;
public:

    Catalog_manager();
    ~Catalog_manager();
    Index create_table(Table &table);
    vector<Index> indexInTable(string tname);
    void create_index(Index &index);
    void drop_table(string tablename);
    void drop_index(string indexname, string tablename);
    void update_table(Table &table);
    void update_index(Index &index);
    Table get_index_table(string iname);
    Table get_table_information(string tname);
    Index get_index_information(string iname);
    //不是很清楚这个函数的参数意思，暂时理解为表的某列号上是否存在Index的判断，存在输出false
    bool exist_index(string tname, int test);
    //输入表名，若表名存在输出false.合法则输出true
    bool exist_table(string tname);
    //输入索引名,根据是否存在输出是否合法
    bool exist_index (string iname);
    //输入表名列号，输出改属性上是否已经存在索引，存在输出false
    bool attr_index_exist(string tname, int cnum);
    Index find_index(string tname, int cnum);
    //返回表的第几个属性
    int get_column_number(string tname, string columnName);
    int get_column_amount(string tname);
};
#endif // CATALOG_MANAGER_H