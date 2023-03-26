#ifndef DNCOMPRESSORUTIL_H
#define DNCOMPRESSORUTIL_H

#include <QString>
#include <QMap>
#include <QList>

QMap<QString,unsigned char> PUNCT_MAP;
QMap<QString,unsigned char> KEYWORDS_MAP;
QMap<unsigned char,QString> WORD_VALUES_MAP;
QMap<unsigned char,QString> PUNCT_VALUES_MAP;
QList<QString> KEYWORDS_LIST= {"asm","auto","bool","break","case",
                               "catch","char","class","const","const_cast","continue",
                               "default","delete","do","double","dynamic_cast",
                               "else","enum","explicit","export","extern",
                               "false","float","for","friend","goto",
                               "if","inline","int","long","mutable","namespace",
                               "operator","private","protected","public",
                               "register","reinterpret_cast","return",
                               "short","signed","sizeof","static",
                               "static_cast","struct","switch",
                               "template","throw","true","try",
                               "typedef","typeid","typename",
                               "union","unsigned","using",
                               "virtual","void","volatile","wchar_t","while",
                               "#include","#define","#if","else","elif","endif",
                               "#ifdef","#ifndef","#error","string"};

const QStringList PUNCT_LIST ={
    ",",";","{","}","(",")",
        "::",":","/=","/",
        "++","+=","+",
        "--","-=","->","-",
        "*=","*","%=","%",
        "==","=","!=","!",
        "&&","&=","&",
        "||","|=","|",
        "^=","^",
        "<=","<<=","<<","<",
        ">=",">>=",">>",">",
        "?","~",".","[","]","\\"
};

int PUNCTLIST_SIZE=0;
int KEYLIST_SIZE=0;//关键字列表长度
int curStat =0;//自动机当前状态
int curRowIdx=0;//行号标记
int curColIdx=0;//当前行的列标

#endif // DNCOMPRESSORUTIL_H
