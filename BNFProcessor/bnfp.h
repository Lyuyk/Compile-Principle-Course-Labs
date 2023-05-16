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

    void simplifyGrammar();

    void printSimplifiedGrammar();
    void printGrammar(QPlainTextEdit *e);

private:
    bool isTerminator(QChar c);
    bool isNonTerminator(QChar c);
    bool isProductionR_terminable(const QSet<QChar> &nonTmrSet, const QString &productionR);
    bool isProductionR_reachable(const QSet<QChar> &nonTmrSet, const QSet<QChar> &tmrSet, const QString &productionR);

    void initGrammar();

    QString getGrammarString();

    void eliminateLRecursion();
    // 将文法规则中replaceC为左部的产生式代入replaced做为左部的产生式中
    void replaceL(const QChar &replaceC, const QChar &replaced);
    // 将一个产生式右部中的第一个符号用replaceC的产生式替换
    QSet<QString> replaceL(const QChar &replaceC, const QString &productionR);
    QChar getNewTmr();

    void eliminateLCommonFactor();
    // 替换最左边的非终结符直到出现终结符为止
    QSet<QString> replaceLNonTmr(const QString &productionR);
    //获取产生式右部集合的前缀
    QSet<QChar> getRPPrefixes(const QSet<QString> &productionRSet);
    //寻找一个前缀的所有产生式的集合
    QSet<QString> findPrefixProductions(const QChar &prefix, const QSet<QString> &productionRSet);

    QSet<QString> getFirstSet(const QChar &nonTmr);//求非终结符的first集
    QSet<QString> getFirstSet(const QString &productionR);//求右部产生式first集
    QMap<QChar, QSet<QString>> getFirstSet();//取得非终结符的first集
    QMap<QChar, QSet<QChar>> getFollowSet();//取得非终结符的follow集
    void firstNfollowSet();//求解first与follow集合元素

private:
    QString GrammarStr;

    QChar startChar;//开始符号
    QSet<QChar> non_terminatorSet;//非终结符集合
    QSet<QChar> terminatorSet;//终结符集合
    QMap<QChar,QSet<QString>> GM_productionMap;//文法产生式

    QMap<QChar, QMap<QString, QSet<QString>>> firstSetMap;//first集合元素
    QMap<QChar, QSet<QChar>> followSetMap;//follow集合元素
};

#endif // BNFP_H
