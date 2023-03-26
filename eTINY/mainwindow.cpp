
#include "mainwindow.h"
#include "analyze.h"
#include "ui_mainwindow.h"
#include "interfaceUtil.h"

#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->action_openFIle,&QAction::triggered,this,&MainWindow::on_pushButton_open_clicked);
    connect(ui->action_closeFile,&QAction::triggered,this,&MainWindow::on_pushButton_closeSrc_clicked);
    connect(ui->action_saveFile,&QAction::triggered,this,&MainWindow::on_pushButton_save_clicked);
    connect(ui->action_analyseSrcProgram,&QAction::triggered,this,&MainWindow::on_pushButton_analyzeSrcCode_clicked);
    connect(ui->action_clearAnalysis,&QAction::triggered,this,&MainWindow::on_pushButton_clearConsole_clicked);
    connect(ui->action_openFIle,&QAction::triggered,this,&MainWindow::on_pushButton_analyzeSrcCode_clicked);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_open_clicked()
{
    srcCode=openFileToUi(ui->plainTextEdit_sourceCode);
    if(srcCode=="-1")
    {
        outConsole(ui->plainTextEdit_console,"文件打开失败");
    }
    else
        outConsole(ui->plainTextEdit_console,"文件打开成功");

}

void MainWindow::on_pushButton_closeSrc_clicked()
{
    srcCode.clear();
    ui->plainTextEdit_sourceCode->clear();

    outConsole(ui->plainTextEdit_console,"文件已关闭");
}

void MainWindow::on_pushButton_save_clicked()
{
    QString tgStr=ui->plainTextEdit_sourceCode->toPlainText();
    bool flag=(saveFileFromUi(tgStr));
    if(flag)
        QMessageBox::information(NULL,"文件","文件保存成功");
    else
        QMessageBox::warning(NULL, "文件", "文件保存失败");
}

void MainWindow::on_pushButton_clearConsole_clicked()
{
    ui->plainTextEdit_console->clear();
}

void MainWindow::on_pushButton_analyzeSrcCode_clicked()
{
    /* 初始化TINY语言分析器 */
    lineno=0;
    linepos=0;
    bufsize=0;
    EOF_flag=false;

    QFile sourceCodeFile {"tmp_sourceCode.tmp"};

    if(sourceCodeFile.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        sourceCodeFile.write(ui->plainTextEdit_sourceCode->toPlainText().toStdString().c_str());
        sourceCodeFile.close();
    }

    AnalyzeCode();

    QFile resultFile {"tmp_result.tmp"};

    if(resultFile.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        ui->plainTextEdit_analysis->setPlainText(resultFile.readAll());
        resultFile.close();
    }
    outConsole(ui->plainTextEdit_console,"分析完成");

    QFile::remove("tmp_sourceCode.tmp");
    QFile::remove("tmp_result.tmp");
}


void MainWindow::on_pushButton_clearAnalysis_clicked()
{
    ui->plainTextEdit_analysis->clear();
    outConsole(ui->plainTextEdit_console,"结果已清空");
}

