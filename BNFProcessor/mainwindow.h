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
    void printConsole(QString content);//控制台输出

private:
    BNFP BNFProcessor;//BNF文法处理器类对象

    QString m_grammarStr;//文法字符串

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
