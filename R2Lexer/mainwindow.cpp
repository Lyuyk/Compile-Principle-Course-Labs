/****************************************************
 * @Copyright © 2021-2023 Lyuyk. All rights reserved.
 *
 * @FileName: mainwindow.cpp
 * @Brief: 主窗体源文件
 * @Module Function:
 *
 * @Current Version: 1.2
 * @Author: Lyuyk
 * @Modifier: Lyuyk
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
    connect(ui->action_exit,&QAction::triggered,this,&MainWindow::exit);//退出程序
    connect(ui->action_openFile,&QAction::triggered,this,&MainWindow::on_pushButton_openRegexFile_clicked);//打开正则表达式文件
    connect(ui->action_saveFile,&QAction::triggered,this,&MainWindow::on_pushButton_saveRegexToFile_clicked);//保存正则表达式到文件
    connect(ui->action_2NFA,&QAction::triggered,this,&MainWindow::on_pushButton_2NFA_clicked);//正则表达式转换为NFA
    connect(ui->action_2DFA,&QAction::triggered,this,&MainWindow::on_pushButton_2DFA_clicked);//NFA转换为DFA
    connect(ui->action_mDFA,&QAction::triggered,this,&MainWindow::on_pushButton_mDFA_clicked);//DFA最小化
    connect(ui->action_Lexer,&QAction::triggered,this,&MainWindow::on_pushButton_Lexer_clicked);//最小化DFA生成Lexer

    /*表格属性设置*/
    ui->tableWidget_NFA->setEditTriggers(QAbstractItemView::NoEditTriggers);//不允许编辑
    ui->tableWidget_DFA->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget_mDFA->setEditTriggers(QAbstractItemView::NoEditTriggers);

    /*界面初始化*/
    ui->tabWidget_Graph->setCurrentIndex(0);

    /*按键操作初始化*/
    ui->pushButton_2DFA->setDisabled(true);//功能未能使用前禁用按钮
    ui->pushButton_mDFA->setDisabled(true);
    ui->pushButton_Lexer->setDisabled(true);

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
    printConsole("读取正则表达式文件...");
    while(!textInput.atEnd())
    {
        line=textInput.readLine().toUtf8();//按行读取文件
        regexStr.append(line.trimmed());
        ui->plainTextEdit_Regex->insertPlainText(line);//一行行显示
        regexStr+=line;//初始化regexStr
    }    
    srcFile.close();

    printConsole("正则表达式文件读取完成...");
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
    QMessageBox::information(NULL,"文件","正则表达式文件保存成功");

    printConsole("正则表达式文件保存成功(saveRegex)");
}


void MainWindow::on_pushButton_2NFA_clicked()
{
    regexStr=ui->plainTextEdit_Regex->toPlainText().trimmed();//获取正则表达式

    NDFAG.reg2NFA(regexStr);//调用转换函数
    printConsole("正则表达式已转换为NFA");

    NDFAG.printNFA(ui->tableWidget_NFA);//显示

    ui->tabWidget_Graph->setCurrentIndex(1);//设置显示页面
    ui->pushButton_2DFA->setEnabled(true);//完成转换工作后可以允许NFA到DFA的转换工作
}

/**
 * @brief MainWindow::on_pushButton_2DFA_clicked
 * NFA转换为DFA的槽函数，调用NFA转DFA主函数及相关交互函数操作
 */
void MainWindow::on_pushButton_2DFA_clicked()
{
    NDFAG.NFA2DFA();

    /*======================显示处理========================*/

    ui->tabWidget_Graph->setCurrentIndex(2);
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
    ui->tabWidget_Graph->setCurrentIndex(3);

    /*======================交互界面处理========================*/

    ui->pushButton_Lexer->setEnabled(true);
}

/**
 * @brief MainWindow::on_pushButton_Lexer_clicked
 *使用最小化DFA生成NFA槽函数，调用最小化DFA生成Lexer函数及相关交互操作处理
 */
void MainWindow::on_pushButton_Lexer_clicked()
{
    NDFAG.mDFA2Lexer(srcFilePath);

    /*==========显示处理=================*/
    //切换表格
    ui->tabWidget_Graph->setCurrentIndex(4);
    //显示词法分析程序
    NDFAG.printLexer(ui->plainTextEdit_Lexer);
}

/**
 * @brief MainWindow::printConsole
 * @param str
 * 控制台输出
 */
void MainWindow::printConsole(QString str)
{
    QString timeStr=QTime::currentTime().toString("HH:mm:ss.zzz");
    ui->plainTextEdit_console->appendPlainText("["+timeStr+"] "+str);
}

/**
 * @brief MainWindow::on_pushButton_clearConsole_clicked
 * 复位键槽函数，将重新初始化
 */
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
    ui->pushButton_Lexer->setDisabled(true);

    /*一些参数的初始化*/
    regexStr="";
    NDFAG.init();
}



