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
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tabWidget->setCurrentIndex(0);

    //界面交互处理
    ui->pushButton_check->setDisabled(true);
    ui->pushButton_eliminateLeftCommonFactor->setDisabled(true);
    ui->pushButton_eliminateLeftRecursion->setDisabled(true);
    ui->pushButton_set->setDisabled(true);

    /*菜单栏与槽函数连接*/
    connect(ui->action_exit, &QAction::triggered, this, &MainWindow::exit);
    connect(ui->action_open, &QAction::triggered, this, &MainWindow::on_pushButton_open_clicked);
    connect(ui->action_save, &QAction::triggered, this, &MainWindow::on_pushButton_save_clicked);
    connect(ui->action_simplify, &QAction::triggered, this, &MainWindow::on_pushButton_process_clicked);
    connect(ui->action_check, &QAction::triggered, this, &MainWindow::on_pushButton_check_clicked);
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
    QString time=QDateTime::currentDateTime().toString("[ hh:mm:ss.zzz ]");

    ui->plainTextEdit_console->insertPlainText(time+content+'\n');
}









void MainWindow::printSimplifiedGrammar()
{
    ui->plainTextEdit_simplified->clear();
    ui->plainTextEdit_console->insertPlainText(getTime()+"输出化简文法...\n");

    printGrammar(*ui->plainTextEdit_simplified);

    ui->plainTextEdit_console->insertPlainText(getTime()+"输出化简文法完成...\n");
    ui->tabWidget->setCurrentIndex(1);
}

void MainWindow::printGrammar(QPlainTextEdit &e)
{
    e.insertPlainText(getGrammarString());
}

void MainWindow::eliminateLRecursion()
{
    //给非终结符规定顺序
    QList<QChar> tmp_nonTmrList;
    for(const auto &tmp_nonTmr: qAsConst(non_terminatorSet))
    {
        tmp_nonTmrList.push_back(tmp_nonTmr);
    }

    //按照该顺序遍历所有非终结符
    for(auto curChar{tmp_nonTmrList.begin()};curChar != tmp_nonTmrList.end(); curChar++)
    {
        for(auto preChar{tmp_nonTmrList.begin()};preChar != curChar; preChar++)
        {
            replaceL(*preChar, *curChar);
        }

        //消除直接左递归，定义不含左递归的产生式右部集合
        QSet<QString> tmp_nonLRecursionRSet;
        //包含左递归的产生式右部的集合
        QSet<QString> tmp_LRecursionRSet;

        for(const auto &tmp_productionR: qAsConst(GM_productionMap[*curChar]))
        {
            if(tmp_productionR.isEmpty() || *curChar != tmp_productionR[0])
            {
                tmp_nonLRecursionRSet.insert(tmp_productionR);
            }//去除左递归对应的符号
            else
            {
                QString tmpR=tmp_productionR;
                tmpR.remove(0,1);//去掉第一个字符
                tmp_LRecursionRSet.insert(tmpR);
            }
        }

        //若非终结符产生式不包含左递归则跳过判断
        if(tmp_LRecursionRSet.isEmpty()) continue;

        //否则申请新的非终结符
        QChar newNonTmr=getNewTmr();

        //对于 A-> Aa|b形式，先添加B->aB|ɛ 这样一条产生式
        for(const auto &tmp_productionR: tmp_LRecursionRSet)
        {
            GM_productionMap[newNonTmr].insert(tmp_productionR+newNonTmr);
        }
        GM_productionMap[newNonTmr].insert("");

        //移除原来的产生式"A->Aa|b"
        GM_productionMap.remove(*curChar);

        // 添加 A->bB这样一条产生式
        for(const auto &tmp_productionR: tmp_nonLRecursionRSet)
        {
            GM_productionMap[*curChar].insert(tmp_productionR+newNonTmr);
        }
    }


    simplifyGrammar();
}

