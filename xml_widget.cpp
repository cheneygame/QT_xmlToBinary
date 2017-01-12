#include "xml_widget.h"
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QSize>
#include <QProgressDialog>
#include "utility.h"

#define MAX_NAME_SIZE 64;
#define MAX_DESCPIPE_SIZE 128;



XML_Widget::XML_Widget(QWidget *parent)
    : QWidget(parent)
{
    st_config.init();
    resize( QSize( 700, 600 ));
    server_path = "./config_server.xml";
    client_path = "./config_client.xml";
    server_path_Button = new QPushButton("server_path");
    client_path_Button = new QPushButton("clien_path");
    show_Label = new QLabel(server_path);
    choose_config_Button = new QPushButton("change_config");
    excel_tbl_Button = new QPushButton("start");
    show_TextEdit = new QTextEdit();


    QString config_file_name = show_Label->text();
    parseConfig_Xml(config_file_name,st_config);

    select_widget = new QTreeWidget();
    init_select_widget();

    grid_LayOut= new QGridLayout(this);
    grid_LayOut->addWidget(server_path_Button,0,1);
    grid_LayOut->addWidget(client_path_Button,1,1);
    grid_LayOut->addWidget(show_Label,2,0);
    grid_LayOut->addWidget(choose_config_Button,2,1);
    grid_LayOut->addWidget(excel_tbl_Button,3,1);
    grid_LayOut->addWidget(select_widget,3,0);
    grid_LayOut->addWidget(show_TextEdit,4,0);

    connect(choose_config_Button,SIGNAL(clicked()),this,SLOT(showFile()));
    connect(excel_tbl_Button,SIGNAL(clicked()),this,SLOT(startParse()));
    connect(select_widget,SIGNAL(itemChanged(QTreeWidgetItem*,int)),this,SLOT(treeItemChanged(QTreeWidgetItem*,int)));
    connect(server_path_Button,SIGNAL(clicked()),this,SLOT(setServerPath()));
    connect(client_path_Button,SIGNAL(clicked()),this,SLOT(setClientPath()));
}

XML_Widget::~XML_Widget()
{

}


void XML_Widget::addToTextEdit(QString text,bool isnextLine,bool isError)
{
    show_TextEdit->moveCursor(QTextCursor::EndOfLine);
    if(isnextLine)
        show_TextEdit->insertPlainText("\n");
    else
        show_TextEdit->insertPlainText("\t");
    if(isError)
        show_TextEdit->setTextColor(QColor(Qt::red));
    else
        show_TextEdit->setTextColor(QColor(Qt::black));
    show_TextEdit->insertPlainText(text);


}

void XML_Widget::startParse()
{
    //进度条
    QProgressDialog *progressDialog=new QProgressDialog();
    progressDialog->setFont(QFont("ZYSong18030",12));
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setMinimumDuration(5);
    progressDialog->setWindowTitle(tr("Please Wait"));
    progressDialog->setLabelText(tr("Parsing..."));
    progressDialog->setCancelButtonText(tr("Cancel"));
    progressDialog->setRange(0,st_config.xml_file_Vec.size());

    show_TextEdit->clear();
    addToTextEdit("###########################################################################\n",true);
    addToTextEdit("\t START "+show_Label->text()+"\n",true);

    QVector<QString>::iterator vec_iter = st_config.xml_file_Vec.begin();
    QTreeWidgetItem *group = select_widget->takeTopLevelItem(0);
    if(!group) return;
    int child_max_num = group->childCount();
    progressDialog->setRange(0,child_max_num);
    progressDialog->setValue(0);
    int counter = 0;
    for(;vec_iter != st_config.xml_file_Vec.end();++vec_iter)
    {
        ++counter;
        progressDialog->setValue(counter);
        if(progressDialog->wasCanceled())
            break;
        //检测相应的项有没被选中

        QTreeWidgetItem *child = group->child(counter-1);
        if(!child) continue;
        if(child->checkState(0) != Qt::Checked)
            continue;

        struct ST_EXCEL_INFO st_xml_info;
        QString file_name = st_config.xmlfilepath+*vec_iter;
        if(parseExcel_Xml(file_name,st_xml_info))
        {
            if(XLSX2TBL(st_config.excelfilepath,st_config.outputpath,st_xml_info))
            {
                addToTextEdit("\nparse "+/*st_config.excelfilepath+*/st_xml_info.excel_file_name+"   to   "+/*st_config.outputpath+*/st_xml_info.output_file_name+" success",true);
                continue;
            }
        }
        addToTextEdit("\nERROR  parse "+file_name+" FAILURE!!!",true,true);
        //break;

    }//end of for
    addToTextEdit("\n\tALL FINISHED !",true);
    addToTextEdit("\n###########################################################################",true);
    init_select_widget();//不知道为什么这个控件会消失，这里初始化一下
}



