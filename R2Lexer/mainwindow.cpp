/****************************************************
 * @Copyright © 2021-2023 Lyuyk. All rights reserved.
 *
 * @FileName: mainwindow.cpp
 * @Brief: 主窗体源文件
 * @Module Function:
 *
 * @Current Version: 1.2
 * @Author: 吕英勤
 * @Modifier: 吕英勤
 * @Finished Date: 2023/3/3
 *
 * @Version History: 1.1
 *                   1.2 current version
 *
 ****************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ndfa.h"
//Qt lib
#include <QHash>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
//std lib
#include <vector>
#include <queue>
#include <set>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /*菜单栏与槽函数连接*/
    connect(ui->action_exit,&QAction::triggered,this,&MainWindow::exit);
    connect(ui->action_openFile,&QAction::triggered,this,&MainWindow::on_pushButton_openRegexFile_clicked);
    connect(ui->action_saveFile,&QAction::triggered,this,&MainWindow::on_pushButton_saveRegexToFile_clicked);
    connect(ui->action_2NFA,&QAction::triggered,this,&MainWindow::on_pushButton_2NFA_clicked);
    connect(ui->action_2DFA,&QAction::triggered,this,&MainWindow::on_pushButton_2DFA_clicked);
    connect(ui->action_mDFA,&QAction::triggered,this,&MainWindow::on_pushButton_mDFA_clicked);

    /*表格属性设置*/
    ui->tableWidget_NFA->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget_DFA->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget_mDFA->setEditTriggers(QAbstractItemView::NoEditTriggers);

    /*界面初始化*/
    ui->tabWidget_Graph->setCurrentIndex(0);

    /*按键操作初始化*/
    ui->pushButton_2DFA->setDisabled(true);
    ui->pushButton_mDFA->setDisabled(true);

    /*一些参数的初始化*/
    regexStr="";
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::exit()
{
    window()->close();
}

/**
 * @brief MainWindow::on_pushButton_openRegexFile_clicked
 * 打开正则表达式文件
 */
void MainWindow::on_pushButton_openRegexFile_clicked()
{
    QString srcFilePath=QFileDialog::getOpenFileName(this,"选择正则表达式文件（仅.txt类型）","/", "Files(*.txt)");
    QFile srcFile(srcFilePath);
    if(!srcFile.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QMessageBox::warning(NULL, "文件", "文件打开失败");
        return;
    }
    QTextStream textInput(&srcFile);
    textInput.setEncoding(QStringConverter::Utf8);//设置编码，防止中文乱码

    QString line;
    ui->plainTextEdit_console->insertPlainText("读取正则表达式文件...\n");
    while(!textInput.atEnd())
    {
        line=textInput.readLine().toUtf8();//按行读取文件
        regexStr.append(line.trimmed());
        ui->plainTextEdit_Regex->insertPlainText(line);//一行行显示
        regexStr+=line;//初始化regexStr
    }
    ui->plainTextEdit_console->insertPlainText("读取完成...\n");
    srcFile.close();
}

/**
 * @brief MainWindow::on_pushButton_saveRegexToFile_clicked
 * 保存正则表达式到文件（覆盖写入）
 */
void MainWindow::on_pushButton_saveRegexToFile_clicked()
{
    QString tgtFilePath=QFileDialog::getSaveFileName(this,"选择保存路径","/");
    QFile tgtFile(tgtFilePath);
    if(!tgtFile.open(QIODevice::ReadWrite|QIODevice::Text|QIODevice::Truncate))
    {
        QMessageBox::warning(NULL, "文件", "文件打开/写入失败");
        return;
    }
    QTextStream outputFile(&tgtFile);
    QString tgStr=ui->plainTextEdit_Regex->toPlainText();
    outputFile<<tgStr;
    tgtFile.close();
    QMessageBox::information(NULL,"文件","文件保存成功(saveRegex)");
}