void MainWindow::replaceL(const QChar &replaceC, const QChar &replaced)
{
    QSet<QString> tmp_replacedProductionRSet = GM_productionMap[replaced];

    GM_productionMap.remove(replaced);

    for(const auto &tmp_replaceProductionR: tmp_replacedProductionRSet)
    {
        if(tmp_replaceProductionR.isEmpty() || replaceC != tmp_replaceProductionR[0])
        {
            GM_productionMap[replaced].insert(tmp_replaceProductionR);
            continue;
        }

        QSet<QString> replacedResult=replaceL(replaceC,tmp_replaceProductionR);

        for(const auto &replacedC: qAsConst(replacedResult))
            GM_productionMap[replaced].insert(replacedC);
    }


}



QSet<QChar> MainWindow::getRPPrefixes(const QSet<QString> &productionRSet)
{
    QSet<QChar> prefixes;

    for(const auto &productionR: productionRSet)
    {
        if(productionR.isEmpty())continue;
        prefixes.insert(productionR[0]);
    }

    return prefixes;
}

QSet<QString> MainWindow::findPrefixProductions(const QChar &prefix, const QSet<QString> &productionRSet)
{
    QSet<QString> preProductionSet;

    for(const auto &productionR: productionRSet)
    {
        if(!productionR.isEmpty() && prefix==productionR[0])
        {
            QString tmp=productionR;
            tmp.remove(0,1);
            preProductionSet.insert(tmp);
        }
    }

    return preProductionSet;
}

QSet<QString> MainWindow::getFirstSet(const QChar &nonTmr)
{
    QSet<QString> firstSet;

    for(const auto &tmp_productionR: GM_productionMap[nonTmr])
    {
        firstSet |= getFirstSet(tmp_productionR);
    }

    return firstSet;
}

QSet<QString> MainWindow::getFirstSet(const QString &productionR)
{
    if(productionR.isEmpty())
        return {""};

    QSet<QString> firstSet;

    for(const auto &c: productionR)
    {
        if(isTerminator(c))
            return firstSet |= c;

        firstSet |= getFirstSet(c);

        if(!firstSet.contains(""))
            return firstSet;

        firstSet.remove("");
    }
    return firstSet |= "";
}

QMap<QChar, QSet<QString> > MainWindow::getFirstSet()
{
    QMap<QChar, QSet<QString>> resMap;

    for(const auto &tmp_productionL: firstSetMap.keys())
    {
        for(const auto &tmp_productionRSet: firstSetMap[tmp_productionL])
        {
            resMap[tmp_productionL] |= tmp_productionRSet;
        }
    }
    return resMap;
}




/**
 * @brief MainWindow::on_pushButton_open_clicked
 * 打开文法文件
 */
void MainWindow::on_pushButton_open_clicked()
{
    QString srcFilePath=QFileDialog::getOpenFileName(this,"选择文法文件（仅.txt类型）","/", "Files(*.txt)");
    QFile srcFile(srcFilePath);
    if(!srcFile.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QMessageBox::warning(NULL, "文件", "文件打开失败");
        return;
    }
    QTextStream textInput(&srcFile);
    textInput.setEncoding(QStringConverter::Utf8);//设置编码，防止中文乱码

    QString line;
    ui->plainTextEdit_console->insertPlainText(getTime()+"读取文法文件...\n");
    while(!textInput.atEnd())
    {
        line=textInput.readLine().toUtf8();//按行读取文件
        m_grammarStr.append(line.trimmed());
        ui->plainTextEdit_edit->insertPlainText(line+'\n');//一行行显示
        m_grammarStr+=line;//初始化GrammarStr
    }
    ui->plainTextEdit_console->insertPlainText(getTime()+"读取完成...\n");
    srcFile.close();
}

/**
 * @brief MainWindow::on_pushButton_save_clicked
 * 将编辑框中编辑好的文法文本保存到选择的路径中
 */
void MainWindow::on_pushButton_save_clicked()
{
    QString tgtFilePath=QFileDialog::getSaveFileName(this,"选择保存路径","/");
    QFile tgtFile(tgtFilePath);
    if(!tgtFile.open(QIODevice::ReadWrite|QIODevice::Text|QIODevice::Truncate))
    {
        QMessageBox::warning(NULL, "文件", "文件保存失败");
        return;
    }
    QTextStream outputFile(&tgtFile);
    QString tgStr=ui->plainTextEdit_edit->toPlainText();
    outputFile<<tgStr;
    tgtFile.close();
    QMessageBox::information(NULL,"文件(saveGrammarRule)","文件保存成功");
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
    BNFProcessor.initGrammar();
    BNFProcessor.simplifyGrammar();

    BNFProcessor.printSimplifiedGrammar(ui->plainTextEdit_simplified);
    on_pushButton_check_clicked();
    on_pushButton_set_clicked();
    //on_pushButton_isGrammarLinear_clicked();
}

