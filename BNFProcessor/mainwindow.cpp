/*====================================================

  Copyright(C) 2022-2023 Lyuyk
  All rights reserved

  文件名称:mainwindow.cpp
  摘要:主窗口源文件
  当前版本号:v1.1
  版本历史信息：v1.0：基本功能完成，有穷自动机转换部分存在问题


  created by Lyuyk at 11/3/2022

=====================================================*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ndfa.h"

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

void MainWindow::outConsole(QString content)
{
    ui->plainTextEdit_console->insertPlainText(getTime()+content+'\n');
}

QString MainWindow::getTime()
{
    QDateTime dateTime=QDateTime::currentDateTime();
    QString timeStamp="["+dateTime.toString("hh:mm:ss.zzz")+"] ";
    return timeStamp;
}

bool MainWindow::isTerminator(QChar c)
{
    return c.isLower() && c.isLetter();
}

bool MainWindow::isNonTerminator(QChar c)
{
    return c.isUpper() && c.isLetter();
}

/**
 * @brief MainWindow::isProductionR_terminable
 * 判断右部是否可终结
 * @param nonTmrSet
 * @param productionR
 * @return
 */
bool MainWindow::isProductionR_terminable(const QSet<QChar> &nonTmrSet, const QString &productionR)
{
    for(const auto c: productionR)
    {
        if(isNonTerminator(c) and not nonTmrSet.contains(c)) return false;
    }
    return true;
}

bool MainWindow::isProductionR_reachable(const QSet<QChar> &nonTmrSet, const QSet<QChar> &tmrSet, const QString &productionR)
{
    for(const auto &c: productionR)
    {
        if((isNonTerminator(c) and not nonTmrSet.contains(c)) or
                (isTerminator(c) and not tmrSet.contains(c)))
        {
            return false;
        }
    }
    return true;
}

void MainWindow::initGrammar()
{
    startChar='\0';
    terminatorSet.clear();
    non_terminatorSet.clear();
    GM_productionMap.clear();

    QString grammarString=ui->plainTextEdit_edit->toPlainText();
    QStringList productions = grammarString.split("\n");

    startChar=productions[0].at(0);

    for(const auto &subProduction: qAsConst(productions))
    {
        if(subProduction.simplified().isEmpty())continue;//略去空行

        QStringList productionContent = subProduction.split("->");//分离左右部
        QChar productionL = productionContent[0].simplified().at(0);//左部
        QStringList productionR_list=productionContent.at(1).split("|");//右部

        for(const auto &productionR: qAsConst(productionR_list))
        {
            for(const auto&ch: productionR)
            {
                if(isTerminator(ch))
                {
                    terminatorSet.insert(ch);
                }
            }

            if(productionR.simplified()=="@")
            {
                GM_productionMap[productionL].insert("");
            }
            else
            {
                GM_productionMap[productionL].insert(productionR.simplified());
            }
        }
    }

    simplifyGrammar();
}

