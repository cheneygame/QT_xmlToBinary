#include "utility.h"
#include <QDebug>
#include <QTextCodec>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QDomDocument>
#include "xlsx/xlsxdocument.h"

#define MAX_ROW_COLUMN 1000000
#define IS_UTF82UNICODE 0

int GetFiledType(QString type_name)
{
    int type = 0;
    if(type_name == "int")
        type = SELF_TYPE_INT32;
    else if(type_name == "longlong")
        type = SELF_TYPE_INT64;
    else if(type_name == "string")
        type = SELF_TYPE_STRING;
    else if(type_name == "date")
        type = SELF_TYPE_DATE;
    else if(type_name == "short")
        type = SELF＿TYPE_SHORT;
    else if(type_name == "double")
        type = SELF＿TYPE_DOUBLE;
    else if(type_name == "float")
        type = SELF＿TYPE_FLOAT;
    return type;
}

int CopyValueToBuf(struct ST_FIELD_INFO st_field,QVariant &value,char*buf,int &max_size)
{
    int size = 0;
    if(st_field.type == SELF_TYPE_INT32)
    {
        int data = value.toInt();
        size = sizeof(data);
        memcpy(buf,&data,size);
        max_size= size;
    }
    else if(st_field.type == SELF_TYPE_INT64)
    {
        long long data = value.toLongLong();
        size = sizeof(data);
        memcpy(buf,&data,size);
        max_size= size;
    }
    else if(st_field.type == SELF_TYPE_STRING)
    {
        //addLine(value.toString(),true);
        //addLine(QString("  src_size : %1").arg(value.toString().toLocal8Bit().length()));
        QString data;

        if(IS_UTF82UNICODE)//utf8 2 unicode
        {
            QTextCodec *codec=QTextCodec::codecForName("UTF-8");//"GB2312");
            //转unicode
            data = codec->toUnicode( value.toString().toLocal8Bit());//.data());

        }
        else //utf8
        {
            data = value.toString();//.toUtf8();
           // data = QString::fromUtf8(value.toString().toUtf8().data());
        }
        //addLine(data,true);
        //addLine(QString("  out_size : %1").arg(data.toLocal8Bit().length()));

        short nlen = st_field.size;
        int off = 0;
        memcpy(buf,&nlen,sizeof(nlen));
        off += sizeof(nlen);
        memcpy(buf+off,data.toLocal8Bit().data(),nlen);
        max_size = off+data.toLocal8Bit().length();//这个要回传真实字符串的大小
        buf[max_size] = '\0';

/*
        {
            addLine("buf=",true);
            for(int counter= 0 ; counter <data.toLocal8Bit().length();++counter)
            {
               addLine(QString(buf[off+counter]));
               addLine("-");
            }
        }
*/
        //memcpy(buf+off,data.toLocal8Bit().data(),nlen);

    }
    else if(st_field.type == SELF_TYPE_DATE)
    {

    }
    else if(st_field.type == SELF＿TYPE_SHORT)
    {
        short data = value.toInt();
        size = sizeof(data);
        memcpy(buf,&data,size);
        max_size= size;
    }
    else if(st_field.type == SELF＿TYPE_DOUBLE)
    {
        double data = value.toDouble();
        size = sizeof(double);
        memcpy(buf,&data,size);
        max_size= size;
        //qDebug() << "SELF＿TYPE_DOUBLE:" <<size<<"|"<<sizeof(data)<<"|"<<data<<"|"<<value;
    }
    else if(st_field.type == SELF＿TYPE_FLOAT) //should not use float in lua
    {
        float data = value.toFloat();
        size = sizeof(float);
        memcpy(buf,&data,size);
        max_size= size;
        //qDebug() << "SELF＿TYPE_FLOAT:" <<size<<"|"<<sizeof(data)<<"|"<<data<<"|t1:"<<t1<<"|aa:"<<aa;
    }
    return st_field.type;

}



