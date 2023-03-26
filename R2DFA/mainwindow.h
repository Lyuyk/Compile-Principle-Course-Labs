#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <set>

#include "DStructure.h"

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
    void exit();

    void on_pushButton_openRegexFile_clicked();

    void on_pushButton_saveRegexToFile_clicked();

    void on_pushButton_2NFA_clicked();

    void on_pushButton_2DFA_clicked();

    void on_pushButton_mDFA_clicked();

    void insert(QString &s, int n, QChar ch);
    void preProcess(QString &s);
    int priority(QChar ch);
    void add(NFANode *n1, NFANode *n2, QChar ch);
    void add(NFANode *n1, NFANode *n2);
    NFAGraph createNFA(int sum);    
    QString in2Suffix(QString s);
    NFAGraph strToNfa(QString s);

    QSet<int> e_closure(QSet<int> s);

    QSet<int> move_e_cloure(QSet<int> s, QChar ch);

    bool isEnd(NFAGraph n, QSet<int> s);

    int findSetNum(int count, int n);

    void on_pushButton_clearConsole_clicked();

private:
    Ui::MainWindow *ui;

    QString regexStr;//正则表达式字符串
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
#endif // MAINWINDOW_H