void MainWindow::simplifyGrammar()
{
    //1.消除形如U->U等有害规则
    const QList<QChar> &tmp_GM_productionMap_keys=GM_productionMap.keys();
    for(const auto &tmp_productionL: tmp_GM_productionMap_keys)
    {
        GM_productionMap[tmp_productionL].remove(tmp_productionL);
    }

    //2.消除不可终结的无用符号以及无用产生式
    QSet<QChar> tmp_nonTmrSet;
    QMap<QChar, QSet<QString>> tmp_productions;

    qsizetype preNonTmrSetSize=tmp_nonTmrSet.size();
    qsizetype curNonTmrSetSize=std::numeric_limits<qsizetype>::max();

    while(preNonTmrSetSize != curNonTmrSetSize)
    {
        preNonTmrSetSize=curNonTmrSetSize;

        const QList<QChar> &tmp_GM_productionMap_keys=GM_productionMap.keys();
        for(const auto &tmp_productionL: tmp_GM_productionMap_keys)
        {
            if(tmp_nonTmrSet.contains(tmp_productionL)) continue;

            for(const auto &tmp_productionR: qAsConst(GM_productionMap[tmp_productionL]))
            {
                if(isProductionR_terminable(tmp_nonTmrSet, tmp_productionR))
                {
                    tmp_nonTmrSet.insert(tmp_productionL);//有一个产生式符合就结束判断
                    break;
                }
            }
        }

        curNonTmrSetSize=tmp_nonTmrSet.size();//更新当前非终结符数量
    }

    //4.遍历产生式，对于每一条产生式，如果它的右部是可以终结的，就将该条产生式加入产生式集合映射中；
    for(const auto &tmp_productionL: qAsConst(tmp_nonTmrSet))
    {
        for(const auto &tmp_productionR: GM_productionMap.value(tmp_productionL))
        {
            if(isProductionR_terminable(tmp_nonTmrSet, tmp_productionR))
            {
                tmp_productions[tmp_productionL].insert(tmp_productionR);
            }
        }
    }

    //5.更新非终结符集合与产生式集合；
    non_terminatorSet=tmp_nonTmrSet;
    GM_productionMap=tmp_productions;

    //c.消除不可达的无用符号以及无用产生式；
    tmp_nonTmrSet.clear();
    tmp_productions.clear();
    QSet<QChar> tmp_tmrSet;
    tmp_nonTmrSet.insert(startChar);

    preNonTmrSetSize=tmp_nonTmrSet.size();
    curNonTmrSetSize=std::numeric_limits<qsizetype>::max();
    qsizetype curTmrSetSize=std::numeric_limits<qsizetype>::max();
    qsizetype preTmrSetSize=tmp_tmrSet.size();


    while(preNonTmrSetSize != curNonTmrSetSize ||
          preTmrSetSize != curTmrSetSize)
    {
        preNonTmrSetSize=curNonTmrSetSize;
        preTmrSetSize=curTmrSetSize;

        const QList<QChar> &tmp_GM_productionMap_keys=GM_productionMap.keys();
        for(const auto &tmp_productionL: tmp_GM_productionMap_keys)
        {
            if(!non_terminatorSet.contains(tmp_productionL))continue;

            for(const auto &tmp_productionR: qAsConst(GM_productionMap[tmp_productionL]))
            {
                for(const auto &c: tmp_productionR)
                {
                    if(isNonTerminator(c))
                    {
                        tmp_nonTmrSet.insert(c);
                    }
                    else
                    {
                        tmp_tmrSet.insert(c);
                    }
                }
            }

            curNonTmrSetSize=tmp_nonTmrSet.size();
            curTmrSetSize=tmp_tmrSet.size();
        }

        //4.
        for(const auto &tmp_productionL: qAsConst(tmp_nonTmrSet))
        {
            for(const auto &tmp_productionR: GM_productionMap.value(tmp_productionL))
            {
                if(isProductionR_reachable(tmp_nonTmrSet, tmp_tmrSet, tmp_productionR))
                {
                    tmp_productions[tmp_productionL].insert(tmp_productionR);
                }
            }
        }

        GM_productionMap=tmp_productions;
        non_terminatorSet=tmp_nonTmrSet;
        terminatorSet=tmp_tmrSet;
    }

}

