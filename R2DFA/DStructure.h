#ifndef DSTRUCTURE_H
#define DSTRUCTURE_H

#include<QList>
#include<QMap>
#include<QMessageBox>
#include<QQueue>
#include<QSet>
#include<QStack>

#include<set>

#define ARR_MAX_NUM 256
#define DFA_NODE_EDGE_COUNT 16
struct NFANode
{
    int stateNum;//当前NFA节点状态（号）
    int tState;//通过非epsilon边转换到的状态号
    QChar val;//非epsilon的NFA状态弧上的值
    QSet<int> eSUnion;//状态号集合，即当前状态通过epsilon边转移到的状态的 状态号集合

    //bool isEpsilon;//是否epsilon边
    //QSet<NFANode*> eNUnion;//状态号节点集合，即当前状态通过epsilon边转移到的状态的 状态号集合
    //bool isEnd;//是否终态
    //QMap<QChar, NFANode*> tMap;//状态转换表，即通过非epsilon边转换到的状态的 转换表
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

#endif // DSTRUCTURE_H
