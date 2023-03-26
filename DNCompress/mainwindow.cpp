#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dncompressorUtil.h"
#include <QFile>
#include <QString>
#include <QMessageBox>
#include <QTextStream>
#include <QFileDialog>
#include <QDebug>
#include <fstream>
#include <QRegularExpression>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->action_Openfile,SIGNAL(triggered()),this,SLOT(on_pushButton_openfile_clicked()));//打开文件
    connect(ui->action_Quit,SIGNAL(triggered()),this,SLOT(on_action_Quit_triggered()));//关闭窗口
    connect(ui->action_ClearAll,SIGNAL(triggered()),this,SLOT(on_pushButton_closefile_clicked()));//清空窗口
    connect(ui->action_ESavefile,SIGNAL(triggered()),this,SLOT(on_pushButton_eSaveTargetCode_clicked()));

    ui->pushButton_compress->setDisabled(1);
    ui->pushButton_decompress->setDisabled(1);
    ui->pushButton_eSaveTargetCode->setDisabled(1);


    //初始化两个对照表（单词替换表和符号替换表）
    KEYLIST_SIZE=KEYWORDS_LIST.size();
    PUNCTLIST_SIZE=PUNCT_LIST.size();
    for(int i=0;i<KEYLIST_SIZE;i++)//128~198
    {
        KEYWORDS_MAP.insert(KEYWORDS_LIST[i],(unsigned char)i+128);//初始化保留字替换表
        WORD_VALUES_MAP.insert((unsigned char)i+128,KEYWORDS_LIST[i]);
    }
    for(int i=0;i<PUNCTLIST_SIZE;i++)
    {
        PUNCT_MAP.insert(PUNCT_LIST[i],(unsigned char)i+200);
        PUNCT_VALUES_MAP.insert((unsigned char)i+200,PUNCT_LIST[i]);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_openfile_clicked()
{
    ui->srcCode_textEdit->clear();
    ui->compressCode_textEdit->clear();
    ui->console_textEdit->clear();
    srcCodeList.clear();//清空列表
    ui->pushButton_compress->setDisabled(1);
    ui->pushButton_decompress->setDisabled(1);
    ui->pushButton_eSaveTargetCode->setDisabled(1);

    QString srcFilePath=QFileDialog::getOpenFileName(this,"选择文件（仅.cpp .h类型）","/", "Files(*.h *.cpp)");
    QFile srcFile(srcFilePath);
    if(!srcFile.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QMessageBox::warning(NULL, "文件", "文件打开失败");
        return;
    }
    QTextStream textInput(&srcFile);
    textInput.setEncoding(QStringConverter::Utf8);//设置编码，防止中文乱码

    QString line;
    while(!textInput.atEnd())
    {
        line=textInput.readLine().toUtf8();//按行读取文件
        srcCodeList.append(line.trimmed());
        ui->srcCode_textEdit->insertPlainText(line+'\n');//一行行显示
    }

    srcCodeList.removeAll("");//清除所有的空行
    for (int i=0;i<srcCodeList.size();i++)
    {
        srcCodeList[i]=srcCodeList[i].simplified();//预处理，去掉首尾空格以及中间多余空格（中间多于两个空格只保留一个）
        srcCodeList[i]=srcCodeList[i].replace(QRegularExpression("# *include"),"#include"); //不考虑这些情况
        srcCodeList[i]=srcCodeList[i].replace(QRegularExpression("# *define"),"#define");
        srcCodeList[i]=srcCodeList[i].replace(QRegularExpression("# *ifdef"),"#ifdef"); //不考虑这些情况
        srcCodeList[i]=srcCodeList[i].replace(QRegularExpression("# *ifndef"),"#ifndef");
        srcCodeList[i]=srcCodeList[i].replace(QRegularExpression("# *error"),"#error"); //不考虑这些情况
        srcCodeList[i]=srcCodeList[i].replace(QRegularExpression("# *if"),"#if");
        ui->console_textEdit->insertPlainText(srcCodeList[i]+'\n');//输出显示于控制台中
        //srcCodeList[i]=srcCodeList[i].replace(QRegularExpression("# *include"),"#include"); //不考虑这些情况
    }
    ui->tabWidget->setCurrentIndex(2);
    ui->pushButton_compress->setEnabled(1);
}

/**
 * @brief MainWindow::on_action_Quit_triggered
 * 关闭窗口
 */
void MainWindow::on_action_Quit_triggered()
{
    window()->close();
}

/**
 * @brief MainWindow::on_pushButton_closefile_clicked
 * 关闭打开的源文件（包括清空各个输出窗口）
 */
void MainWindow::on_pushButton_closefile_clicked()
{
    ui->srcCode_textEdit->clear();
    srcCodeList.clear();
    ui->compressCode_textEdit->clear();
    ui->decompressCode_textEdit->clear();
    ui->console_textEdit->clear();
    ui->lineEdit_compress->clear();
    ui->lineEdit_decompress->clear();
    ui->pushButton_compress->setDisabled(1);
    ui->pushButton_decompress->setDisabled(1);
    ui->pushButton_eSaveTargetCode->setDisabled(1);
    ui->tabWidget->setCurrentIndex(0);
}

/**
 * @brief MainWindow::on_pushButton_compress_clicked
 * 压缩源代码文件，压缩得到的二进制串会显示在窗口中，同时在本地会保存一个压缩文件
 */
void MainWindow::on_pushButton_compress_clicked()
{
    ui->compressCode_textEdit->clear();//清空之前的输出
    srcCodeList.clear();//清空全局代码串
    QStringList srcCodeList = ui->srcCode_textEdit->toPlainText().split("\n");

    QString srcCode;
    for(QString &qstr:srcCodeList)
    {
        qstr=qstr.replace(QRegularExpression("# *include"),"#include"); //不考虑这些情况
        qstr=qstr.replace(QRegularExpression("# *define"),"#define");
        qstr=qstr.replace(QRegularExpression("# *ifdef"),"#ifdef"); //不考虑这些情况
        qstr=qstr.replace(QRegularExpression("# *ifndef"),"#ifndef");
        qstr=qstr.replace(QRegularExpression("# *error"),"#error"); //不考虑这些情况
        qstr=qstr.replace(QRegularExpression("# *if"),"#if");
        srcCode+=qstr.simplified()+'\n'; //换行符用于一些情况的判断
    }

    std::fstream outFile;
    outFile.open(".\\compress.bin",std::ios::binary|std::ios::out|std::ios::ate);
    char c;
    unsigned char code;
    std::string tmpStr;
    int curStrIdx=0;
    ui->console_textEdit->insertPlainText("srcCode size:"+QString::number(srcCode.length())+'\n');


    while(true)
    {
        //qDebug()<<curStrIdx<<Qt::endl;
        if(curStrIdx>=srcCode.length()-1) break;//若读完则结束分析
        c=srcCode.at(curStrIdx).toLatin1();curStrIdx++;
        switch (c)
        {
            case ',': code=200, outFile<<code;break;
            case ';': code=201, outFile<<code;break;
            case '{': code=202, outFile<<code;break;
            case '}': code=203, outFile<<code;break;
            case '(': code=204, outFile<<code;break;
            case ')': code=205, outFile<<code;break;
            case ':':
            {
                c=srcCode.at(curStrIdx).toLatin1();curStrIdx++;
                switch (c)
                {
                    case ':': // ::情况
                    {
                        curStrIdx++;
                        code=206,outFile<<code;
                        break;
                    }
                    default: // : 情况
                    {
                        code = 207, outFile<<code;
                        break;
                    }
                }
                break;
            }
            case '\''://（若读到"'",处理字符常量'c'）
            {
                code = 254;
                c = srcCode.at(curStrIdx).toLatin1();curStrIdx++;
                if(c=='\\')//如果是转义字符
                {
                    outFile<<code<<(unsigned char)246<<c<<(unsigned char)255;
                }
                else //如果不是转义字符
                {
                    outFile<<code<<c<<(unsigned char)255;
                }
                curStrIdx++;//处理字符常量后面的'号
                break;
            }
            case '\"':
            {
                code = 247;
                tmpStr = "";
                char preCh=c;//排除字符串中被转义的"号
                c = srcCode.at(curStrIdx).toLatin1();curStrIdx++;
                while(!(preCh!='\\' && c== '\"'))
                {
                    tmpStr += c;
                    preCh = c;
                    c = srcCode.at(curStrIdx).toLatin1();curStrIdx++;
                }
                outFile << code << tmpStr << (unsigned char)248;//checkthis========================
                break;
            }
            case '/':
            {
                c = srcCode.at(curStrIdx).toLatin1();//看一下下一个字符是什么来判断，不需要自增
                switch(c)
                {
                    case '/': //单行注释的情况，则其后面的直接去掉直到出现换行
                    {
                        c = srcCode.at(curStrIdx).toLatin1();curStrIdx++;
                        while(c!='\n')//直到本行结束
                        {
                            c = srcCode.at(curStrIdx).toLatin1();curStrIdx++;
                        }
                        break;
                    }
                    case '=': // 处理/=除运算情况
                    {
                        curStrIdx++;
                        code = 208;
                        outFile << code;
                        break;
                    }
                    case '*': // 处理多行注释情况
                    {
                        curStrIdx++;
                        char preCh;
                        c = srcCode.at(curStrIdx).toLatin1();curStrIdx++;
                        while(true)
                        {
                            preCh =c;
                            c = srcCode.at(curStrIdx).toLatin1();curStrIdx++;
                            if(preCh=='*' && c=='/')
                                break;
                        }
                        break;
                    }
                    default: //除号/情况处理
                    {
                        code=209;
                        outFile<<code;
                        break;
                    }
                }
                break;
            }
            case '+': //这里处理 +运算 ++自增 +=运算
            {
                c = srcCode.at(curStrIdx).toLatin1();//仅读取下一个字符作为判断
                switch(c)
                {
                    case '+': // ++运算
                    {
                        curStrIdx++;
                        code=210;
                        outFile<<code;
                        break;
                    }
                    case '=': // +=运算
                    {
                        curStrIdx++;
                        code=211;
                        outFile<<code;
                        break;
                    }
                    default:
                    {
                            code = 212;
                            outFile<<code;
                            break;
                    }
                }
                break;
            }
            case '-':
            {
                c = srcCode.at(curStrIdx).toLatin1();
                switch(c)
                {
                    case '-': //自减--
                    {
                        curStrIdx++;
                        code=213;
                        outFile<<code;
                        break;
                    }
                    case '=':
                    {
                        curStrIdx++;
                        code=214;
                        outFile<<code;
                        break;
                    }
                    case '>': // ->
                    {
                        curStrIdx++;
                        code=215;
                        outFile<<code;
                        break;
                    }
                    default: // 减号
                    {
                        code=216;
                        outFile<<code;
                        break;
                    }
                }
                break;
            }
            case '*':
            {
                c = srcCode.at(curStrIdx).toLatin1();
                switch(c)
                {
                    case '=': // *=情况
                    {
                        curStrIdx++;
                        code=217;
                        outFile<<code;
                        break;
                    }
                    default:
                    {
                        code =218;
                        outFile<<code;
                        break;
                    }
                }
                break;
            }
            case '%':
            {
                c = srcCode.at(curStrIdx).toLatin1();
                switch(c)
                {
                    case '=': // %=
                    {
                        curStrIdx++;
                        code=219;
                        outFile<<code;
                        break;
                    }
                    default: // %
                    {
                        code=220;
                        outFile<<code;
                        break;
                    }
                }
                break;
            }
            case '=':
            {
                c = srcCode.at(curStrIdx).toLatin1();
                switch(c)
                {
                    case '=':
                    {
                        curStrIdx++;
                        code=221;
                        outFile<<code;
                        break;
                    }
                    default:// = only
                    {
                        code=222;
                        outFile<<code;
                        break;
                    }
                }
                break;
            }
            case '!':
            {
                c = srcCode.at(curStrIdx).toLatin1();
                switch(c)
                {
                    case '=':// !=
                    {
                        curStrIdx++;
                        code=223;
                        outFile<<code;
                        break;
                    }
                    default:// !
                    {
                        code=224;
                        outFile<<code;
                        break;
                    }
                }
                break;
            }
            case '&':
            {
                c = srcCode.at(curStrIdx).toLatin1();
                switch(c)
                {
                    case '&':// &&
                    {
                        curStrIdx++;
                        code=225;
                        outFile<<code;
                        break;
                    }
                    case '=':// &=
                    {
                        curStrIdx++;
                        code=226;
                        outFile<<code;
                        break;
                    }
                    default: // &
                    {
                        code=227;
                        outFile<<code;
                        break;
                    }
                }
                break;
            }
            case '|':
            {
                c = srcCode.at(curStrIdx).toLatin1();
                switch(c)
                {
                    case '|':// ||
                    {
                        curStrIdx++;
                        code=228;
                        outFile<<code;
                        break;
                    }
                    case '=':// |=运算
                    {
                        curStrIdx++;
                        code=229;
                        outFile<<code;
                        break;
                    }
                    default:// |
                    {
                        code=230;
                        outFile<<code;
                        break;
                    }
                }
                break;
            }
            case '^':
            {
                c = srcCode.at(curStrIdx).toLatin1();
                switch (c)
                {
                    case '=': // ^=情况
                    {
                        curStrIdx++;
                        code=231;
                        outFile<<code;
                        break;
                    }
                    default: // ^情况
                    {
                        code=232;
                        outFile<<code;
                        break;
                    }
                }
                break;
            }
            case '<':
            {
                c = srcCode.at(curStrIdx).toLatin1();
                if(isalnum(c)||c=='_')
                {
                    //code = 9;
                    tmpStr = "(";
                    curStrIdx++;
                    while(!(c=='>'))
                    {
                        tmpStr += c;//不转换
                        c = srcCode.at(curStrIdx).toLatin1();curStrIdx++;
                    }
                    outFile << tmpStr << ')';//checkthis======
                    break;
                }
                switch (c)
                {
                    case '=':
                    {
                        curStrIdx++;
                        code=233;
                        outFile<<code;
                        break;
                    }
                    case '<':
                    {
                        curStrIdx++;c = srcCode.at(curStrIdx).toLatin1();//checkthis
                        switch (c)
                        {

                            case '=':
                            {
                                curStrIdx++;
                                code=234;
                                outFile<<code;
                                break;
                            }
                            default:// <<
                            {
                                curStrIdx++;
                                code=235;
                                outFile<<code;
                                break;
                            }
                        }
                        break;
                    }
                    default: // <
                    {
                        code=236;
                        outFile<<code;
                        break;
                    }
                }
                break;
            }
            case '>':
            {
                c = srcCode.at(curStrIdx).toLatin1();
                switch (c)
                {
                    case '=': // >=
                    {
                        curStrIdx++;
                        code=237;
                        outFile<<code;
                        break;
                    }
                    case '>':
                    {
                        curStrIdx++;c = srcCode.at(curStrIdx).toLatin1();
                        switch (c)
                        {
                            case '=': //>>=
                            {
                                curStrIdx++;
                                code=238;
                                outFile<<code;
                                break;
                            }
                            default: // >>
                            {
                                curStrIdx++;
                                code=239;
                                outFile<<code;
                                break;
                            }
                        }
                        break;
                    }
                    default: // >
                    {
                        code=240;
                        outFile<<code;
                        break;
                    }
                }
                break;
            }
            case '?':
            {
                code=241;
                outFile<<code;
                break;
            }
            case '~':
            {
                code=242;
                outFile<<code;
                break;
            }
            case '.':// checkthis
            {
                code=243;
                outFile<<code;
                break;
            }
            case '[':
            {
                code=244;
                outFile<<code;
                break;
            }
            case ']':
            {
                code=245;
                outFile<<code;
                break;
            }
            case ' ':
            {
                code=251;
                outFile<<code;
                break;
            }
            case '\n':
            {
                outFile<<'\n';
                break;
            }
            default:
            {
                if(isdigit(c))
                {
                    code=249;
                    tmpStr="";
                    tmpStr+=c;
                    c = srcCode.at(curStrIdx).toLatin1();curStrIdx++;
                    while(!(isspace(c)||c==','||c==';'||c==']'||c=='}'))
                    {
                        tmpStr+=c;
                        c = srcCode.at(curStrIdx).toLatin1();curStrIdx++;
                    }
                    curStrIdx--;
                    outFile << code << tmpStr << (unsigned char)250;
                    break;
                }
                else if(!isspace(c))//若非空白
                {
                    tmpStr="";
                    tmpStr+=c;
                    c=srcCode.at(curStrIdx).toLatin1();//观察下一个字符

                    while(isalnum(c)||c=='_')// 其它变量，这里由于前面分支的筛选因此不会出现数字第一
                    {
                        c = srcCode.at(curStrIdx).toLatin1();curStrIdx++;
                        tmpStr+=c;
                        c = srcCode[curStrIdx].toLatin1();//checkthis==============
                    }

                    if(c=='>'){curStrIdx++;c = srcCode.at(curStrIdx).toLatin1();tmpStr+=c;}

                    //qDebug()<<QString::fromStdString(tmpStr)<<Qt::endl;
                    if(KEYWORDS_MAP.contains(QString::fromStdString(tmpStr)))
                    {
                        outFile<<KEYWORDS_MAP[QString::fromStdString(tmpStr)];//输出对应关键字和预处理指令的替换标记
                        //qDebug()<<QString::fromStdString(tmpStr)<<Qt::endl;
                    }
                    else //一般的标识符
                    {
                        code=(unsigned char)252;
                        outFile<<code<<tmpStr<<(unsigned char)253;//checkthis
                    }
                }
            }
        }
    }
    outFile.close();

    std::ifstream readF;
    readF.open(".\\compress.bin",std::ios::binary|std::ios::in);
    QString srcStr="";

    while(true)
    {
        srcStr+=QChar::fromLatin1(readF.get());
        if(readF.eof())break;
    }
    ui->compressCode_textEdit->insertPlainText(srcStr);
    ui->tabWidget->setCurrentIndex(0);//界面调整到压缩代码显示的界面
    ui->lineEdit_compress->setText(QDir::currentPath());
    ui->pushButton_decompress->setEnabled(1);
}

/**
 * @brief MainWindow::on_pushButton_decompress_clicked
 * 解压缩操作，将压缩文件解压缩得到的结果将会保存在本地，同时解压的文本会显示在窗口中，而且可以自行选择将解压缩文件另存为到另外一个地方。
 */
void MainWindow::on_pushButton_decompress_clicked()
{

    ui->decompressCode_textEdit->clear();//清空之前的输出
    ui->tabWidget->setCurrentIndex(2);

    std::ifstream readF;
    readF.open(".\\compress.bin",std::ios::binary);
    ui->tabWidget->setCurrentIndex(1);//调整到解压缩代码界面

    unsigned char code;
    int fileSize=0;
    std::string tmpStr;

    QString dncCode="";
    while(true)
    {
        code=readF.get();
        fileSize++;
        if(readF.eof()){ui->console_textEdit->insertPlainText("decompressCode size:"+QString::number(fileSize)+'\n');break;}
        if(code > 127 && code < 200)
        {
            dncCode+=WORD_VALUES_MAP[code];
        }
        else if(code>=200 && code < 247)
        {
            dncCode+=PUNCT_VALUES_MAP[(int)code];
        }
        else
        {
            switch (code)
            {
                case '(': //引用头文件'<'与'>'转义情况
                {
                    dncCode+='<';
                    while(true)
                    {
                        code=readF.get();
                        fileSize++;
                        if(code==')'){dncCode+='>';break;}//若达到引用头文件名的'>'时则结束该判断
                        dncCode+=QChar(code);//否则直接输出

                    }
                    break;
                }
                case 247: // 字符串情况处理
                {
                    dncCode+='"';
                    while(true)
                    {
                        code=readF.get();
                        fileSize++;
                        if(code==248){dncCode+='"';break;}
                        dncCode+=char(code);//字符串内字符直接输出
                        //qDebug()<<code<<"(XX";
                    }
                    break;
                }
                case 249: //数字情况处理
                {
                    while(true)
                    {
                        code=readF.get();
                        fileSize++;
                        if(code==250)break;
                        dncCode+=QChar(code);//数字内字符直接输出
                    }
                    break;
                }
                case 251: //空格情况处理
                {
                    dncCode+=' ';
                    break;
                }
                case 252: //一般标识符情况处理
                {
                    while(true)
                    {
                        code=readF.get();
                        fileSize++;
                        if(code==253)break;
                        dncCode+=QChar(code);//数字内字符直接输出
                    }
                    break;
                }
                case 254:
                {
                    dncCode+='\'';
                    while(true)
                    {
                        code=readF.get();
                        fileSize++;
                        if(code==255){dncCode+='\'';break;}
                        dncCode+=QChar(code);//字符串内字符直接输出
                    }
                    break;
                }
                default:
                {
                    dncCode+=QChar(code);

                    break;
                }
            }
        }        
    }
    ui->decompressCode_textEdit->insertPlainText(dncCode);
    decompressCode=dncCode;

    QFile srcFile(".\\output.txt");
    if(!srcFile.open(QIODevice::ReadWrite|QIODevice::Text|QIODevice::Truncate))
    {
        QMessageBox::warning(NULL, "文件", "文件打开失败(decompressUtil)");
    }
    QTextStream textOutput(&srcFile);
    textOutput.setEncoding(QStringConverter::Utf8);//设置编码，防止中文乱码
    textOutput<<dncCode;
    srcFile.close();
    ui->lineEdit_decompress->setText(QDir::currentPath());
    ui->pushButton_eSaveTargetCode->setEnabled(1);
}


void MainWindow::on_pushButton_eSaveTargetCode_clicked()
{
    QString tgtFilePath=QFileDialog::getSaveFileName(this,"选择路径","/");
    qDebug()<<tgtFilePath;
    QFile tgtFile(tgtFilePath);
    if(!tgtFile.open(QIODevice::ReadWrite|QIODevice::Text|QIODevice::Truncate))
    {
        QMessageBox::warning(NULL, "文件", "文件打开失败");
        return;
    }
    QTextStream outputFile(&tgtFile);
    outputFile<<decompressCode;
    tgtFile.close();
    QMessageBox::information(NULL,"文件","文件保存成功(eSave)");
}
