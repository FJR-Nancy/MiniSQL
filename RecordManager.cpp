#include <stdlib.h>
#include "recordmanager.h"

using namespace std;

extern BufferManager buffer;

RecordManager::RecordManager() {}

bool compare(Table table, Row row, vector<Condition> conditions);
Row connectRow(Table table, string filename, int address);
//Row connectRow(Table table,FILE *fp,int address);

int RecordManager::createTable(Table table)
{
    string filename = table.name + ".table";

    fstream  fout(filename.c_str(), ios::out);
    if(fout)
    {
        fout.close();
        return SUCCESS;
    }
    else
        return CRETABERR;
}

int RecordManager::dropTable(Table table)
{
    string filename = table.name + ".table";

    if( remove(filename.c_str()) != 0 )
        return DRPTABERR;
    else
        return SUCCESS;
}

int RecordManager::insertValue(Table &table, Row row)
{
    int i = 0;
    int address;
    string filename = table.name + ".table";
    //    FILE *fp; //
    char *attribute_char;
    int attribute_int;
    float attribute_float;

    //    fp=fopen(filename.c_str(),"rb+");//

    if(table.emptyNum == 0) //if there is not a block or the blocks are all full,create a new block
    {
        table.emptyList.push_back(BLOCK * table.blockNum + ((BLOCK / table.totalLength) - 1)*table.totalLength);
        for(int j = 0; j < (BLOCK / table.totalLength) - 1; j++)
        {
            table.emptyList.push_back(table.emptyList.back() - table.totalLength);
        }
        table.blockNum++;
        table.emptyNum += BLOCK / table.totalLength;
    }

    table.recordNum++;
    address = table.emptyList.back();
    table.emptyList.pop_back();
    table.emptyNum--;
    table.recordList.push_back(address);

    //    fseek(fp,address,SEEK_SET);//the file stream point locate at current positin

    for(i = 0; i < row.columns.size(); i++)
    {
        if(table.attributes.at(i).type == CHAR)
        {
            attribute_char = (char *)row.columns.at(i).c_str();
            buffer.writeBufferData(filename, address, attribute_char, 1, table.attributes.at(i).length);
            //			fwrite(attribute_char,1,table.attributes.at(i).length, fp);
        }
        else if(table.attributes.at(i).type == INT)
        {
            attribute_int = atoi(row.columns.at(i).c_str());
            buffer.writeBufferData(filename, address, &attribute_int, 4, 1);
            //			fwrite(&attribute_int,4,1, fp);
        }
        else
        {
            attribute_float = atof(row.columns.at(i).c_str());
            buffer.writeBufferData(filename, address, &attribute_float, 4, 1);
            //			fwrite(&attribute_float,4,1, fp);//
        }
        address += table.attributes.at(i).length;
    }
    //    fclose(fp);//

    return SUCCESS;
}

int RecordManager::deleteValue(Table &table)
{
    for(int i = 0; i < table.recordNum; i++)
    {
        table.emptyList.push_back(table.recordList.back());
        table.emptyNum++;
        table.recordList.pop_back();
    }
    int deleteNum = table.recordNum;
    table.recordNum = 0;
    //   table.blockNum=0;

    return deleteNum;
}

vector<int> RecordManager::deleteValue_without_index(Table &table, vector<Condition> conditions)
{
    string filename = table.name + ".table";
    //    FILE *fp; //
    int address;
    Row tempRow;
    vector<int> addresses;
    //    int deleteNum=0;

    //    fp=fopen(filename.c_str(),"rb");//

    for(int i = 0; i < table.recordNum; i++)
    {
        address = table.recordList.at(i);
        //       tempRow=connectRow(table,fp,address);
        tempRow = connectRow(table, filename, address);
        if(compare(table, tempRow, conditions)) //to judge whether the row meets conditions
        {
            table.emptyList.push_back(table.recordList.at(i));
            table.emptyNum++;
            addresses.push_back(table.recordList.at(i));
            table.recordList.erase(table.recordList.begin() + i);
            table.recordNum--;
            //            deleteNum++;
            i--;
            //            table.blockNum=0;
        }
    }
    return addresses;
    //    return deleteNum;
}

vector<int> RecordManager::deleteValue_with_index(Table &table, vector<Condition> conditions, vector<int> addresses)
{
    string filename = table.name + ".table";
    //    FILE *fp; //
    int address;
    Row tempRow;
    vector<int> return_addresses;
    //    int deleteNum=0;

    //    fp=fopen(filename.c_str(),"rb");//

    if(conditions.size()==0){
        for(int i = 0; i < addresses.size(); i++)
        {
            address = addresses.at(i);
            table.emptyList.push_back(address);
            table.emptyNum++;
            return_addresses.push_back(address);
            vector<int>::iterator it;
            for(it=table.recordList.begin();it!=table.recordList.end();it++){
                if(*it==address){
                    table.recordList.erase(it);
                    table.recordNum--;
                    break;
                }
            }

        }
        return return_addresses;
    }
    for(int i = 0; i < addresses.size(); i++)
    {
        address = addresses.at(i);
        //        tempRow=connectRow(table,fp,address);
        tempRow = connectRow(table, filename, address);
        if(compare(table, tempRow, conditions)) //to judge whether the row meets conditions
        {
            table.emptyList.push_back(address);
            table.emptyNum++;
            return_addresses.push_back(address);
            vector<int>::iterator it;
            for(it=table.recordList.begin();it!=table.recordList.end();it++){
                if(*it==address){
                    table.recordList.erase(it);
                    table.recordNum--;
                    break;
                }
            }
            //            deleteNum++;
            i--;
            //            table.blockNum=0;
        }
    }
    //    fclose(fp);//

    return return_addresses;
    //    return deleteNum;
}

