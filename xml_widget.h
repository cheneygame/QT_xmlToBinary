#ifndef XML_WIDGET_H
#define XML_WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QProgressBar>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTreeWidget>

#include "utility.h"




class XML_Widget : public QWidget
{
    Q_OBJECT

public:
    XML_Widget(QWidget *parent = 0);
    ~XML_Widget();

private:
    struct ST_CONFIG_INFO st_config;

    QLabel *show_Label;
    QPushButton *choose_config_Button;
    QPushButton *excel_tbl_Button;
    QTextEdit *show_TextEdit;

    QGridLayout *grid_LayOut;
    QTreeWidget *select_widget;
    
    QString server_path;
    QString client_path;
    QPushButton *server_path_Button;
    QPushButton *client_path_Button;

    void addToTextEdit(QString text,bool isnextLine = false,bool isError = false);
    void init_select_widget();
    void updateParentItem(QTreeWidgetItem* item);



private slots:
    void startParse();
    void showFile();
    void treeItemChanged(QTreeWidgetItem* item, int column);
    void setServerPath();
    void setClientPath();


};

#endif // XML_WIDGET_H
