#ifndef BNFP_H
#define BNFP_H

#include <QPlainTextEdit>
#include <QString>
#include <QChar>
#include <QStack>
#include <QSet>
#include <QMap>

class BNFP
{
public:
    BNFP();
    void Init();//类初始化

    void InitGrammar(QString s);//文法初始化

    void SimplifyGrammar();//文法化简
    void EliminateLRecursion();//消除左递归
    void EliminateLCommonFactor();//消除左公共因子
    void FirstNfollowSet();//求解first与follow集合元素

    void printGrammar(QPlainTextEdit *e);//输出文法
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

private:
    QString m_grammarStr;//存储待处理文法字符串

    QString m_startChar;//开始符号
    QSet<QString> m_nonTmrSet;//非终结符集合
    QSet<QString> m_tmrSet;//终结符集合
    QMap<QString, QVector<QVector<QString>>> m_GM_productionMap;//文法产生式（左部->右部候选式vector[终结符/非终结符]

    QMap<QChar, QMap<QString, QSet<QString>>> m_firstSetMap;//first集合元素//todo
    QMap<QString, QSet<QString>> m_followSetMap;//follow集合元素
};

#endif // BNFP_H