Data RecordManager::select(Table table)
{
    Data datas;
    int i;
    string filename = table.name + ".table";
    //    FILE *fp; //
    int address;
    Row tempRow;

    //    fp=fopen(filename.c_str(),"rb");//

    for(i = 0; i < table.recordNum; i++)
    {
        address = table.recordList.at(i);
        //        tempRow=connectRow(table,fp,address);
        tempRow = connectRow(table, filename, address);
        datas.rows.push_back(tempRow);
    }
    //    fclose(fp);//

    return datas;
}

Data RecordManager::select_without_index(Table table, vector<Condition> conditions)
{
    Data datas;
    int i;
    string filename = table.name + ".table";
    //    FILE *fp; //
    int address;
    Row tempRow;

    //    fp=fopen(filename.c_str(),"rb");//

    for(i = 0; i < table.recordNum; i++)
    {
        address = table.recordList.at(i);
        //        tempRow=connectRow(table,fp,address);
        tempRow = connectRow(table, filename, address);
        if(compare(table, tempRow, conditions)) //to judge whether the row meets conditions
            datas.rows.push_back(tempRow);
    }
    //    fclose(fp);//

    return datas;
}

Data RecordManager::select_with_index(Table table, vector<Condition> conditions, vector<int> addresses)
{
    Data datas;
    int i;
    string filename = table.name + ".table";
    //    FILE *fp; //
    Row tempRow;
    int address;

    //    fp=fopen(filename.c_str(),"rb");//

    for(i = 0; i < addresses.size(); i++)
    {
        address = addresses.at(i);
        //        tempRow=connectRow(table,fp,address);
        tempRow = connectRow(table, filename, address);
        if(compare(table, tempRow, conditions)) //to judge whether the row meets conditions
            datas.rows.push_back(tempRow);
    }
    //    fclose(fp);//

    return datas;
}

bool compare(Table table, Row row, vector<Condition> conditions)
{
    for(int i = 0; i < conditions.size(); i++)
    {
        int colNum = conditions.at(i).columnNum;
        string value1 = row.columns.at(colNum);
        string value2 = conditions.at(i).value;
        int value_int1, value_int2;
        float value_float1, value_float2;

        switch(table.attributes.at(colNum).type)
        {
        case CHAR:
            switch(conditions.at(i).op)
            {
            case Lt:
                if(value1 >= value2) return false;
                break;
            case Le:
                if(value1 > value2) return false;
                break;
            case Gt:
                if(value1 <= value2) return false;
                break;
            case Ge:
                if(value1 < value2) return false;
                break;
            case Eq:
                if(value1 != value2) return false;
                break;
            case Ne:
                if(value1  == value2) return false;
                break;
            }
            break;
        case INT:
            value_int1 = atoi(value1.c_str());
            value_int2 = atoi(value2.c_str());
            switch(conditions.at(i).op)
            {
            case Lt:
                if(value_int1 >= value_int2) return false;
                break;
            case Le:
                if(value_int1 > value_int2) return false;
                break;
            case Gt:
                if(value_int1 <= value_int2) return false;
                break;
            case Ge:
                if(value_int1 < value_int2) return false;
                break;
            case Eq:
                if(value_int1 != value_int2) return false;
                break;
            case Ne:
                if(value_int1  == value_int2) return false;
                break;
            }
            break;
        case FLOAT:
            value_float1 = atof(value1.c_str());
            value_float2 = atof(value2.c_str());
            switch(conditions.at(i).op)
            {
            case Lt:
                if(value_float1 >= value_float2) return false;
                break;
            case Le:
                if(value_float1 > value_float2) return false;
                break;
            case Gt:
                if(value_float1 <= value_float2) return false;
                break;
            case Ge:
                if(value_float1 < value_float2) return false;
                break;
            case Eq:
                if(value_float1 != value_float2) return false;
                break;
            case Ne:
                if(value_float1  == value_float2) return false;
                break;
            }
            break;
        }
    }

    return true;
}