QString MainWindow::getGrammarString()
{
    QString grammarString;

    const QList<QChar> &tmp_GM_productionMap_keys=GM_productionMap.keys();
    for(const QChar &tmp_productionL: tmp_GM_productionMap_keys)
    {
        QString tmp_productionRStr;
        for(const QString &tmp_productionR: qAsConst(GM_productionMap[tmp_productionL]))
        {
            if(tmp_productionR.isEmpty())
            {
                tmp_productionRStr += "| @ ";
            }
            else
            {
                tmp_productionRStr += " | " + tmp_productionR;
            }
        }
        QString pLine=QString(tmp_productionL)+" -> "+tmp_productionRStr.remove(0,2);//字符串处理去掉首个右部'|'
        grammarString += (pLine+'\n');
    }

    return grammarString;
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

QSet<QString> MainWindow::replaceL(const QChar &replaceC, const QString &productionR)
{
    QSet<QString> resSet;

    for(const auto &replaceCProductionR: GM_productionMap[replaceC])
    {
        QString replaced=productionR;
        replaced.replace(0,1,replaceCProductionR);
        resSet.insert(replaced);
    }
    return resSet;
}

QChar MainWindow::getNewTmr()
{
    QSet<QChar> tmp_availableNonTmrSet;
    for(char c='A';c<='Z';c++)
    {
        tmp_availableNonTmrSet.insert(c);
    }
    tmp_availableNonTmrSet -= non_terminatorSet;
    if(tmp_availableNonTmrSet.isEmpty())
    {
        qDebug()<<"[文法化简]:非终结符超限，申请失败";

        //throw QString{ "[文法化简]:非终结符超限，申请失败" };
    }

    return *non_terminatorSet.insert(*tmp_availableNonTmrSet.begin());
}

void MainWindow::eliminateLCommonFactor()
{
    QStack<QChar> tmp_nonTmrStk;

    for(const auto &tmp_nonTmr: GM_productionMap.keys())
    {
        tmp_nonTmrStk.push_back(tmp_nonTmr);
    }
    while(!tmp_nonTmrStk.isEmpty())
    {
        QChar tmp_productionL=tmp_nonTmrStk.pop();

        QSet<QString> tmp_replaceResSet;
        for(const auto &tmp_productionR: GM_productionMap[tmp_productionL])
        {
            tmp_replaceResSet |= replaceLNonTmr(tmp_productionR);
        }

        GM_productionMap.remove(tmp_productionL);

        if(tmp_replaceResSet.contains(""))
            GM_productionMap[tmp_productionL].insert("");

        QSet<QChar> commonPrefixes=getRPPrefixes(tmp_replaceResSet);

        for(const auto &commonPrefix: commonPrefixes)
        {
            QSet<QString> suffixes=findPrefixProductions(commonPrefix,tmp_replaceResSet);

            if(suffixes.size()>1)
            {
                QChar newNonTmr=getNewTmr();

                GM_productionMap[newNonTmr]=suffixes;

                GM_productionMap[tmp_productionL].insert(QString(commonPrefix)+newNonTmr);

                tmp_nonTmrStk.push_back(newNonTmr);
            }
            else
            {
                GM_productionMap[tmp_productionL].insert(commonPrefix + *suffixes.begin());
            }
        }
    }

    simplifyGrammar();
}

QSet<QString> MainWindow::replaceLNonTmr(const QString &productionR)
{
    if(productionR.isEmpty() || isTerminator(productionR[0]))
    {
        return {productionR};
    }

    QSet<QString> resSet;

    QStack<QString> tmpStk;
    tmpStk.push_back(productionR);

    //推导次数限制
    short count=4;
    qDebug()<<'-';
    while(!tmpStk.isEmpty())
    {
        qDebug()<<1;
        if(--count==0)
        {
            qDebug()<<"推导超限（4次），无法消除左公因子";
            throw QString{"推导超限（4次），无法消除左公因子"};

        }

        //带入栈中的数据处理
        QString curProduction=tmpStk.pop();
        QSet<QString> newResSet=replaceL(curProduction.at(0),curProduction);

        for(const auto &newProductionR: qAsConst(newResSet))
        {
            if(newProductionR.isEmpty() || isTerminator(newProductionR[0]))
            {
                resSet.insert(newProductionR);
            }
            else
            {
                tmpStk.push_back(newProductionR);
            }
        }
    }

    return resSet;
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

QMap<QChar, QSet<QChar> > MainWindow::getFollowSet()
{
    return followSetMap;
}

void MainWindow::firstNfollowSet()
{
    followSetMap.clear();
    firstSetMap.clear();
    QList<QPair<QChar, QChar>> pendingFollow;

    followSetMap[startChar].insert('$');

    for(const auto &tmp_productionL: non_terminatorSet)//非终结符
    {
        for(const auto &tmp_productionR: GM_productionMap[tmp_productionL])//对于右部每一条产生式
        {
            firstSetMap[tmp_productionL][tmp_productionR]=getFirstSet(tmp_productionR);

            for(int idx=0;idx<tmp_productionR.length();++idx)//对于每一条产生式逐个
            {
                if(isNonTerminator(tmp_productionR[idx]))//若为非终结符则开求
                {
                    QString tmp=tmp_productionR;
                    tmp.remove(0,idx+1);
                    QSet<QString> tmp_firstSet=getFirstSet(tmp);

                    //remove epsilon
                    for(const auto &tmr: tmp_firstSet)//后面的first集元素除了eps外全加进去
                    {
                        if(!tmr.isEmpty())
                            followSetMap[tmp_productionR[idx]] |= tmr[0];
                    }

                    if(tmp_firstSet.contains(""))
                    {
                        pendingFollow.push_back({tmp_productionL, tmp_productionR[idx]});
                    }
                }
            }
        }
    }

    bool finished=false;
    while(!finished)
    {
        finished=true;
        for(const auto &[from, to]: pendingFollow)
        {
            qsizetype size=followSetMap[to].size();
            followSetMap[to] |= followSetMap[from];

            if(size != followSetMap[to].size())
                finished=false;
        }
    }
}

int MainWindow::isLinearGrammar()
{
    int linearGrammarFlag=0;//设置标志位，0为未初始化，1为左线性文法，2为右线性文法，其它为非线性文法

    for(const auto&tmp_productionL: GM_productionMap.keys())
    {
        //检查左部是否为
        if(tmp_productionL.isUpper())
        {
            //检查右部
            for(const auto&tmp_productionR: GM_productionMap[tmp_productionL])
            {
                //若为空串
                if(tmp_productionR=="")continue;
                //若产生式长度为1（必须为终结符）
                if(tmp_productionR.length()==1)
                {
                    //终结符
                    if(tmp_productionR[0].isLower())continue;
                    else return 4;
                }
                else if(tmp_productionR.length()==2)
                {
                    /*这里开始检查是否左右线性文法*/
                    if(linearGrammarFlag==0)
                    {
                        if(tmp_productionR[0].isUpper() && tmp_productionR[1].isLower())
                        {/*左线性文法*/
                            linearGrammarFlag=1;
                        }
                        else if(tmp_productionR[0].isLower() && tmp_productionR[1].isUpper())
                        {/*右线性文法*/
                            linearGrammarFlag=2;
                        }
                        else return 5;
                    }
                    else if(linearGrammarFlag==1)
                    {
                        if(tmp_productionR[0].isUpper() && tmp_productionR[1].isLower())continue;
                        else return 6;
                    }
                    else
                    {
                        if(tmp_productionR[0].isLower() && tmp_productionR[1].isUpper())continue;
                        else return 7;
                    }
                }
                else return 8;
            }
        }
        else return 3;
    }

    return linearGrammarFlag;
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
        GrammarStr.append(line.trimmed());
        ui->plainTextEdit_edit->insertPlainText(line+'\n');//一行行显示
        GrammarStr+=line;//初始化GrammarStr
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

void MainWindow::on_pushButton_process_clicked()
{
    initGrammar();
    simplifyGrammar();
    printSimplifiedGrammar();
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
    eliminateLRecursion();
    printGrammar(*ui->plainTextEdit_leftRecursion);
    outConsole("消除左递归完成...");
    //消除左公因子
    eliminateLCommonFactor();
    printGrammar(*ui->plainTextEdit_leftCommonFactor);
    outConsole("消除左公因子完成...");

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

//void MainWindow::on_pushButton_isGrammarLinear_clicked()
//{
//    int result=isLinearGrammar();qDebug()<<"LinearResult:"<<result;;
//    switch (result)
//    {
//        case 1:
//        {/*左线性文法*/
//            NDFA NDFAProcessor;
//            NDFAProcessor.init();
//            NDFAProcessor.lGrammarToDFA(GM_productionMap,terminatorSet,non_terminatorSet,startChar);
//            NDFAProcessor.printDFA(ui->tableWidget_DFA);
//            NDFAProcessor.printNFA(ui->tableWidget_NFA);
//            outConsole("转换成DFA...");
//            ui->checkBox_lLinearGrammar->setEnabled(true);
//            ui->checkBox_linearGrammar->setEnabled(true);
//            ui->checkBox_linearGrammar->setChecked(true);
//            ui->checkBox_lLinearGrammar->setChecked(true);
//            break;
//        }
//        case 2:
//        {
//            /*右线性文法*/
//            NDFA NDFAProcessor;
//            NDFAProcessor.init();
//            NDFAProcessor.rGrammarToDFA(GM_productionMap,terminatorSet,non_terminatorSet,startChar);
//            outConsole("转换成DFA...");
//            NDFAProcessor.printNFA(ui->tableWidget_NFA);
//            NDFAProcessor.printDFA(ui->tableWidget_DFA);
//            ui->checkBox_rLinearGrammar->setEnabled(true);
//            ui->checkBox_linearGrammar->setEnabled(true);
//            ui->checkBox_linearGrammar->setChecked(true);
//            ui->checkBox_rLinearGrammar->setChecked(true);
//            break;
//        }
//        default:
//        {/*非线性文法*/
//            ui->checkBox_nonLinearGrammar->setEnabled(true);
//            ui->checkBox_nonLinearGrammar->setChecked(true);
//            break;
//        }
//    }
//}

void MainWindow::on_pushButton_clearConsole_clicked()
{
    ui->plainTextEdit_console->clear();
}
