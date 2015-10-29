#include "CatalogManager.h"


Catalog_manager::Catalog_manager()
{
    tabnum = 0;
    indnum = 0;
    fstream  tablein("d:/table.g", ios::in);
    tablein >> tabnum;
    for ( int i = 0; i < tabnum ; i++)
    {
        Table table;
        tablein >> table.name;
        tablein >> table.blockNum;
        tablein >> table.recordNum;
        tablein >> table.attriNum;
        tablein >> table.totalLength;
        tablein >> table.primaryKey;
        tablein >> table.emptyNum;
        int j;
        int a = 0;
        for ( j = 0; j < table.recordNum; j++)
        {
            tablein >> a;
            table.recordList.push_back(a);
        }
        for (j = 0; j < table.emptyNum; j++)
        {
            tablein >> a;
            table.emptyList.push_back(a);
        }
        for (j = 0; j < table.attriNum; j++)
        {
            Attribute attribute;
            tablein >> attribute.name;
            tablein >> attribute.type;
            tablein >> attribute.length;
            tablein >> attribute.isPrimaryKey;
            tablein >> attribute.isUnique;
            table.attributes.push_back(attribute);
        }
        ttable.push_back(table);
    }
    tablein.close();
    fstream  indexin("d:/index.g", ios::in);
    indexin >> indnum;
    for (int i = 0; i < indnum; i++)
    {
        Index index;
        indexin >> index.indexName;
        indexin >> index.tableName;
        indexin >> index.attriNum;
        indexin >> index.blockNum;
        indexin >> index.freeList;
        indexin >> index.attriLength;
        indexin >> index.rootAddress;
        indexin >> index.type;
        iindex.push_back(index);
    }
    indexin.close();
}
//将table和index信息从容器中写入到文
Catalog_manager:: ~Catalog_manager()
{
    fstream  tableout( "d:/table.g", ios::out);
    tableout << tabnum << endl;
    int i, j;
    for (i = 0; i < tabnum; i++)
    {
        tableout << ttable[i].name << endl;
        tableout << ttable[i].blockNum << " " << ttable[i].recordNum << " " << ttable[i].attriNum << endl;
        tableout << ttable[i].totalLength << " ";
        tableout << ttable[i].primaryKey << " ";
        tableout << ttable[i].emptyNum << endl;
        for (j = 0; j < ttable[i].recordNum; j++)
        {
            tableout << ttable[i].recordList[j] << " ";
        }
        tableout << endl;
        for(j = 0; j < ttable[i].emptyNum; j++)
        {
            tableout << ttable[i].emptyList[j] << " ";
        }
        tableout << endl;
        for (j = 0; j < ttable[i].attriNum; j++)
        {
            tableout << ttable[i].attributes[j].name << " ";
            tableout << ttable[i].attributes[j].type << " ";
            tableout << ttable[i].attributes[j].length << " ";
            tableout << ttable[i].attributes[j].isPrimaryKey << " ";
            tableout << ttable[i].attributes[j].isUnique << endl;
        }
    }
    tableout.close();
    fstream  indexout("d:/index.g", ios::out);
    indexout << indnum << endl;
    for (i = 0; i < indnum; i++)
    {
        indexout << iindex[i].indexName << " ";
        indexout << iindex[i].tableName << " ";
        indexout << iindex[i].attriNum << " ";
        indexout << iindex[i].blockNum << " ";
        indexout << iindex[i].freeList << " ";
        indexout << iindex[i].attriLength << " ";
        indexout << iindex[i].rootAddress << " ";
        indexout << iindex[i].type << endl;
    }

    indexout.close();
}
//创建表
Index Catalog_manager::create_table(Table &table)
{
    int a = 0;
    for (int i = 0; i < table.attriNum; i++)
    {
        a = a + table.attributes[i].length;
    }
    if (table.totalLength != a)
    {
        table.totalLength = a;
    }
    ttable.push_back(table);
    tabnum++;
    Index indext;
    indext.indexName = table.name + "Index";
    indext.tableName = table.name;
    indext.attriNum = table.primaryKey;
    indext.attriLength = table.attributes[table.primaryKey].length;
    indext.type = table.attributes[table.primaryKey].type;
    iindex.push_back(indext);
    indnum++;
    return indext;
}

