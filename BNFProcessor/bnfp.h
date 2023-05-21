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

class BNFP
{
public:
    BNFP();
    void init();//类初始化

    void initGrammar(QString s);//文法初始化

    void simplifyGrammar();//文法化简
    void eliminateLRecursion();//消除左递归
    void eliminateLCommonFactor();//消除左公共因子
    void firstNfollowSet();//求解first与follow集合元素
    void constructLL1ParsingTable();//生成LL1分析表

    void printGrammar(QPlainTextEdit *e);//输出文法
    void printLL1ParsingTable(QTableWidget *table);//输出LL1分析表

    QMap<QChar, QSet<QString>> getFirstSet();//取得非终结符的first集
    QMap<QChar, QSet<QChar>> getFollowSet();//取得非终结符的follow集

private:    
    bool isTerminator(QString s);//是否终结符
    bool isNonTerminator(QString s);//是否非终结符
    bool isProductionR_terminable(const QSet<QString> &nonTmrSet, const QVector<QString> &productionR);//右部是否可终止
    bool isProductionR_reachable(const QSet<QString> &nonTmrSet, const QSet<QString> &tmrSet, const QVector<QString> &productionR);//右部是否可达

    // 将文法规则中replaceC为左部的产生式代入replaced做为左部的产生式中
    void replaceL(const QChar &replaceC, const QChar &replaced);
    // 将一个产生式右部中的第一个符号用replaceC的产生式替换
    QSet<QString> replaceL(const QChar &replaceC, const QString &productionR);
    QChar getNewTmr();//申请新终结符


    // 替换最左边的非终结符直到出现终结符为止
    QSet<QString> replaceLNonTmr(const QString &productionR);
    //获取产生式右部集合的前缀
    QSet<QChar> getRPPrefixes(const QSet<QString> &productionRSet);
    //寻找一个前缀的所有产生式的集合
    QSet<QString> findPrefixProductions(const QChar &prefix, const QSet<QString> &productionRSet);

    QSet<QString> getFirstSet(const QChar &nonTmr);//求非终结符的first集
    QSet<QString> getFirstSet(const QString &productionR);//求右部产生式first集

    void decode(QString s);//对编码lex文件进行解码

private:
    QString m_grammarStr;//存储待处理文法字符串

    QString m_startChar;//开始符号
    QSet<QString> m_nonTmrSet;//非终结符集合
    QSet<QString> m_tmrSet;//终结符集合
    QMap<QString, pdnR> m_GM_productionMap;//文法产生式（左部->右部候选式vector[终结符/非终结符]

    //QMap<QString, QSet<QString>> m_firstSetMap;//first集合元素//todo
    //QMap<QString, QSet<QString>> m_followSetMap;//follow集合元素

    QMap<QString, QMap<QString,QString>> m_LL1Table;//LL1分析表
    QList<QString> m_programCode;//用户输入的待分析的程序
};

#endif // BNFP_H
