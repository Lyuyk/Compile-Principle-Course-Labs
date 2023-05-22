/****************************************************
 * @Copyright © 2021-2023 Lyuyk. All rights reserved.
 *
 * @FileName: ndfa.h
 * @Brief: NFA、DFA图结构头文件
 * @Module Function:
 *
 * @Current Version: 1.3
 * @Author: Lyuyk
 * @Modifier: Lyuyk
 * @Finished Date: 2023/4/18
 *
 * @Version History: 1.1
 *                   1.2 部分结构体改动及优化
 *                   1.3 current version
 *
 ****************************************************/
#ifndef NDFA_H
#define NDFA_H

#include<QFileInfo>
#include<QHeaderView>
#include<QList>
#include<QMap>
#include<QMessageBox>
#include<QQueue>
#include<QSet>
#include<QStack>
#include<QTableWidget>
#include<QPlainTextEdit>

#include<set>

#define ARR_MAX_SIZE 1024 //定义存储节点数组大小上限
#define ARR_TEMP_SIZE 128 //定义临时结构体数组大小
#define DFA_NODE_EDGE_COUNT 16 //定义DFA节点的边数上限

class NDFA
{

public:
    //NFA节点结构体
    struct NFANode
    {
        int stateNum;//当前NFA节点状态（号）
        int toState;//通过非epsilon边转换到的状态号
        QString value;//非epsilon的NFA状态弧上的值
        QSet<int> epsToSet;//状态号集合，即当前状态通过epsilon边转移到的状态的 状态号集合

        void init()//初始化函数
        {
            stateNum=-1;
            toState=-1;
            value=' ';
            epsToSet.clear();
        }
    };

    //NFA子图结构体
    struct NFAGraph
    {
        NFANode* startNode;//NFA头指针
        NFANode* endNode;//NFA尾指针
    };

    //DFA节点结构体
    struct DFANode
    {
        int stateNum;//DFA状态号

        QSet<int> NFANodeSet;//存储当前DFA节点包含的NFA节点序号
        QMap<QString, int> DFAEdgeMap;//存储当前DFA节点的边——节点映射

        void init()
        {
            stateNum=-1;
            NFANodeSet.clear();
            DFAEdgeMap.clear();
        }
    };


    //mDFA节点结构体
    struct mDFANode
    {
        QSet<int> DFAStatesSet;//存储当前最小化DFA节点包含DFA节点序号的集合
        QMap<QString, int> mDFAEdgesMap;//当前最小化DFA节点通过某操作符到达最小化DFA节点序号

        void init()
        {
            DFAStatesSet.clear();
            mDFAEdgesMap.clear();
        }
    };

    struct mDFAGraph
    {
        int startState;//最小化DFA的初态
        QSet<int> endStateSet;//最小化DFA的终态集
    };

    //状态集结构体
    struct stateSet
    {
        QSet<int> DFAStateSet;//该状态集合的集合 包含的状态集合标号
        int stateSetId;//所属状态集合号
    };

public:
    NDFA();
    void init();//初始化类

    NFAGraph strToNfa(QString s);//将正则表达式转换为NFA
    void opPriorityMapInit();//初始化操作符优先级
    void insConnOp(QString str,int curState,QStack<QChar> &opStack,QStack<NFAGraph> &NFAStack);//判断是否需要插入连接&符号
    void pushOpStackProcess(QChar ch,QStack<QChar> &opStack,QStack<NFAGraph> &NFAStack);//运算符入栈处理子函数
    void opProcess(QChar ch,QStack<NFAGraph> &NFAStack);//根据运算符转换NFA处理子函数



    void printNFA(QTableWidget *table);//输出NFA状态转换表
    void printDFA(QTableWidget *table);//输出DFA状态转换表
    void printMDFA(QTableWidget *table);//输出mDFA状态转换表
    void printLexer(QPlainTextEdit *widget);//输出Lexer（词法分析程序）代码

public:
    NFAGraph createNFA(int sum);//按顺序新建一个NFA子图
    void add(NFANode *n1, NFANode *n2, QString ch);//n1、n2节点间添加非eps边
    void add(NFANode *n1, NFANode *n2);//n1、n2节点间添加eps边


    void reg2NFA(QString regStr);//正则表达式转换位NFA
    void NFA2DFA();//NFA转换为DFA
    void DFA2mDFA();//DFA的最小化
    QString mDFA2Lexer(QString filePath);//最小化DFA生成Lexer

public:
    void setPath(QString srcFilePath, QString tmpFilePath);
    void setKeywordStr(QString kStr);

private:
    void get_e_closure(QSet<int> &tmpSet);//求epsilon闭包

    int getStateId(QSet<int> set[],int cur);//查询当前DFA节点属于哪个状态集（号）

    bool genLexCase(QList<QString> tmpList, QString &codeStr, int idx, bool flag);

private:
    QString m_reg_keyword_str;//关键字正则串
    QString m_lexerCodeStr;//词法分析器代码

    QString m_srcFilePath;//词法分析程序路径
    QString m_tmpFilePath;//词法分析程序输出路径

    QSet<QString> m_keyWordSet;//关键字集合
    QSet<QString> m_opCharSet;//操作符集合
    QSet<QChar> m_opSet={'(',')','|','*','+','?'};//运算符集合

    QSet<int> m_DFAEndStateSet;//存储DFA终态状态号集合
    QSet<int> m_dividedSet[ARR_MAX_SIZE]; //划分出来的集合数组，存储DFA状态号集的数组（最小化DFA时用到的）

    QMap<QChar, int> opPriorityMap;//存储运算符优先级

    NFANode m_NFAStateArr[ARR_MAX_SIZE];//NFA状态数组
    DFANode m_DFAStateArr[ARR_MAX_SIZE];//DFA状态数组
    mDFANode m_mDFANodeArr[ARR_MAX_SIZE];//mDFA状态数组

    NFAGraph m_NFAG;//NFA图
    //DFAGraph DFAG;//NFA转换得的DFA图
    mDFAGraph m_mDFAG;//最小化的DFA图

    int m_NFAStateNum;//NFA状态下标计数（从0开始）
    int m_DFAStateNum;//DFA状态下标计数（从0开始）
    int m_mDFAStateNum;//mDFA状态数量计数，亦是划分出来的集合数（从1开始）

};


#endif // NDFA_H
