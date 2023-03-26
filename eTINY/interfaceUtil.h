#ifndef INTERFACEUTIL_H
#define INTERFACEUTIL_H

#include <QPlainTextEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QObject>
#include <QFile>
#include <QString>

QString openFileToUi(QPlainTextEdit *e)
{
    QString srcFilePath=QFileDialog::getOpenFileName(NULL,QObject::tr("选择源文件"),"./", "Files(*.txt *.tny)");
    QFile srcFile(srcFilePath);
    if(!srcFile.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QMessageBox::warning(NULL, QObject::tr("文件"), QObject::tr("文件打开失败"));
        return "-1";
    }
    QTextStream textInput(&srcFile);
    textInput.setEncoding(QStringConverter::Utf8);//设置编码，防止中文乱码

    QString line;
    QString targetText;
    while(!textInput.atEnd())
    {
        line=textInput.readLine().toUtf8();//按行读取文件
        targetText.append(line.trimmed());//文末尾处理
        e->insertPlainText(line+'\n');//一行行显示
        targetText+=line;
    }
    srcFile.close();

    return targetText;
}

bool saveFileFromUi(QString targetStr)
{
    QString tgtFilePath=QFileDialog::getSaveFileName(NULL,QObject::tr("选择保存路径"),"./");
    QFile tgtFile(tgtFilePath);
    if(!tgtFile.open(QIODevice::ReadWrite|QIODevice::Text|QIODevice::Truncate))
    {

        return false;
    }
    QTextStream outputFile(&tgtFile);
    outputFile<<targetStr;
    tgtFile.close();
    return true;
}

void outConsole(QPlainTextEdit *c,QString s)
{
    QDateTime dateTime=QDateTime::currentDateTime();
    QString timeStamp="["+dateTime.toString("hh:mm:ss.zzz")+"] ";
    c->insertPlainText(timeStamp+s+'\n');
}

QString getTime()
{
    QDateTime dateTime=QDateTime::currentDateTime();
    QString timeStamp="["+dateTime.toString("hh:mm:ss.zzz")+"] ";
    return timeStamp;
}

#endif // INTERFACEUTIL_H