void XML_Widget::showFile()
{
    QString file_name = QFileDialog::getOpenFileName(this,"open file dialog","./","XML files(*.xml)");
    show_Label->setText(file_name);
    if(show_Label->text() == "")
       show_Label->setText(server_path);
    QString config_file_name = show_Label->text();
    parseConfig_Xml(config_file_name,st_config);
    init_select_widget();
}

void XML_Widget::init_select_widget()
{
    select_widget->clear();
    select_widget->setHeaderHidden(true);
    //第一个分组
    QTreeWidgetItem *group1 = new QTreeWidgetItem(select_widget);
    group1->setText(0,"select all");
    group1->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsSelectable);
    group1->setCheckState(0,Qt::Unchecked);

    const int max_sub_size = st_config.xml_file_Vec.size();

    for(int counter = 0; counter < max_sub_size; ++counter)
    {
        QTreeWidgetItem *subItem = new QTreeWidgetItem(group1);
        subItem->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsSelectable);
        QString sub_name = st_config.xml_file_Vec[counter];
        subItem->setText(0,sub_name.arg(counter+1));
        subItem->setCheckState(0,Qt::Unchecked);
    }
    select_widget->expandItem(group1);
}


void XML_Widget::updateParentItem(QTreeWidgetItem* item)
{
    if( !item )
        return;
    QString itemText = item->text(0);
    //selcected
    if(Qt::Checked == item->checkState(0))
    {
        //QTreeWidgetItem *parent = item->parent();
        int count = item->childCount();
        if(count >0)
        {
            //其子节点都设置成这个节点的状态
            for(int counter = 0 ;counter<count; ++counter)
            {
                item->child(counter)->setCheckState(0,Qt::Checked);
            }
        }
        else//子节点
        {
            updateParentItem(item);
        }
    }
    else if(Qt::Unchecked == item->checkState(0))
    {
        int count = item->childCount();
        if(count >0)
        {
            //其子节点都设置成这个节点的状态
            for(int counter = 0 ;counter<count; ++counter)
            {
                item->child(counter)->setCheckState(0,Qt::Unchecked);
            }
        }
        else//子节点
        {
            updateParentItem(item);
        }
    }
}

void XML_Widget::treeItemChanged(QTreeWidgetItem* item, int column)
{
   // qDebug()<<"column="<<column;
    QTreeWidgetItem *parent = item->parent();
    if(!parent)
    {
        //return;
        if(item->checkState(0) == Qt::Checked)
        {
           // qDebug()<<"parent-Checked";
            int childCount = item->childCount();
            for(int counter = 0 ;counter<childCount; ++counter)
            {
               QTreeWidgetItem *childItem = item->child(counter);
               if(childItem && childItem->checkState(0) == Qt::Unchecked)
                   childItem->setCheckState(0,Qt::Checked);
            }//end of for
        }
        else if(item->checkState(0) == Qt::Unchecked)
        {
            //qDebug()<<"parent-Unchecked";
            int childCount = item->childCount();
            for(int counter = 0 ;counter<childCount; ++counter)
            {
               QTreeWidgetItem *childItem = item->child(counter);
               if(childItem && childItem->checkState(0) == Qt::Checked)
                   childItem->setCheckState(0,Qt::Unchecked);
            }//end of for
        }
        else
        {
            //qDebug()<<"parent-PartiallyChecked";
        }
    }
    else
    {
        int selectedCount = 0;
        int childCount = parent->childCount();
        for(int counter = 0 ;counter<childCount; ++counter)
        {
           QTreeWidgetItem *childItem = parent->child(counter);
           if(childItem && childItem->checkState(0) == Qt::Checked)
               ++selectedCount;
        }//end of for
        //qDebug()<<"selectedCount="<<selectedCount<<"; childCount="<<childCount;
        if(selectedCount <= 0)
        {
            parent->setCheckState(0,Qt::Unchecked);
        }
        else if(selectedCount>0 && selectedCount <childCount)
        {
            parent->setCheckState(0,Qt::PartiallyChecked);
        }
        else if(selectedCount == childCount)
        {
            parent->setCheckState(0,Qt::Checked);
        }
    }
}

void XML_Widget::setServerPath()
{
    show_Label->setText(server_path);
    QString config_file_name = show_Label->text();
    parseConfig_Xml(config_file_name,st_config);
    init_select_widget();
}

void XML_Widget::setClientPath()
{
    show_Label->setText(client_path);
    QString config_file_name = show_Label->text();
    parseConfig_Xml(config_file_name,st_config);
    init_select_widget();
}

