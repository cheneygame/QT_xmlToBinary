#ifndef UTILITY_H
#define UTILITY_H
#include <QString>
#include <QHash>
#include <QVector>
#include <QVariant>


enum SELF_TYPE
{
    SELF_TYPE_INT32 = 1,
    SELF_TYPE_INT64 = 2,
    SELF_TYPE_STRING = 3,
    SELF_TYPE_DATE = 4,
    SELF＿TYPE_SHORT = 5,
    SELF＿TYPE_DOUBLE = 6,
};

struct ST_XML_NODE_VALUE_INFO
{
    ST_XML_NODE_VALUE_INFO()
    {
        name = "";
        type= 0;
        data = 0;
    }
    QString name;
    int type;
    QVariant data;
};

struct ST_FIELD_INFO
{
    ST_FIELD_INFO()
    {
        name= "";
        type = 0;
        size = 0;
        column = 0;
    }

    QString name;
    int type;
    int size;
    int column ;//对应excel表的列数
};

struct ST_EXCEL_INFO
{
    ST_EXCEL_INFO()
    {
        item_name = "";
        excel_file_name = "";
        excel_sheet_name = "";
        output_file_name = "";
        field_info.clear();
    }
    QString item_name;
    QString excel_file_name;
    QString excel_sheet_name;
    QString output_file_name;
    QVector<ST_FIELD_INFO> field_info;
};

struct ST_CONFIG_INFO
{
    ST_CONFIG_INFO()
    {
        init();
    }
    void init()
    {
        name = "";
        xmlfilepath = "";
        excelfilepath = "";
        outputpath = "";
        xml_file_Vec.clear();
    }

    QString name;
    QString xmlfilepath;
    QString excelfilepath;
    QString outputpath;
    QVector<QString> xml_file_Vec;
};


int GetFiledType(QString type_name);
int CopyValueToBuf(struct ST_FIELD_INFO st_field,QVariant &value,char*buf,int &max_size);

void addLine(QString new_line,bool next_row = false);
void parseConfig_Xml(QString file_name,struct ST_CONFIG_INFO &st_config);
bool parseExcel_Xml(QString file_name,struct ST_EXCEL_INFO &st_config);
bool XLSX2TBL(QString excel_path_name,QString output_path_name,struct ST_EXCEL_INFO &st_xml_info);



#endif // UTILITY_H