/**
 * @brief MainWindow::on_pushButton_check_clicked
 * 检查是否有左公因子和左递归
 */
void MainWindow::on_pushButton_check_clicked()
{
    //消除左递归
    BNFProcessor.eliminateLRecursion();
    BNFProcessor.printGrammar(ui->plainTextEdit_leftRecursion);
    outConsole("消除左递归完成...");
    //消除左公因子
    BNFProcessor.eliminateLCommonFactor();
    BNFProcessor.printGrammar(*ui->plainTextEdit_leftCommonFactor);
    printConsole("消除左公因子完成...");

    //ui->pushButton_isGrammarLinear->setEnabled(true);

}

void MainWindow::on_pushButton_eliminateLeftRecursion_clicked()
{
    ui->plainTextEdit_leftRecursion->clear();

    outConsole("消除左递归...");
    eliminateLRecursion();
    ui->plainTextEdit_console->insertPlainText(getTime()+"消除左递归完成...\n");

    printGrammar(*ui->plainTextEdit_leftRecursion);
    ui->plainTextEdit_console->insertPlainText(getTime()+"输出处理结果...\n");

    ui->tabWidget->setCurrentIndex(2);
    ui->pushButton_eliminateLeftCommonFactor->setEnabled(true);
}

void MainWindow::on_pushButton_eliminateLeftCommonFactor_clicked()
{
    ui->plainTextEdit_leftCommonFactor->clear();

    ui->plainTextEdit_console->insertPlainText(getTime()+"消除左公因子...\n");
    eliminateLCommonFactor();
    ui->plainTextEdit_console->insertPlainText(getTime()+"消除左公因子完成...\n");

    printGrammar(*ui->plainTextEdit_leftCommonFactor);
    ui->plainTextEdit_console->insertPlainText(getTime()+"输出处理结果...\n");

    ui->tabWidget->setCurrentIndex(2);
}

void MainWindow::on_pushButton_set_clicked()
{
    firstNfollowSet();
    ui->tableWidget_firstSet->clear();
    ui->tableWidget_followSet->clear();

    ui->tableWidget_firstSet->setRowCount(firstSetMap.size());
    ui->tableWidget_followSet->setRowCount(followSetMap.size());

    /*followSet*/
    int rowN=0;
    for(auto c:followSetMap.keys())
    {
        ui->tableWidget_followSet->setItem(rowN,0,new QTableWidgetItem(c));
        QString setStr="";
        for(auto ch:followSetMap[c])
        {
            setStr+=", "+QString(ch);
        }
        setStr.remove(0,1);
        ui->tableWidget_followSet->setItem(rowN,1,new QTableWidgetItem(setStr));
        rowN++;
    }

    /*firstSet*/
    QMap<QChar, QSet<QString>> firstSetMap=getFirstSet();
    rowN=0;
    for(auto c:firstSetMap.keys())
    {
        ui->tableWidget_firstSet->setItem(rowN,0,new QTableWidgetItem(c));
        QString setStr="";
        for(auto ch:firstSetMap[c])
        {
            if(ch.isEmpty())
            {
                setStr+=", @ ";
                continue;
            }
            setStr+=", "+QString(ch);
        }
        setStr.remove(0,1);
        ui->tableWidget_firstSet->setItem(rowN,1,new QTableWidgetItem(setStr));
        rowN++;
    }

}

void MainWindow::on_pushButton_simplify_clicked()
{
    initGrammar();
    simplifyGrammar();
    printSimplifiedGrammar();
    ui->pushButton_check->setEnabled(true);
    ui->pushButton_eliminateLeftRecursion->setEnabled(true);
    ui->pushButton_set->setEnabled(true);

}


void MainWindow::on_pushButton_clearConsole_clicked()
{
    ui->plainTextEdit_console->clear();
}