void addLine(QString new_line ,bool next_row )
{
    /*
    show_TextEdit->moveCursor(QTextCursor::EndOfLine);
    if(next_row)
        show_TextEdit->insertPlainText("\n");
    else
        show_TextEdit->insertPlainText("\t");
    show_TextEdit->insertPlainText(new_line);
    */

    QFile *log_file = new QFile();
    log_file->setFileName("./log.txt");
    log_file->open(QIODevice::Append);
    if(next_row)
        log_file->write("\r\n");
    else
        log_file->write("\t");
    log_file->write(new_line.toLocal8Bit().data());
    log_file->close();
}

bool parseExcel_Xml(QString file_name,struct ST_EXCEL_INFO &st_config)
{
    addLine(file_name,true);
    if(file_name.isEmpty())
          return false;

      QFile file(file_name);
      if(!file.open(QFile::ReadOnly | QFile::Text))
      {
          QMessageBox::information(NULL, QString("title"), QString("open error 2!")+file_name);

          return false;
      }

      QDomDocument document;
      QString error;
     // struct ST_EXCEL_INFO st_config;
      int row = 0, column = 0;
      if(!document.setContent(&file, false, &error, &row, &column))
      {
          QMessageBox::information(NULL, QString("title"), QString("parse file failed at line row and column") + QString::number(row, 10) + QString(",") + QString::number(column, 10));

          return false;
      }

      if(document.isNull())
      {
          QMessageBox::information(NULL, QString("title"), QString("document is null!"));

          return false;
      }

      QDomElement root = document.documentElement();
      if(root.isNull())
              return false;
      //root_tag_name
      QString root_tag_name = root.tagName();

      QDomElement table_node = root.firstChildElement();//默认只有一个子节点table
      if(table_node.isNull())
              return false;
      st_config.item_name = table_node.attribute("name");
      st_config.excel_file_name = table_node.attribute("ExcelFile");
      st_config.excel_sheet_name = table_node.attribute("sheet");
      st_config.output_file_name = table_node.attribute("OutputFile");
      addLine(table_node.tagName(),true);
      addLine(st_config.item_name);
      addLine(st_config.excel_file_name);
      addLine(st_config.excel_sheet_name);
      addLine(st_config.output_file_name);

      QDomNodeList field_list = table_node.childNodes();//filed 子节点
      int list_num = field_list.count();
      st_config.field_info.clear();
      for(int counter = 0; counter < list_num; ++counter)
      {
          QDomElement field_node = field_list.item(counter).toElement();
          if(field_node.isNull())
              continue;
            struct ST_FIELD_INFO field_info;
            field_info.name = field_node.attribute("name");
            field_info.type = GetFiledType(field_node.attribute("type"));
            field_info.size = field_node.attribute("size").toInt();
            //addLine(field_node.tagName(),true);
            //addLine(field_info.name);
            //addLine(field_node.attribute("type"));
            //addLine(field_node.attribute("size")) ;
            st_config.field_info.push_back(field_info);

      }//end of for
      return true;
}



void parseConfig_Xml(QString file_name,struct ST_CONFIG_INFO &st_config)
{
    if(file_name.isEmpty())
          return;

      QFile file(file_name);
      if(!file.open(QFile::ReadOnly | QFile::Text))
      {
          QMessageBox::information(NULL, QString("title"), QString("open error 2!")+file_name);

          return;
      }

      QDomDocument document;
      QString error;
     // struct ST_EXCEL_INFO st_config;
      int row = 0, column = 0;
      if(!document.setContent(&file, false, &error, &row, &column))
      {
          QMessageBox::information(NULL, QString("title"), QString("parse file failed at line row and column") + QString::number(row, 10) + QString(",") + QString::number(column, 10));

          return;
      }

      if(document.isNull())
      {
          QMessageBox::information(NULL, QString("title"), QString("document is null!"));

          return;
      }

      QDomElement root = document.documentElement();
      if(root.isNull())
              return;
      //root_tag_name
      QString root_tag_name = root.tagName();
      addLine(root_tag_name,true);
      QDomElement solution_node = root.firstChildElement();//默认只有一个子节点solution
      if(solution_node.isNull())
              return;
        st_config.name = solution_node.attribute("name");
        st_config.xmlfilepath = solution_node.attribute("xmlfilepath");
        st_config.excelfilepath = solution_node.attribute("excelfilepath");
        st_config.outputpath = solution_node.attribute("outputpath");

      QDomNodeList package_list = solution_node.childNodes();//package子节点
      int list_num = package_list.count();
       st_config.xml_file_Vec.clear();
      for(int counter = 0; counter < list_num; ++counter)
      {
          QDomElement package_node = package_list.item(counter).toElement();
          if(package_node.isNull())
              continue;
          st_config.xml_file_Vec.push_back(package_node.attribute("xml"));

      }//end of for

}


