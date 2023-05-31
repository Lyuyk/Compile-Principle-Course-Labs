/****************************************************
 * @Copyright © 2021-2023 Lyuyk. All rights reserved.
 *
 * @FileName: bnfp.h
 * @Brief: BNF文法处理类头文件
 * @Module Function:
 *
 * @Current Version: 1.3
 * @Author: Lyuyk
 * @Modifier: Lyuyk
 * @Finished Date: 2023/5/16
 *
 * @Version History: 1.1
 *                   1.2 完善了部分注释，提高了部分代码的可读性
 *                   1.3 降低代码耦合性，增添了任务二要求的与LL(1)分析相关的功能
 *
 ****************************************************/
#ifndef BNFP_H
#define BNFP_H

#include <QPlainTextEdit>
#include <QTableWidget>
#include <QTreeWidget>
#include <QDateTime>
#include <QHeaderView>
#include <QQueue>
#include <QString>
#include <QChar>
#include <QStack>
#include <QSet>
#include <QMap>

//右部结构体
struct pdnR
{
    QList<QStringList> pdnRights;//产生式右部，QStringList存储每一条候选式，当中的QString为候选式的单词
    QSet<QString> firstSet;//产生式左部First集
    QSet<QString> followSet;//产生式右部Follow集
};

//分析树结构体，任意多叉树
struct parseTreeNode
{
    QString value;
    QList<parseTreeNode*> children;

    parseTreeNode(QString val):value(val){}
};

class BNFP
{
public:
    BNFP();
    void init();//类初始化

    void initGrammar(QString s);//文法初始化

    void simplifyGrammar();//文法化简
    void eliminateLRecursion();//消除左递归
    void eliminateLCommonFactor();//消除左公共因子
    void firstNFollowSet();//求解first与求解follow集合元素
    void constructLL1ParsingTable();//生成LL1分析表
    bool LL1Parsing(QString progStr,QPlainTextEdit *console,QString language);//使用LL1分析表进行语法分析//todo

    void printGrammar(QPlainTextEdit *e);//输出文法
    void printSet(QTableWidget *table,bool isFirst=true);//输出First/Follow集
    void printLL1ParsingTable(QTableWidget *table);//输出LL1分析表
    void printParseTree(QTreeWidget *t);//输出树

    QMap<QString, QSet<QString>> getFirstSet();//取得非终结符的first集
    QMap<QString, QSet<QString>> getFollowSet();//取得非终结符的follow集

private:        
    void eliminateLRecursion(int index,QSet<QString>& newNonTmrSet);//消除左递归子函数

    void lFactorCount(QList<QStringList> list,QStringList pdnR,int &count);//记录最长左公因子个数
    QString getNewTmr(QString curTmr);//申请新终结符
    QString findL(QMap<QString,QVector<QStringList>> newSet,QList<QStringList> Temp);

    void computeFirstSet();//求解first与
    void computeFollowSet();//求解follow集合元素

    void decodeLex(QString language);//对词法分析程序编码的lex文件进行解码

    void printInfo(QString content,QPlainTextEdit* e){e->appendPlainText(QDateTime::currentDateTime().toString("[hh:mm:ss.zzz] ")+content);}

    QTreeWidgetItem* getChildItem(parseTreeNode* parentNode,QTreeWidgetItem* parentItem);
private:
    QString m_grammarStr;//存储待处理文法字符串

    QString m_startChar;//开始符号
    QList<QString> m_nonTmrSet;//非终结符集合，Qt6中QList与QVector基本无异
    QSet<QString> m_tmrSet;//终结符集合
    QMap<QString, pdnR> m_GM_productionMap;//文法产生式（左部->右部候选式vector[终结符/非终结符]

    //QMap<QString, QSet<QString>> m_firstSetMap;//first集合元素//todo
    //QMap<QString, QSet<QString>> m_followSetMap;//follow集合元素

    QMap<QString, QMap<QString,QStringList>> m_LL1Table;//LL1分析表

    QString m_lexPrgStr;//存储词法分析程序编码的源程序串
    QList<QString> m_programCode;//用户输入的待分析的程序

    parseTreeNode *m_parseTreeRoot;
    QTreeWidgetItem *m_treeRoot;
};

#endif // BNFP_H
