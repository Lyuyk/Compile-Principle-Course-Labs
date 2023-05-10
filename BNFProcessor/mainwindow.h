#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QString>
#include <QChar>
#include <QSet>
#include <QMap>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_open_clicked();

    void on_pushButton_save_clicked();

    void on_pushButton_arrowChar_clicked();

    void on_pushButton_epsilonChar_clicked();

    void on_pushButton_orChar_clicked();

    void on_pushButton_process_clicked();

    void on_pushButton_check_clicked();

    void on_pushButton_eliminateLeftRecursion_clicked();

    void on_pushButton_eliminateLeftCommonFactor_clicked();

    void on_pushButton_set_clicked();

    void on_pushButton_simplify_clicked();

    void on_pushButton_clearConsole_clicked();

private:
    void exit();
    void outConsole(QString content);
    QString getTime();


private:
    bool isTerminator(QChar c);
    bool isNonTerminator(QChar c);
    bool isProductionR_terminable(const QSet<QChar> &nonTmrSet, const QString &productionR);
    bool isProductionR_reachable(const QSet<QChar> &nonTmrSet, const QSet<QChar> &tmrSet, const QString &productionR);

    void initGrammar();
    void simplifyGrammar();
    QString getGrammarString();
    void printSimplifiedGrammar();
    void printGrammar(QPlainTextEdit &e);

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

    int isLinearGrammar();//判断是否线性文法

private:
    QString GrammarStr;

    QChar startChar;//开始符号
    QSet<QChar> non_terminatorSet;//非终结符集合
    QSet<QChar> terminatorSet;//终结符集合
    QMap<QChar,QSet<QString>> GM_productionMap;//文法产生式

    QMap<QChar, QMap<QString, QSet<QString>>> firstSetMap;//first集合元素
    QMap<QChar, QSet<QChar>> followSetMap;//follow集合元素

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
