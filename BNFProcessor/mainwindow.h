/****************************************************
 * @Copyright © 2021-2023 Lyuyk. All rights reserved.
 *
 * @FileName: mainwindow.h
 * @Brief: 主窗口头文件
 * @Module Function:
 *
 * @Current Version: 1.3
 * @Author: Lyuyk
 * @Modifier: Lyuyk
 * @Finished Date: 2023/4/21
 *
 * @Version History: 1.1
 *                   1.2 完善了部分注释，提高了部分代码的可读性
 *                   1.3 对窗体新功能，函数做出相应的适配
 *
 ****************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QString>
#include <QChar>
#include <QSet>
#include <QMap>

#include "bnfp.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
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

    void on_pushButton_arrowChar_clicked();//箭头编辑按钮
    void on_pushButton_epsilonChar_clicked();//epsilon键编辑按钮槽函数
    void on_pushButton_orChar_clicked();//|键编辑按钮槽函数

    void on_pushButton_process_clicked(); // 全部处理

    void on_pushButton_simplify_clicked(); // 文法化简

    void on_pushButton_eliminateLeftRecursion_clicked(); // 消除左递归

    void on_pushButton_eliminateLeftCommonFactor_clicked(); // 消除左公因子

    void on_pushButton_set_clicked(); // 求解First集&Follow集

    void on_pushButton_LL1_clicked(); // 构造LL1分析表

    void on_pushButton_clearConsole_clicked(); // 清空输出台

    void on_pushButton_clearAll_clicked(); //复位槽函数

    void on_pushButton_CST_clicked(); // LL1分析并生成分析树

    void on_pushButton_AST_clicked(); // 构造语法树并显示

private:
    void exit();
    void printConsole(QString content); // 控制台输出

private:
    BNFP BNFProcessor; // BNF文法处理器类对象

    QString m_grammarStr; // 文法字符串
    QString tmp;

    enum tabs
    {
        grammarTab,
        simplifyTab,
        leftCurTab,
        leftCommonTab,
        firstSetTab,
        followSetTab,
        ll1TableTab,
        syntaxTab,
        CSTTab,
        ASTTab
    } tabIdx;

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