//Row connectRow(Table table,FILE *fp,int address){
Row connectRow(Table table, string filename, int address)
{
    Row tempRow;
    char attribute_char[32];//
    int attribute_int;
    float attribute_float;

    for(int j = 0; j < table.attriNum; j++)
    {
        //        fseek(fp,address,SEEK_SET);//the file stream point locate at current position
        if(table.attributes.at(j).type == CHAR)
        {
            buffer.getBufferData(filename, address, attribute_char, 1, table.attributes.at(j).length);
            //			fread ( attribute_char, 1, table.attributes.at(j).length, fp) ;//
            tempRow.columns.push_back(attribute_char);
        }
        else if(table.attributes.at(j).type == INT)
        {
            buffer.getBufferData(filename, address, &attribute_int, 4, 1);
            //			fread ( &attribute_int, 4, 1, fp) ;//
            itoa(attribute_int, attribute_char, 10);
            tempRow.columns.push_back(attribute_char);
        }
        else
        {
            //			fread ( &attribute_float, 4, 1, fp) ;//
            buffer.getBufferData(filename, address, &attribute_float, 4, 1);
            sprintf(attribute_char, "%f", attribute_float);
            tempRow.columns.push_back(attribute_char);
        }

        address += table.attributes.at(j).length;
    }

    return tempRow;
}

vector<string> RecordManager::select(Table table, int columnNum)
{
    vector<string> stringData;
    int i;
    string filename = table.name + ".table";
    //    FILE *fp; //
    int address;
    int length = 0;
    char attribute_char[32];
    int attribute_int;
    float attribute_float;

    //	fp=fopen(filename.c_str(),"rb");//

    for(int j = 0; j < columnNum; j++)
    {
        length += table.attributes.at(j).length;
    }

    if(table.attributes.at(columnNum).type == CHAR)
    {
        for(i = 0; i < table.recordNum; i++)
        {
            address = table.recordList.at(i) + length;
            //			fseek(fp,address,SEEK_SET);//the file stream point locate at current position
            buffer.getBufferData(filename, address, attribute_char, 1, table.attributes.at(columnNum).length);
            //			fread ( attribute_char, 1, table.attributes.at(columnNum).length, fp) ;
            stringData.push_back(attribute_char);
        }
    }
    else if(table.attributes.at(columnNum).type == INT)
    {
        for(i = 0; i < table.recordNum; i++)
        {
            address = table.recordList.at(i) + length;
            //			fseek(fp,address,SEEK_SET);//the file stream point locate at current position
            buffer.getBufferData(filename, address, &attribute_int, 4, 1);
            //			fread ( &attribute_int, 4, 1, fp) ;//
            itoa(attribute_int, attribute_char, 10);
            stringData.push_back(attribute_char);
        }
    }
    else
    {
        for(i = 0; i < table.recordNum; i++)
        {
            address = table.recordList.at(i) + length;
            //			 fseek(fp,address,SEEK_SET);//the file stream point locate at current position
            buffer.getBufferData(filename, address, &attribute_float, 4, 1);
            //			 fread ( &attribute_float, 4, 1, fp) ;//
            sprintf(attribute_char, "%f", attribute_float);
            stringData.push_back(attribute_char);
        }
    }
    //    fclose(fp);//

    return stringData;
}

vector<string> RecordManager::select(Table table, int columnNum, vector<int> addresses)
{
    vector<string> stringData;
    int i;
    string filename = table.name + ".table";
    //    FILE *fp; //
    int address;
    int length = 0;
    char attribute_char[32];
    int attribute_int;
    float attribute_float;

    //	fp=fopen(filename.c_str(),"rb");//

    for(int j = 0; j < columnNum; j++)
    {
        length += table.attributes.at(j).length;
    }

    if(table.attributes.at(columnNum).type == CHAR)
    {
        for(i = 0; i < addresses.size(); i++)
        {
            address = addresses.at(i) + length;
            //			fseek(fp,address,SEEK_SET);//the file stream point locate at current position
            buffer.getBufferData(filename, address, attribute_char, 1, table.attributes.at(columnNum).length);
            //			fread ( attribute_char, 1, table.attributes.at(columnNum).length, fp) ;
            stringData.push_back(attribute_char);
        }
    }
    else if(table.attributes.at(columnNum).type == INT)
    {
        for(i = 0; i < addresses.size(); i++)
        {
            address = addresses.at(i) + length;
            //			fseek(fp,address,SEEK_SET);//the file stream point locate at current position
            buffer.getBufferData(filename, address, &attribute_int, 4, 1);
            //			fread ( &attribute_int, 4, 1, fp) ;//
            itoa(attribute_int, attribute_char, 10);
            stringData.push_back(attribute_char);
        }
    }
    else
    {
        for(i = 0; i < addresses.size(); i++)
        {
            address = addresses.at(i) + length;
            //			 fseek(fp,address,SEEK_SET);//the file stream point locate at current position
            buffer.getBufferData(filename, address, &attribute_float, 4, 1);
            //			 fread ( &attribute_float, 4, 1, fp) ;//
            sprintf(attribute_char, "%f", attribute_float);
            stringData.push_back(attribute_char);
        }
    }
    //    fclose(fp);//

    return stringData;
}