bool XLSX2TBL(QString excel_path_name,QString output_path_name,struct ST_EXCEL_INFO &st_xml_info)
{
    qDebug() << excel_path_name;
    QString excel_file_name =  excel_path_name+(st_xml_info.excel_file_name);
    QString tbl_file_name = output_path_name+(st_xml_info.output_file_name);
    addLine(QString("parseEXCEL: ")+excel_file_name,true);
    QXlsx::Document xlsx(excel_file_name);
    xlsx.selectSheet(st_xml_info.excel_sheet_name);
    //get max row
    int max_row = 0;
    while(max_row<MAX_ROW_COLUMN)
    {
        if(xlsx.read(max_row+1,1).toString() == "")
            break;
        ++max_row;
    }//end of while
    addLine((QString("max row: %1").arg(max_row-1)),true);//第一行描述信息不计算在内
    if(max_row <=1)
        return false;
    //get max column
    int max_column = 0;
    while(max_column <MAX_ROW_COLUMN)
    {
        if(xlsx.read(1,max_column+1).toString() == "")
            break;
        ++max_column;
    }
    addLine((QString("max column: %1").arg(max_column)));



    //get need column seq by xml
    addLine("row ",true);
    QVector<ST_FIELD_INFO>::iterator vec_iter;
    for(vec_iter=st_xml_info.field_info.begin();vec_iter!=st_xml_info.field_info.end();++vec_iter)
    {
        for(int cur_col= 1; cur_col <=max_column;++cur_col )
        {
            if(xlsx.read(1,cur_col).toString() == vec_iter->name)
            {
                vec_iter->column = cur_col;
                addLine((vec_iter->name+":%1").arg(vec_iter->column));
                break;
            }
        }//end of for
    }//end of for

    //parse xlsx 2 tbl
    QFile *tbl_file = new QFile();
    tbl_file->setFileName(tbl_file_name);
    tbl_file->open(QIODevice::WriteOnly);
    tbl_file->reset();
    int  total_size = 0;

    char row_buf[512];
    short field_size = max_row-1;
    memcpy(row_buf,&field_size,sizeof(field_size));
    tbl_file->write(row_buf,sizeof(field_size));
    total_size += sizeof(field_size);

    for( int cur_row = 2; cur_row <= max_row ; ++cur_row)
    {
        for(vec_iter=st_xml_info.field_info.begin();vec_iter!=st_xml_info.field_info.end();++vec_iter)
        {
            QVariant cell_value= xlsx.read(cur_row,vec_iter->column);

            //if(vec_iter == st_xml_info.field_info.begin())
              //  addLine("",true);
            //addLine(cell_value.toString());

            char buf[512];
            int max_size = 0;
            CopyValueToBuf(*vec_iter,cell_value,buf,max_size);
            if(vec_iter->type == SELF_TYPE_STRING)//这个类型的大小按xml配置的来，最后以字节放'\0'
            {
                //buf[max_size] = '\0';
                max_size = vec_iter->size+ sizeof(short);//实际字符串长度变量的大小+字符串大小
            }
            total_size += max_size;
            tbl_file->write(buf,max_size);
        }//end of for vec
    }//end of for cur_row
    tbl_file->close();
    addLine(QString("total_size : %1").arg(total_size),true);
    return true;
}

