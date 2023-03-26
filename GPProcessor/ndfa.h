#ifndef NDFA_H
#define NDFA_H

#include <QList>
#include <QMap>
#include <QMessageBox>
#include <QQueue>
#include <QSet>
#include <QStack>
#include <QTableWidget>
#include <set>

#define ARR_MAX_NUM 256
#define DFA_NODE_EDGE_COUNT 16

class NDFA
{
public:
    struct NFANode
    {
        int stateNum;//当前NFA节点状态（号）
        int tState;//通过非epsilon边转换到的状态号
        QChar val;//非epsilon的NFA状态弧上的值
        QSet<int> eSUnion;//状态号集合，即当前状态通过epsilon边转移到的状态的 状态号集合
    };

    struct NFAGraph
    {
        NFANode* startNode;//NFA头指针
        NFANode* endNode;//NFA尾指针
    };

    struct Edge
    {
        QChar input;//弧上的值
        int tgtState;//指向的状态号
    };

    struct DFANode
    {
        int stateNum;//DFA状态号
        int edgeCount;//该DFA状态节点的出度（射出的弧数）
        QSet<int> em_closure_NFA;//NFA的ε-move()闭包
        Edge edges[DFA_NODE_EDGE_COUNT];//DFA状态上射出的弧/边
        bool isEnd;//是否终态

    };

    struct DFAGraph
    {
        int startState;//DFA的初态
        QSet<int> endStates;//DFA的终态集合
        QSet<QChar> endCharSet;//DFA的终结符号集合
        int tranArr[ARR_MAX_NUM][26];//DFA的转移矩阵
    };

    struct stateSet
    {
        QSet<int> DFAStateSet;//该状态集合的集合 包含的状态集合标号
        int stateSetNum;//该状态集合可以转换到的 状态集的标号
    };

public:
    NDFA();
    void init();
    void lGrammarToDFA(QMap<QChar,QSet<QString>> productionMap,QSet<QChar> tmrSet,QSet<QChar> nonTmrSet,QChar start);
    void rGrammarToDFA(QMap<QChar,QSet<QString>> productionMap,QSet<QChar> tmrSet,QSet<QChar> nonTmrSet,QChar start);
    void printLinearGrammar(QTableWidget *table);
    void printNFA(QTableWidget *table);
    void printDFA(QTableWidget *table);

protected:
    NFAGraph createNFA(QChar start, QChar end);
    NFAGraph createNFA(int start,int end);
    void add(NFANode *n1, NFANode *n2, QChar ch);
    void add(NFANode *n1, NFANode *n2);

    QSet<int> e_closure(QSet<int> s);
    QSet<int> move_e_cloure(QSet<int> s, QChar ch);
    bool isEnd(NFAGraph n, QSet<int> s);
    int findSetNum(int count, int n);

private:
    void NFA2DFA();
    void DFA2mDFA();

private:
    QMap<int, QChar> i2cMap;
    QMap<QChar, int> c2iMap;
    QSet<QString> OpCharSet;//操作符集合
    QSet<int> dividedSet[ARR_MAX_NUM]; //划分出来的集合数组（最小化DFA时用到的）

    NFANode NFAStateArr[ARR_MAX_NUM];//NFA状态数组
    DFANode mDFANodeArr[ARR_MAX_NUM];//mDFA状态数组
    DFANode DFAStateArr[ARR_MAX_NUM];//DFA状态数组

    NFAGraph NFAG;//NFA图
    DFAGraph DFAG;//NFA转换得的DFA图
    DFAGraph mDFAG;//最小化的DFA图

    int NFAStateNum;//NFA状态计数
    int DFAStateNum;//DFA状态计数
    int mDFAStateNum;//mDFA状态计数，亦是划分出来的集合数
};

#endif // NDFA_H
