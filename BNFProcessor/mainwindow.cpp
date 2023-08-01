/****************************************************
 * @Copyright © 2021-2023 Lyuyk. All rights reserved.
 *
 * @FileName: mainwindow.cpp
 * @Brief: 主窗口源文件
 * @Module Function:
 *
 * @Current Version: 1.3
 * @Author: Lyuyk
 * @Modifier: Lyuyk
 * @Finished Date: 2023/4/21
 *
 * @Version History: 1.1
 *                   1.2 完善了部分注释，提高了部分代码的可读性
 *                   1.3 对窗体新功能，函数做出相应的适配
 *
 ****************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QFileDialog>
#include <QList>
#include <QMessageBox>
#include <QStack>
#include <QStringList>
#include <QTextStream>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tabWidget->setCurrentIndex(grammarTab);

    // 界面交互处理
    ui->pushButton_eliminateLeftCommonFactor->setDisabled(true);
    ui->pushButton_eliminateLeftRecursion->setDisabled(true);
    ui->pushButton_set->setDisabled(true);
    ui->pushButton_LL1->setDisabled(true);
    ui->pushButton_CST->setDisabled(true);
    ui->pushButton_AST->setDisabled(true);

    /*菜单栏与槽函数连接*/
    connect(ui->action_exit, &QAction::triggered, this, &MainWindow::exit);
    connect(ui->action_open, &QAction::triggered, this, &MainWindow::on_pushButton_open_clicked);
    connect(ui->action_save, &QAction::triggered, this, &MainWindow::on_pushButton_save_clicked);
    connect(ui->action_simplify, &QAction::triggered, this, &MainWindow::on_pushButton_process_clicked);
    connect(ui->action_set, &QAction::triggered, this, &MainWindow::on_pushButton_set_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief MainWindow::exit
 * 退出程序
 */
void MainWindow::exit()
{
    window()->close();
}

/**
 * @brief MainWindow::printConsole
 * @param content
 * 控制台输出文字函数
 */
void MainWindow::printConsole(QString content)
{
    QString time = QDateTime::currentDateTime().toString("[hh:mm:ss.zzz] ");

    ui->plainTextEdit_console->appendPlainText(time + content);
}

/**
 * @brief MainWindow::on_pushButton_open_clicked
 * 打开文法文件
 */
void MainWindow::on_pushButton_open_clicked()
{
    QString srcFilePath = QFileDialog::getOpenFileName(this, "选择文法文件（仅.txt类型）", "/", "Files(*.txt)");
    QFile srcFile(srcFilePath);
    if (!srcFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(nullptr, "文件", "文件打开失败");
        return;
    }
    QTextStream textInput(&srcFile);
    textInput.setEncoding(QStringConverter::Utf8); // 设置编码，防止中文乱码

    QString line;
    printConsole("读取文法文件...");
    while (!textInput.atEnd())
    {
        line = textInput.readLine().toUtf8(); // 按行读取文件
        m_grammarStr.append(line.trimmed() + '\n');
        ui->plainTextEdit_edit->appendPlainText(line); // 一行行显示
    }
    printConsole("读取完成...");
    srcFile.close();
}

/**
 * @brief MainWindow::on_pushButton_save_clicked
 * 将编辑框中编辑好的文法文本保存到选择的路径中
 */
void MainWindow::on_pushButton_save_clicked()
{
    QString tgtFilePath = QFileDialog::getSaveFileName(this, "选择保存路径", "/");
    QFile tgtFile(tgtFilePath);
    if (!tgtFile.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
    {
        QMessageBox::warning(nullptr, "文件", "文件保存失败");
        return;
    }
    QTextStream outputFile(&tgtFile);
    QString tgStr = ui->plainTextEdit_edit->toPlainText();
    outputFile << tgStr;
    tgtFile.close();
    QMessageBox::information(nullptr, "文件(saveGrammarRule)", "文件保存成功");
}

void MainWindow::on_pushButton_arrowChar_clicked()
{
    ui->plainTextEdit_edit->insertPlainText("->");
}

void MainWindow::on_pushButton_epsilonChar_clicked()
{
    ui->plainTextEdit_edit->insertPlainText("@");
}

void MainWindow::on_pushButton_orChar_clicked()
{
    ui->plainTextEdit_edit->insertPlainText("|");
}

/**
 * @brief MainWindow::on_pushButton_process_clicked
 * 文法问题全部处理按钮槽函数
 */
void MainWindow::on_pushButton_process_clicked()
{
    BNFProcessor.initGrammar(m_grammarStr);
    BNFProcessor.simplifyGrammar();

    BNFProcessor.printGrammar(ui->plainTextEdit_simplified);
    on_pushButton_eliminateLeftRecursion_clicked();
    on_pushButton_eliminateLeftCommonFactor_clicked();
    on_pushButton_set_clicked();
    on_pushButton_LL1_clicked();
    on_pushButton_CST_clicked();
    on_pushButton_AST_clicked();
}

/**
 * @brief MainWindow::on_pushButton_simplify_clicked
 * 文法化简槽函数
 */
void MainWindow::on_pushButton_simplify_clicked()
{
    m_grammarStr = ui->plainTextEdit_edit->toPlainText();
    BNFProcessor.initGrammar(m_grammarStr);
    BNFProcessor.simplifyGrammar();
    BNFProcessor.printGrammar(ui->plainTextEdit_simplified);
    ui->tabWidget->setCurrentIndex(simplifyTab);

    ui->pushButton_eliminateLeftRecursion->setEnabled(true);
}

/**
 * @brief MainWindow::on_pushButton_eliminateLeftRecursion_clicked
 * 消除左递归槽函数
 */
void MainWindow::on_pushButton_eliminateLeftRecursion_clicked()
{
    ui->plainTextEdit_leftRecursion->clear();

    printConsole("消除左递归...");
    BNFProcessor.eliminateLRecursion();
    printConsole("消除左递归完成...");

    BNFProcessor.printGrammar(ui->plainTextEdit_leftRecursion);
    printConsole("输出处理结果...");

    ui->tabWidget->setCurrentIndex(leftCurTab);
    ui->pushButton_eliminateLeftCommonFactor->setEnabled(true);
}

/**
 * @brief MainWindow::on_pushButton_eliminateLeftCommonFactor_clicked
 * 消除左公因子槽函数
 */
void MainWindow::on_pushButton_eliminateLeftCommonFactor_clicked()
{
    ui->plainTextEdit_leftCommonFactor->clear();

    printConsole("消除左公因子...");
    BNFProcessor.eliminateLCommonFactor();
    printConsole("消除左公因子完成...");

    BNFProcessor.printGrammar(ui->plainTextEdit_leftCommonFactor);
    printConsole("输出处理结果...");

    ui->pushButton_set->setEnabled(true);
    ui->tabWidget->setCurrentIndex(leftCommonTab);
}

/**
 * @brief MainWindow::on_pushButton_set_clicked
 * 求解First集和Follow集
 */
void MainWindow::on_pushButton_set_clicked()
{
    BNFProcessor.firstNFollowSet();
    BNFProcessor.printSet(ui->tableWidget_firstSet, true);
    printConsole("First集计算完成");

    BNFProcessor.printSet(ui->tableWidget_followSet, false);
    printConsole("Follow集计算完成");

    ui->tabWidget->setCurrentIndex(firstSetTab);
    ui->pushButton_LL1->setEnabled(true);
}

void MainWindow::on_pushButton_clearConsole_clicked()
{
    ui->plainTextEdit_console->clear();
}

void MainWindow::on_pushButton_LL1_clicked()
{
    BNFProcessor.constructLL1ParsingTable();
    BNFProcessor.printLL1ParsingTable(ui->tableWidget_LL1);
    ui->tabWidget->setCurrentIndex(ll1TableTab);
    ui->pushButton_CST->setEnabled(true);
    printConsole("构建LL1表完成");
}

void MainWindow::on_pushButton_clearAll_clicked()
{
    BNFProcessor.init();
    m_grammarStr.clear();

    ui->plainTextEdit_edit->clear();
    ui->plainTextEdit_simplified->clear();
    ui->plainTextEdit_leftRecursion->clear();
    ui->plainTextEdit_leftCommonFactor->clear();
    ui->tableWidget_LL1->clear();
    ui->treeWidget_CST->clear();
    ui->tableWidget_firstSet->clear();
    ui->tableWidget_followSet->clear();
    ui->tableWidget_firstSet->setRowCount(0);
    ui->tableWidget_followSet->setRowCount(0);
    ui->tableWidget_LL1->setRowCount(0);
    ui->tableWidget_LL1->setColumnCount(0);

    ui->tabWidget->setCurrentIndex(grammarTab);

    ui->pushButton_eliminateLeftCommonFactor->setDisabled(true);
    ui->pushButton_eliminateLeftRecursion->setDisabled(true);
    ui->pushButton_set->setDisabled(true);
    ui->pushButton_LL1->setDisabled(true);
    ui->pushButton_CST->setDisabled(true);
    ui->pushButton_AST->setDisabled(true);
    printConsole("所有操作已复位");
}

/**
 * @brief MainWindow::on_pushButton_CST_clicked
 * 语法分析槽函数
 */
void MainWindow::on_pushButton_CST_clicked()
{
    QString language = ui->comboBox_language->currentText();
    QString srcProg = ui->plainTextEdit_CST->toPlainText();
    if (srcProg.isEmpty())
    {
        ui->tabWidget->setCurrentIndex(syntaxTab);
        QMessageBox::warning(nullptr, "提示", "请输入源程序再进行语法分析");
        return;
    }
    bool passedFlag = BNFProcessor.LL1Parsing(srcProg, ui->plainTextEdit_console, language);

    ui->label_language->setText(language);//显示语言
    ui->tabWidget->setCurrentIndex(syntaxTab);
    printConsole(language + "语言语法分析结束");
    if (!passedFlag)
        QMessageBox::warning(nullptr, "LL1分析", "源程序语法有误，详情请查看控制台输出");
    else
    {
        ui->tabWidget->setCurrentIndex(CSTTab);
        BNFProcessor.printParseTree(ui->treeWidget_CST);
        printConsole("输出" + language + "分析树");
        ui->pushButton_AST->setEnabled(true);
    }
}

/**
 * @brief MainWindow::on_pushButton_AST_clicked
 * 语法树生成（调试）
 */
void MainWindow::on_pushButton_AST_clicked()
{
    QString language = ui->comboBox_language->currentText();
    BNFProcessor.printAST(ui->treeWidget_AST, language);
    printConsole("输出" + language + "语法树");
    ui->tabWidget->setCurrentIndex(ASTTab);
}