vector<Index> Catalog_manager::indexInTable(string tname)
{
    vector<Index> *a = new vector<Index>;
    for (int i = 0; i < indnum; i++)
    {
        if (iindex[i].tableName == tname)
        {
            a->push_back(iindex[i]);
        }
    }
    return *a;
}
//供API调用储存新的index
void  Catalog_manager:: create_index(Index &index)
{
    for (int i = 0; i < tabnum; i++)
    {
        if (ttable[i].name == index.tableName)
        {
            index.attriLength = ttable[i].attributes[index.attriNum].length;
            index.type = ttable[i].attributes[index.attriNum].type;
        }
    }
    iindex.push_back(index);
    indnum++;
}
//供API调用删除表信息
void  Catalog_manager::drop_table(string tablename)
{
    int i;
    for(i = 0; i < tabnum; i++)
    {
        if (ttable[i].name == tablename)
        {
            ttable.erase (ttable.begin() + i);
            tabnum--;
        }
    }
    for(i = 0; i < indnum; i++)
    {
        if (iindex[i].tableName == tablename)
        {
            iindex.erase(iindex.begin() + i);
            indnum--;
        }
    }
}
//供API调用删除index信息
void  Catalog_manager::drop_index(string indexname, string tablename)
{
    int i;
    for(i = 0; i < indnum; i++)
    {
        if ((iindex[i].indexName == indexname) && (iindex[i].tableName == tablename))
        {
            iindex.erase(iindex.begin() + i);
            indnum--;
        }
    }
}
//供API调用更新table信息
void  Catalog_manager::update_table(Table &table)
{
    int a = 0;
    int i, j;
    for (i = 0; i < table.attriNum; i++)
    {
        a += table.attributes[i].length;
    }
    table.totalLength = a;
    for(i = 0; i < tabnum; i++)
    {
        if (ttable[i].name == table.name)
        {
            for (j = 0; j < indnum; j++)
            {
                if (iindex[j].tableName == table.name)
                {
                    a = iindex[j].attriNum;
                    for(int k = 0; k < table.attriNum; k++)
                    {
                        if (ttable[i].attributes[a].name == table.attributes[k].name)
                            iindex[j].attriNum = k;
                    }
                }
            }
            ttable.erase (ttable.begin() + i);
            ttable.push_back(table);
        }
    }
}
//供API调用更新index信息
void  Catalog_manager::update_index(Index &index)
{
    for(int i = 0; i < indnum; i++)
    {
        if (iindex[i].indexName == index.indexName)
        {
            iindex[i].tableName = index.tableName;
            iindex[i].attriNum = index.attriNum;
            iindex[i].blockNum = index.blockNum;
            iindex[i].freeList = index.freeList;
            iindex[i].attriLength = index.attriLength;
            iindex[i].rootAddress = index.rootAddress;
            iindex[i].type = index.type;
        }
    }
}

Table  Catalog_manager::get_index_table(string iname)
{
    Table f;
    for(int i = 0; i < indnum; i++)
    {
        if (iindex[i].indexName == iname)
        {
            for(int j = 0; j < tabnum; j++)
            {
                if(ttable[j].name == iindex[i].tableName)
                    return ttable[j];
            }
        }

    }
    return f;

}
//传出特定table名的table结构体，供调用者获取该table信息
Table  Catalog_manager::get_table_information(string tname)
{
    Table t;
    for (int i = 0; i < tabnum; i++)
    {
        if(ttable[i].name == tname)
            return ttable[i];
    }

    return t;
}
//传出特定表特定列的属性上的index
Index  Catalog_manager::get_index_information(string iname)
{
    Index c;
    int i;
    for (i = 0; i < indnum; i++)
    {
        if (iindex[i].indexName == iname)
            return iindex[i];
    }
    return c;
}

//不是很清楚这个函数的参数意思，暂时理解为表的某列号上是否存在Index的判断，存在输出true
bool  Catalog_manager::exist_index(string tname, int test)
{
    for (int i = 0; i < indnum; i++)
    {
        if ((iindex[i].tableName == tname) && (iindex[i].attriNum == test))
            return true;
    }
    return false;
}
//输入表名，若表名存在输出true.合法则输出false
bool  Catalog_manager::exist_table(string tname)
{
    for(int i = 0; i < tabnum; i++)
    {
        if (ttable[i].name == tname)
            return true;
    }
    return false;
}

//输入索引名,根据是否存在输出是否合法
bool  Catalog_manager::exist_index (string iname)
{
    for (int i = 0; i < indnum; i++)
    {
        if (iindex[i].indexName == iname)
            return true;
    }
    return false;
}
//输入表名列号，输出改属性上是否已经存在索引，存在输出true
bool  Catalog_manager::attr_index_exist(string tname, int cnum)
{
    for (int i = 0; i < indnum; i++)
    {
        if((iindex[i].tableName == tname) && (iindex[i].attriNum == cnum))
            return true;
    }
    return false;
}
Index  Catalog_manager::find_index(string tname, int cnum)
{
    Index b;
    for (int i = 0; i < indnum; i++)
    {
        if((iindex[i].tableName == tname) && (iindex[i].attriNum == cnum))
            return iindex[i];
    }
    return b;
}
//返回表的第几个属性
int  Catalog_manager::get_column_number(string tname, string columnName)
{
    for (int j = 0; j < tabnum; j++)
    {
        if (ttable[j].name == tname)
        {
            for (int i = 0; i < ttable[j].attriNum; i++)
            {
                if (ttable[j].attributes[i].name == columnName)
                {
                    return i;
                }
            }
        }
    }
    return -1;
}
int  Catalog_manager::get_column_amount(string tname)
{
    for (int i = 0; i < tabnum; i++)
    {
        if (ttable[i].name == tname)
        {
            return ttable[i].attriNum;
        }

    }
    return -1;
}