void MainWindow::on_pushButton_2NFA_clicked()
{
    regexStr=ui->plainTextEdit_Regex->toPlainText();//home
    NDFAG.strToNfa(NDFAG.in2Suffix(regexStr));

//====================================================
//    int rowCount=NFAStateNum;//记录NFA状态数
//    int epsColN=OpCharSet.size()+1;//最后一列 epsilon 列号
//    int colCount=OpCharSet.size()+3;
//    QStringList OpStrList=OpCharSet.values();
//    std::sort(OpStrList.begin(),OpStrList.end());
//    OpStrList.push_front("状态号");
//    OpStrList.push_back("epsilon");
//    OpStrList.push_back("初/终态");

//    //ui->plainTextEdit_console->insertPlainText("NFA states' count:"+QString::number(NFAStateNum)+'\n');
//    //ui->plainTextEdit_console->insertPlainText("Operators' count:"+QString::number(OpCharSet.size())+'\n');

//    qDebug()<<OpStrList;
//    ui->tableWidget_NFA->setRowCount(rowCount);
//    ui->tableWidget_NFA->setColumnCount(colCount);
//    /*设置表头 行*/
//    ui->tableWidget_NFA->setHorizontalHeaderLabels(OpStrList);
//    /*设置表头 列*/
//    for(int row=0;row<rowCount;row++)
//    {
//        //状态号
//        ui->tableWidget_NFA->setItem(row,0,new QTableWidgetItem(QString::number(row)));
//        //epsilon转换集合
//        QString epsSetStr="";
//        QSet<int>::iterator it;
//        for(it=NFAStateArr[row].eSUnion.begin();it!=NFAStateArr[row].eSUnion.end();it++)
//        {
//            epsSetStr+=QString::number(*it)+",";
//        }
//        ui->tableWidget_NFA->setItem(row,epsColN,new QTableWidgetItem(epsSetStr));
//        qDebug()<<NFAStateArr[row].eSUnion;
//        qDebug()<<"eps"+epsSetStr;

//        //非epsilon转换
//        int colN=OpStrList.indexOf(NFAStateArr[row].val);
//        //qDebug()<<colN;
//        if(colN != -1)
//        {
//            ui->tableWidget_NFA->setItem(row,colN,new QTableWidgetItem(QString::number(NFAStateArr[row].tState)));
//        }

//        if(NFAStateArr[row].stateNum==NFAG.startNode->stateNum)
//        {
//            ui->tableWidget_NFA->setItem(row,epsColN+1,new QTableWidgetItem("初态"));
//        }

//        if(NFAStateArr[row].stateNum==NFAG.endNode->stateNum)
//        {
//            ui->tableWidget_NFA->setItem(row,epsColN+1,new QTableWidgetItem("终态"));
//        }


//        //控制台输出
//        //ui->plainTextEdit_console->insertPlainText(QString::number(row)+'\n');
//    }

    NDFAG.printDFA(ui->tableWidget_NFA);

    ui->pushButton_2DFA->setEnabled(true);//完成转换工作后可以允许NFA到DFA的转换工作
}



void MainWindow::on_pushButton_2DFA_clicked()
{
    NDFAG.NFA2DFA();

    /*======================显示处理========================*/

    NDFAG.printDFA(ui->tableWidget_DFA);

    /*======================交互界面处理========================*/

    ui->pushButton_mDFA->setEnabled(true);//允许最小化DFA
}


/**
 * @brief MainWindow::on_pushButton_mDFA_clicked
 * 最小化DFA并将其状态转换表显示出来
 */
void MainWindow::on_pushButton_mDFA_clicked()
{
    NDFAG.DFA2mDFA();

    /*==========显示处理=================*/

    NDFAG.printMDFA(ui->tableWidget_mDFA);

    //切换表格
    ui->tabWidget_Graph->setCurrentIndex(2);

}

void MainWindow::on_pushButton_clearConsole_clicked()
{        
    /*界面初始化*/
    ui->tabWidget_Graph->setCurrentIndex(0);
    ui->plainTextEdit_console->clear();
    ui->plainTextEdit_Regex->clear();
    ui->tableWidget_NFA->clear();
    ui->tableWidget_DFA->clear();
    ui->tableWidget_mDFA->clear();

    /*按键操作初始化*/
    ui->pushButton_2NFA->setEnabled(true);
    ui->pushButton_2DFA->setDisabled(true);
    ui->pushButton_mDFA->setDisabled(true);

    /*一些参数的初始化*/
    regexStr="";

}
