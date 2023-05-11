/****************************************************
 * @Copyright © 2021-2023 Lyuyk. All rights reserved.
 *
 * @FileName: ndfa.h
 * @Brief: NFA、DFA图结构头文件
 * @Module Function:
 *
 * @Current Version: 1.2
 * @Author: Lyuyk
 * @Modifier: Lyuyk
 * @Finished Date: 2023/3/16
 *
 * @Version History: 1.1
 *                   1.2 current version
 *
 ****************************************************/
#ifndef NDFA_H
#define NDFA_H

#include<QList>
#include<QMap>
#include<QMessageBox>
#include<QQueue>
#include<QSet>
#include<QStack>
#include<QTableWidget>

#include<set>

#define ARR_MAX_NUM 512 //定义存储节点数组大小上限
#define DFA_NODE_EDGE_COUNT 16 //定义DFA节点的边数上限

class NDFA
{

public:
    //NFA节点结构体
    struct NFANode
    {
        int stateNum;//当前NFA节点状态（号）
        int tState;//通过非epsilon边转换到的状态号
        QChar val;//非epsilon的NFA状态弧上的值
        QSet<int> epsToSet;//状态号集合，即当前状态通过epsilon边转移到的状态的 状态号集合
    };

    //NFA子图结构体
    struct NFAGraph
    {
        NFANode* startNode;//NFA头指针
        NFANode* endNode;//NFA尾指针
    };

    //边结构体
    struct Edge
    {
        QChar input;//弧上的值
        int toState;//指向的状态号
    };

    //DFA节点结构体
    struct DFANode
    {
        int stateNum;//DFA状态号
        int edgeCount;//该DFA状态节点的出度（射出的弧数）
        QSet<int> em_closure_NFA;//NFA的ε-move()闭包
        Edge edges[DFA_NODE_EDGE_COUNT];//DFA状态上射出的弧/边
        bool isEnd;//是否终态

    };

    //DFA子图结构体
    struct DFAGraph
    {
        int startState;//DFA的初态
        QSet<int> endStates;//DFA的终态集合
        QSet<QChar> endCharSet;//DFA的终结符号集合
        int tranArr[ARR_MAX_NUM][26];//DFA的转移矩阵
        QMap<QString,QString> transMap;//DFA状态转移矩阵
    };

    //状态集结构体
    struct stateSet
    {
        QSet<int> DFAStateSet;//该状态集合的集合 包含的状态集合标号
        int toStateSetNum;//该状态集合可以转换到的 状态集的标号
    };

public:
    NDFA();
    void init();//初始化类

    NFAGraph strToNfa(QString s);

    void printNFA(QTableWidget *table);//输出NFA状态转换表
    void printDFA(QTableWidget *table);//输出DFA状态转换表
    void printMDFA(QTableWidget *table);//输出mDFA状态转换表

public:
    void insert(QString &s, int n, QChar ch);
    int priority(QChar ch);//正则表达式优先级判定
    QString in2Suffix(QString s);//中缀表达式转换为后缀表达式
    QString preProcess(QString str);//预处理

    NFAGraph createNFA(int sum);//新建一个NFA节点
    NFAGraph createNFA(QChar start, QChar end);//建立一个NFA节点
    NFAGraph createNFA(int start,int end);//建立一个从
    void add(NFANode *n1, NFANode *n2, QChar ch);//n1、n2节点间添加非eps边
    void add(NFANode *n1, NFANode *n2);//n1、n2节点间添加eps边

    QSet<int> e_closure(QSet<int> s); //求NFA的epsilon闭包
    QSet<int> move_e_cloure(QSet<int> s, QChar ch);//
    bool isEnd(NFAGraph n, QSet<int> s);
    int findSetNum(int count, int n);

    void reg2NFA();//正则表达式转换位NFA
    void NFA2DFA();//NFA转换为DFA
    void DFA2mDFA();//DFA的最小化

private:
    QSet<QString> reserveWords;//保留字集合

    QSet<QString> OpCharSet;//操作符集合
    QSet<int> dividedSet[ARR_MAX_NUM]; //划分出来的集合数组（最小化DFA时用到的）

    NFANode NFAStateArr[ARR_MAX_NUM];//NFA状态数组
    DFANode mDFANodeArr[ARR_MAX_NUM];//mDFA状态数组
    DFANode DFAStateArr[ARR_MAX_NUM];//DFA状态数组

    QSet<NFANode> NFASet;
    QSet<DFANode> DFASet;
    QSet<DFANode> mDFASet;

    NFAGraph NFAG;//NFA图
    DFAGraph DFAG;//NFA转换得的DFA图
    DFAGraph mDFAG;//最小化的DFA图

    int NFAStateNum;//NFA状态计数
    int DFAStateNum;//DFA状态计数
    int mDFAStateNum;//mDFA状态计数，亦是划分出来的集合数

    QMap<int, QChar> i2cMap;
    QMap<QChar, int> c2iMap;

    QMap<int, QString> i2sMap;
    QMap<QString, int> s2iMap;
};


#endif // NDFA_H
