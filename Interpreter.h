#ifndef _INTERPRETER_H
#define _INTERPRETER_H

#include "MiniSQL.h"
#include <iostream>
#include <fstream>


class Interpreter
{
public:
    //解析树,所有的命令均可以反映到这样一棵树上
    int m_operation;		//要执行的操作或错误代码,用宏表示
    string m_tabname;		//要操作的表格名
    string m_indname;		//要操作的索引名
    string m_colname;
    string m_filename;
    vector<Attribute> column;
    vector<Condition> condition;			//要比较的where字句的链表
    Row row;		//要插入的值链表
    Table getTableInfo;
    Index getIndexInfo;
    int PrimaryKeyPosition;
    int UniquePostion;
    vector<Condition> UniqueCondition;

    Interpreter()
    {
        m_operation = UNKNOWN;
        m_tabname = "";
        m_indname = "";
        m_colname = "";
        PrimaryKeyPosition = -1;
        UniquePostion = -1;
    }
    ~Interpreter() {}

    void Parse(char *command);

    void InitColumn();
    void InitCondition();
    void InitValue();
    void InitTable()
    {
        getTableInfo.name = "";
        getTableInfo.attriNum = getTableInfo.blockNum = getTableInfo.totalLength = 0;
        getTableInfo.attributes.clear();
    }
    void InitIndex()
    {
        getIndexInfo.blockNum = getIndexInfo.attriNum = -1;
        getIndexInfo.indexName = "";
    }

    bool GetWord(string &src, string &des);

protected:

    void MakeInitilate()
    {
        m_operation = UNKNOWN;
        m_tabname = "";
        m_indname = "";
        InitColumn();
        InitCondition();
        InitValue();
        InitTable();
        InitIndex();
    }

    bool GetStr(string &src, string &des);

    short int IsInt(const char *);
    short int IsFloat(char *input);
};


#endif