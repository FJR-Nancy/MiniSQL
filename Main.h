#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <vector>
using namespace std;

#define INT			1
#define FLOAT		2
#define CHAR		3
#define INTLEN		4
#define FLOATLEN	4

#define ISPRIMARYKEY	1
#define NOTPRIMARYKEY	0
#define ISUNIQUE		1
#define NOTUNIQUE		0

//定义操作类型
#define SELECT		0
#define CRETAB		1
#define	CREIND		2
#define	DRPTAB		3
#define DRPIND		4
#define DELETE		5
#define INSERT		6
#define QUIT		7
#define EXEFILE		8

//定义错误类型
#define EMPTY		11
#define UNKNOWN		12
#define SELERR		13
#define	CREINDERR	14
#define CRETABERR	15
#define DELETEERR	16
#define INSERTERR	17
#define DRPTABERR	18
#define DRPINDERR	19
#define EXEFILERR	20
#define NOPRIKEY    21
#define VOIDPRI		22
#define VOIDUNI		23
#define CHARBOUD    24
#define TABLEERROR  25
#define COLUMNERROR 26
#define INSERTNUMBERERROR		27
#define SELECT_WHERE_CAULSE		28
#define SELECT_NOWHERE_CAULSE	29
#define TABLEEXISTED			30
#define INDEXERROR				31
#define INDEXEROR				32

#define COMLEN		400
#define INPUTLEN	200
#define WORDLEN		100
#define VALLEN		300
#define NAMELEN		100

#define SUCCESS 33//
#define BLOCK 4096//

class Attribute
{
public:
    string name;
    int type;
    int length;
    bool isPrimaryKey;
    bool isUnique;
    Attribute()
    {
        isPrimaryKey = false;
        isUnique = false;
        name="";
        type=1;
        length=0;
    }
    Attribute(string _name, int _type, int _length, bool _isPrimaryKey = false, bool _isUnique = false) //
        : name(_name), type(_type), length(_length), isPrimaryKey(_isPrimaryKey), isUnique(_isUnique) {}
};

class Table
{
public:
    string name;   //all the datas is store in file name.table
    int blockNum;	//number of block the datas of the table occupied in the file name.table
    int recordNum;	//number of record in name.table
    int attriNum;	//the number of attributes in the tables
    int totalLength;	//total length of one record, should be equal to sum(attributes[i].length)
    int primaryKey;	//which attribute is the primarykey
    int emptyNum;
    vector<int> recordList;//the point of record list
    vector<int> emptyList;//the point of empty place list
    vector<Attribute> attributes;
    Table()
    {
        name = "";
        blockNum = recordNum = attriNum = totalLength = primaryKey = emptyNum = 0;
        recordList.clear();
        emptyList.clear();
        attributes.clear();
    }
    Table(string name, vector<Attribute> attributes)
    {
        blockNum = recordNum  = totalLength = primaryKey = emptyNum = 0;
        this->name = name;
        this->attriNum = attriNum;
        this->attributes = attributes;
        for(int i = 0; i < attributes.size(); i++)
        {
            totalLength += attributes.at(i).length;
            attriNum += 1;
        }
        recordList.clear();
        emptyList.clear();
        attributes.clear();
    }

};

class Index
{
public:
    string indexName;	//all the datas is store in file index_name.index
    string tableName;	//the name of the table on which the index is create
    int attriNum;			//on which column the index is created
    int blockNum;		//number of block the datas of the index occupied in the file index_name.table
    int freeList;
    int attriLength;
    int rootAddress;
    int type;//the type of the attribute
    Index()
    {
        indexName = "";
        tableName = "";
        attriNum = blockNum = freeList = attriLength = rootAddress = type = 0;
    }
};

class Row
{
public:
    vector<string> columns;
};

class Data//这样就把Data搞成了一个二维数组
{
public:
    vector<Row> rows;
};

//less than, less equal, greater than, greater equal, equal, not equal
enum Comparison {Lt, Le, Gt, Ge, Eq, Ne};
class Condition
{
public:
    Comparison op;
    int columnNum;
    string value;
    Condition(Comparison a, int b, string c): op(a), columnNum(b), value(c) {}
    Condition()
    {
        columnNum = 0;
        value = "";

    }
};


#endif // MAIN_H
