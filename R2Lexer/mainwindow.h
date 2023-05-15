/****************************************************
 * @Copyright © 2021-2023 Lyuyk. All rights reserved.
 *
 * @FileName: mainwindow.h
 * @Brief: 主窗体头文件
 * @Module Function:
 *
 * @Current Version: 1.2
 * @Author: Lyuyk
 * @Modifier: Lyuyk
 * @Finished Date: 2023/3/3
 *
 * @Version History: 1.1
 *                   1.2 current version
 *
 ****************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <set>

#include "ndfa.h"

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

    void on_pushButton_clearConsole_clicked();

    void on_pushButton_Lexer_clicked();

private:
    void printConsole(QString str);

private:    
    Ui::MainWindow *ui;

    QString regexStr;//正则表达式字符串
    QString srcFilePath;//源程序路径
    QString tmpFilePath;//编码输出路径

    NDFA NDFAG;//FA自动机类

};
#endif // MAINWINDOW_H
