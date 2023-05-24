/****************************************************
 * @Copyright © 2021-2023 Lyuyk. All rights reserved.
 *
 * @FileName: bnfp.cpp
 * @Brief: BNF文法处理类源文件
 * @Module Function:
 *
 * @Current Version: 1.3
 * @Author: Lyuyk
 * @Modifier: Lyuyk
 * @Finished Date: 2023/5/16
 *
 * @Version History: 1.1
 *                   1.2 完善了部分注释，提高了部分代码的可读性
 *                   1.3 降低代码耦合性，增添了任务二要求的与LL(1)分析相关的功能
 *
 ****************************************************/

#include "bnfp.h"

BNFP::BNFP()
{
    init();
}

/**
 * @brief BNFP::Init
 * 类各成员变量初始化
 */
void BNFP::init()
{
    m_grammarStr="";//暂存整个文法
    m_startChar="";

    m_nonTmrSet.clear();
    m_tmrSet.clear();

    m_GM_productionMap.clear();
    m_LL1Table.clear();
    m_programCode.clear();
}

/**
 * @brief BNFP::InitGrammar
 * 文法初始化，BNF文法的解析与存储
 */
void BNFP::initGrammar(QString s)
{
    init();

    QStringList t_pdnList = s.split("\n");//分割出一条条产生式


    m_startChar=t_pdnList[0].split("->")[0].trimmed();//开始符号，为第一条产生式左部
    qDebug()<<"m_startChar:"<<m_startChar;

    //扫描每一行，先存左部非终结符，方便后面判断
    for(const auto &t_line: t_pdnList)
    {
        if(t_line.simplified().isEmpty())continue;//略去空行

        QString t_pdnL = t_line.split("->")[0].trimmed();//左部
        m_nonTmrSet.append(t_pdnL);
    }

    for(const auto t_line: t_pdnList)
    {
        if(t_line.isEmpty())continue;
        QString t_leftStr=t_line.split("->")[0];//左部
        QString t_rightStr=t_line.split("->")[1];//右部字符串
        QStringList t_candidateList=t_rightStr.split('|');//暂存储右部列表

        qDebug()<<t_leftStr;
        //遍历右部每一条候选式字符串
        for(const auto &t_cddStr: t_candidateList)
        {
            QList<QString> t_cddList=t_cddStr.trimmed().split(' ');//每一个单词分开

            m_GM_productionMap[t_leftStr].pdnRights.append(t_cddList);//加入调
            for(const auto &t_cWord: t_cddList)//对每一条候选式单词
            {
                    if(!m_nonTmrSet.contains(t_cWord))
                        m_tmrSet.insert(t_cWord);//右部如果不是非终结符，则全部当成终结符加入

            }
        }
    }


}


/**
 * @brief BNFP::simplifyGrammar
 * 化简文法主函数
 */
void BNFP::simplifyGrammar()
{
    bool changedFlag=true;//用于判断文法中的产生式是否有变化
    QSet<QString> t_reachNoEndSet={m_startChar};//暂时可到达但不可终止的非终结符
    QSet<QString> t_reachEndSet;//可终止且可到达的非终结符
    while(changedFlag)
    {
        changedFlag=false;
        for(const auto &t_RneNonTmr: t_reachNoEndSet)//暂时可达不可终止非终结符
        {
            //遍历该产生式的所有候选式
            for (const auto &t_candidateList : m_GM_productionMap[t_RneNonTmr].pdnRights)
            {
                bool allEndFlag=true;//当前候选式中的所有字符是否可终止标志
                //遍历该候选式中的每个单词
                for(const auto &t_cWord: t_candidateList)
                {
                    //若在非终结符集 且不在 可达&可终止集合中
                    if(m_nonTmrSet.contains(t_cWord) && !t_reachEndSet.contains(t_cWord))
                    {
                        allEndFlag=false;//发现有不可终止的
                        if(!t_reachNoEndSet.contains(t_cWord))//如果它不在 可达&不可终止集合中，则加入
                        {
                            t_reachNoEndSet.insert(t_cWord);
                            changedFlag=true;//产生式已经变更
                        }
                    }
                }
                if(allEndFlag)
                {
                    //若候选式中所有单词都可达且可终止，则将当前非终结符移至 可达可终止集合
                    t_reachEndSet.insert(t_RneNonTmr);
                    t_reachNoEndSet.remove(t_RneNonTmr);
                    changedFlag=true;
                }
            }
        }
    }

    QSet<QString> t_nonTmrSet(m_nonTmrSet.begin(),m_nonTmrSet.end());//原来的非终结符集，用于判断
    //若两集合不相同，则说明原来的非终结符集合有不是非终结符的，我们通过循环判断删除
    if(t_nonTmrSet!=t_reachEndSet)
    {
        QSet<QString> t_deletedSet=t_nonTmrSet-t_reachEndSet;//即将删除的非终结符集

        for(const auto &t_word: t_deletedSet)
        {
            m_nonTmrSet.remove(m_nonTmrSet.indexOf(t_word));//移除掉该“非终结符”
            m_GM_productionMap.remove(t_word);//移除该“非终结符”的产生式
        }

        //遍历可达可终结集合
        for(const auto& t_cWord: t_reachEndSet)
        {
            QSet<QStringList> t_deletedListSet={};
            //遍历该产生式的所有候选式
            for (const auto &t_candidateList : m_GM_productionMap[t_cWord].pdnRights)
                for(const auto &t_candidateWord :t_candidateList)//遍历该候选式中的每个单词
                    if(t_nonTmrSet.contains(t_candidateWord) && !m_nonTmrSet.contains(t_candidateWord))//若当中有符号原来是在非终结符集但现在被被删除掉了
                    {
                        QStringList t_deletedList=t_candidateList;
                        t_deletedListSet.insert(t_deletedList);//这种情况我们将这个单条候选式放入删除列表中
                    }
            //遍历删除列表，将需要被删除的候选式删除
            for(const auto& t_delList: t_deletedListSet)
                m_GM_productionMap[t_cWord].pdnRights.removeOne(t_delList);
        }
    }
}

/**
 * @brief BNFP::printGrammar
 * @param e
 * 文法输出主函数
 */
void BNFP::printGrammar(QPlainTextEdit *e)
{
    e->clear();

    QString grammarString;

    //遍历所有左部
    const QList<QString> &t_productionLList=m_GM_productionMap.keys();
    for(const QString &t_productionL: t_productionLList)
    {
        //记录右部
        QString t_productionRStr;
        //每一条候选式
        for(const auto &t_productionR: qAsConst(m_GM_productionMap[t_productionL].pdnRights))
        {
            t_productionRStr+=" | ";
            //每一个操作符
            for(const auto &t_candidate: t_productionR)
            {
                t_productionRStr += t_candidate+' ';
            }
        }
        QString t_Line=QString(t_productionL)+" -> "+t_productionRStr.remove(0,2);//字符串处理去掉首个右部'|'
        grammarString += (t_Line+'\n');
    }

    e->insertPlainText(grammarString);
}

/**
 * @brief BNFP::printSet
 * @param table
 * @param isFirst
 * 输出First/Follow集到表格
 */
void BNFP::printSet(QTableWidget *table,bool isFirst)
{
    table->setRowCount(m_nonTmrSet.size());//行数

    if(isFirst)
    {
        for(int i=0;i<m_nonTmrSet.size();i++)
        {
            QString setStr="{";
            for(const auto &t_first: m_GM_productionMap[m_nonTmrSet[i]].firstSet)
                setStr+=t_first+",";
            setStr+="}";
            table->setItem(i,1,new QTableWidgetItem(setStr));//集合
            table->setItem(i,0,new QTableWidgetItem(m_nonTmrSet[i]));//非终结符
        }
    }
    else
    {
        for(int i=0;i<m_nonTmrSet.size();i++)
        {
            QString setStr="{";
            for(const auto &t_first: m_GM_productionMap[m_nonTmrSet[i]].followSet)
                setStr+=t_first+",";
            setStr+="}";
            table->setItem(i,1,new QTableWidgetItem(setStr));//集合
            table->setItem(i,0,new QTableWidgetItem(m_nonTmrSet[i]));//非终结符
        }
    }
}

/**
 * @brief BNFP::printLL1ParsingTable
 * @param table
 * 显示LL1构造表
 */
void BNFP::printLL1ParsingTable(QTableWidget *table)
{
    table->setRowCount(m_nonTmrSet.size());//行数
    table->setColumnCount(m_tmrSet.size()+2);//列数：空、终结符、结束符

    QStringList headerList=m_tmrSet.values();//终结符
    headerList.push_front("");
    headerList.push_back("$");//结束符号
    table->setHorizontalHeaderLabels(headerList);

    //遍历非终结符
    for(int i=0;i<m_nonTmrSet.size();i++)
    {
        QString t_curNonTmr=m_nonTmrSet[i];//当前非终结符
        table->setItem(i,0,new QTableWidgetItem(t_curNonTmr));

        //遍历终结符及$
        int j=1,colN=1;
        for(;j<headerList.size();j++)
        {
            QString t_tStr=headerList[j];
            if(m_LL1Table[t_curNonTmr].contains(t_tStr))
            {
                table->setItem(i,colN,
                               new QTableWidgetItem(m_LL1Table[t_curNonTmr][t_tStr]));
            }
            colN++;
        }
    }
}

/**
 * @brief BNFP::eliminateLRecursion
 * @param index
 * @param updatedL
 * 消除左递归子函数
 */
void BNFP::eliminateLRecursion(int index, QSet<QString> &newNonTmrSet)
{
    QString t_left=m_nonTmrSet[index];//左部非终结符
    QList<QStringList> t_newRightList;//产生式右部

    //遍历该产生式的所有候选式
    for(const auto &t_candidateList: m_GM_productionMap[t_left].pdnRights)
    {
        bool rcFlag=0;//是否左递归标志
        QString t_cFirst=t_candidateList[0];//候选式首个单词
        if(newNonTmrSet.contains(t_cFirst))
        {   //若暂存终结符集中也出现自己该其则说明含有左递归
            rcFlag=1;
            for(const auto &t_cFirst_candidateList : m_GM_productionMap[t_cFirst].pdnRights)
            {
                QStringList t_cddList=t_candidateList;
                t_cddList.removeFirst();//把第一个元素删掉
                t_cddList=t_cFirst_candidateList+t_cddList;//用t_t...换掉本身的第一个元素，拼起来
                t_newRightList.append(t_cddList);//更新候选式（消除左递归）
            }
        }
        if(!rcFlag)//若没有左递归
            t_newRightList.append(t_candidateList);//则将该候选式加回新的暂存列表中
    }
    m_GM_productionMap[t_left].pdnRights=t_newRightList;//覆写m_nonTmrSet[index]右部

    //直接左递归消除
    for(const auto &t_candidateList : m_GM_productionMap[t_left].pdnRights)
    {
        //遍历该产生式的所有候选式
        QString t_cFirst=t_candidateList[0];
        if(t_cFirst==t_left)//相同即出现A->A...直接左递归
        {
            QString newLeft=getNewTmr(t_left);//申请新的非终结符
            QList<QStringList> t_curElrRights;//存储当前非终结符消除直接左递归后的候选式
            QList<QStringList> t_newNTRights;//存储新非终结符候选式
            for(const auto &t_cddList: m_GM_productionMap[t_left].pdnRights)//遍历该产生式的所有候选式
            {
                QString t_cFirst=t_cddList[0];//候选式首词
                if(t_cFirst==t_left)//左递归出现
                {
                    QStringList t_cList=t_candidateList;
                    t_cList.removeFirst();//删除首词
                    t_cList.append(newLeft);//新的非终结符
                    t_newNTRights.append(t_cList);//加入到新非终结符候选式列表中
                }
                else
                {
                    QStringList t_cList=t_cddList;//没左递归直接加入另外一个集合中
                    t_cList.append(newLeft);
                    t_curElrRights.append(t_cList);
                }
            }
            t_newNTRights.append({"@"});//加入eps
            m_GM_productionMap[t_left].pdnRights=t_curElrRights;//更新当前非终结符的候选式
            //向文法中添加新非终结符
            m_nonTmrSet.append(newLeft);
            m_GM_productionMap[newLeft].pdnRights=t_newNTRights;//添加新非终结符的右部
            break;
        }
    }
}

/**
 * @brief BNFP::eliminateLRecursion
 * 消除文法左递归主函数
 */
void BNFP::eliminateLRecursion()
{
    QSet<QString> t_newNonTmrSet;//存储处理后的非终结符
    for(int i=0;i<m_nonTmrSet.size();i++)
    {
        QString t_nonTmr=m_nonTmrSet[i];//按一定的顺序遍历非终结符，先保存当前非终结符“快照”
        eliminateLRecursion(i,t_newNonTmrSet);
        t_newNonTmrSet.insert(t_nonTmr);//处理完后放回集合中
    }
}

/**
 * @brief BNFP::getNewTmr
 * @return
 * 申请新的终结符
 */
QString BNFP::getNewTmr(QString curTmr)
{
    return curTmr+"'";
}

/**
 * @brief BNFP::decodeLex
 * 解码词法分析程序输出的源程序代码
 */
void BNFP::decodeLex()
{
    QStringList t_lexPrgList=m_lexPrgStr.split('\n');
    for(const auto &line: t_lexPrgList)
    {
        QList<QString> t_wordList=line.trimmed().split(' ');//分开单词
        for(int i=0;i<t_wordList.size()-1;i++)
        {
            QString t_word=t_wordList[i];
            if(!t_word.contains("ID") && !t_word.contains("Digit") && !t_word.contains("Keyword"))
            {
                m_programCode.append(t_word);continue;
            }
            QString t_wordType=t_word.split(':').at(0);
            QString t_wordContent=t_word.split(':').at(1);

            if(t_wordType=="Keyword")
                m_programCode.append(t_wordContent);
            else
            {
                for(const auto&c: t_wordContent)
                    m_programCode.append(c);
            }
        }
    }
    m_programCode.append("#");//规定结束符为#
}

/**
 * @brief BNFP::lFactorCount
 * @param list
 * @param pdnR
 * @param count
 * 记录最长左公因子个数，提取左公因子的子函数
 */
void BNFP::lFactorCount(QList<QStringList> list, QStringList pdnR, int &count)
{
    int t_count=count+1;
    for(const auto& t_pdnR:list)
    {
        if(t_count>t_pdnR.size() || t_pdnR.at(count)!=pdnR.at(count))
            return;
    }
    count=t_count;
    lFactorCount(list,pdnR,count);//递归调用
}

/**
 * @brief BNFP::eliminateLCommonFactor
 * 提取左公因子
 */
void BNFP::eliminateLCommonFactor()
{
    int rcFlag=4;//递归深度

    bool Flag=1;
    int t_nTSetSize=m_nonTmrSet.size();//非终结符集合大小
    while(Flag&&rcFlag--)
    {
        Flag=0;
        QMap<QString,QSet<QStringList>> deletedSetMap={};//记录非终结符要被移除的候选式
        QMap<QString,QSet<QStringList>> appendSet={};//
        QMap<QString,QVector<QStringList>> t_newProductionMap={};//新构造的产生式映射
        //遍历完再删 否则容易导致越界
        for(int i=0;i<t_nTSetSize;i++)//遍历所有产生式
        {
            QString t_curNonTmr=m_nonTmrSet.at(i);//当前非终结符
            QStringList t_curCddList=m_GM_productionMap[t_curNonTmr].pdnRights.at(0);//当前非终结符第一条候选式
            QString t_curCddPrefix=t_curCddList.at(0);//（公共前缀？）当前非终结符第一条候选式的第一个单词
            QList<QStringList> t_eCddList={};//记录被提取的候选式

            //遍历当前非终结符对应的候选式
            for(int j=0;j<m_GM_productionMap[t_curNonTmr].pdnRights.size();j++)
            {
                QStringList t_cdd=m_GM_productionMap[t_curNonTmr].pdnRights.at(j);
                if(t_cdd.at(0)==t_curCddPrefix)//判断该候选式是否有相同左公因子
                    t_eCddList.append(t_cdd);//有就放入提取列表
            }

            //若整个右部都被提取
            if(t_eCddList.count()>1 && t_eCddList.count()==m_GM_productionMap[t_curNonTmr].pdnRights.size())
            {
                int t_lFCount=1;
                lFactorCount(t_eCddList,t_curCddList,t_lFCount);//记录最长左公因子个数
                for(const auto &t_delECdd: t_eCddList)
                {
                    QStringList t_tdelECdd=t_delECdd;
                    deletedSetMap[t_curNonTmr].insert(t_tdelECdd);//记录将被移除的候选式
                }
                for(const auto &t_eCdd: t_eCddList)
                {
                    //形成新产生式的右部
                    t_eCddList.removeOne(t_eCdd);//一条条更新候选式
                    if(t_lFCount != t_eCdd.size())
                        t_eCddList.append(t_eCdd.mid(t_lFCount));
                    else if(!t_eCddList.contains({"@"}))
                        t_eCddList.append({"@"});
                }
                //给新的产生式赋值
                QString left=getNewTmr(t_curNonTmr);//申请新的非终结符
                t_newProductionMap[left]=t_eCddList.toVector();

                //将化简后的候选式加入该产生式
                QStringList leftFactor=t_curCddList.mid(0,t_lFCount);
                leftFactor.append(left);

                appendSet[t_curNonTmr].insert(leftFactor);
            }

            if(deletedSetMap.size()||appendSet.size()||t_newProductionMap.size())
                Flag=true;
        }

        for(const auto& nTmr: deletedSetMap.keys())
            for(const auto& delCdd: deletedSetMap[nTmr])
                m_GM_productionMap[nTmr].pdnRights.removeOne(delCdd);//移除候选式

        for(const auto& nTmr: appendSet.keys())
            for(const auto& appendTmp: appendSet[nTmr])
                m_GM_productionMap[nTmr].pdnRights.append(appendTmp);//增添候选式

        for(const auto& left: t_newProductionMap.keys())
        {
            m_nonTmrSet.append(left);
            m_GM_productionMap[left].pdnRights=t_newProductionMap[left];//增添新候选式
        }

        deletedSetMap.clear();
        appendSet.clear();

        for(int i=0;i<t_nTSetSize;i++)
        {
            QString s=m_nonTmrSet.at(i);

            if(m_GM_productionMap[s].pdnRights.size()>1)
            {
                for(int j=0;j<m_GM_productionMap[s].pdnRights.size();j++)
                {
                    QStringList Ts=m_GM_productionMap[s].pdnRights.at(j);
                    QString tmp=Ts.at(0);
                    if(m_nonTmrSet.contains(tmp))
                    {
                        QStringList del=Ts;
                        for(int k=0;k<m_GM_productionMap[tmp].pdnRights.size();k++)
                        {
                            QStringList ts=m_GM_productionMap[tmp].pdnRights.at(k);

                            appendSet[s].insert(ts+Ts.mid(1));
                        }

                        deletedSetMap[s].insert(del);
                    }
                    else if(tmp=="@"&& Ts.size()>1)
                    {
                        QStringList del=Ts;
                        appendSet[s].insert(Ts.mid(1));
                        deletedSetMap[s].insert(del);
                    }
                }
            }
            if(deletedSetMap.size()||appendSet.size())
                Flag=true;
        }

        for(const QString &s: deletedSetMap.keys())
            for(const QStringList& delTmp: deletedSetMap[s])
                m_GM_productionMap[s].pdnRights.removeOne(delTmp);

        for(QString s: appendSet.keys())
            for(const QStringList& appendTmp: appendSet[s])
                m_GM_productionMap[s].pdnRights.append(appendTmp);
    }

    simplifyGrammar();
}

/**
 * @brief BNFP::firstNFollowSet
 * First集与Follow集的求解
 */
void BNFP::firstNFollowSet()
{
    computeFirstSet();
    computeFollowSet();
}

/**
 * @brief BNFP::computeFirstSet
 * 计算First集
 */
void BNFP::computeFirstSet()
{
    bool changedFlag=true; //标记此轮是否发生了更新，即First是否出现更新
    while(changedFlag)//直到first集不变
    {
        changedFlag=false;
        for(int i=0;i<m_nonTmrSet.size();i++)//遍历所有产生式
        {
            QString t_curLeft=m_nonTmrSet[i];//当前左部
            QVector<QStringList> t_candidateList=m_GM_productionMap[t_curLeft].pdnRights;//当前右部
            //遍历当前右部
            for(int j=0;j<t_candidateList.size();j++)
            {
                QString t_cWord= t_candidateList[j][0];//候选式首字符
                if(!m_nonTmrSet.contains(t_cWord) || t_cWord == "@")//终结符和epsilon（情况1，直接将其加入First集中）
                {
                    if(!m_GM_productionMap[t_curLeft].firstSet.contains(t_cWord))//first集合里是否有t_First
                    {
                        m_GM_productionMap[t_curLeft].firstSet.insert(t_cWord);
                        changedFlag=1;//first集求解情况变更
                    }
                }
                else
                {
                    //t_cFirst为非终结符，将其First集加入左部First集
                    bool epsilonFlag=false;//是否出现epsilon标志
                    for(int k=0;k<t_candidateList[j].size();k++)//遍历单条候选式N->C1C2...Ck
                    {
                        t_cWord=t_candidateList[j][k];
                        if(!m_nonTmrSet.contains(t_cWord))//若遇到终结符
                        {
                            if(!m_GM_productionMap[t_curLeft].firstSet.contains(t_cWord))//first集合里是否有该字符
                            {
                                m_GM_productionMap[t_curLeft].firstSet.insert(t_cWord);//无则加，实际上就是插入操作
                                changedFlag=true;
                            }
                            break;//遇到了终结符可以直接结束了
                        }

                        //查该非终结符的first集合
                        for(const auto &t_firstStr: m_GM_productionMap[t_cWord].firstSet)
                        {
                            if(t_firstStr=="@")//情况2，该情况需要进入下一轮判断，查看规则下一个单词Cx
                                epsilonFlag=true;
                            if(!m_GM_productionMap[t_curLeft].firstSet.contains(t_firstStr))//first集合里是否有tmp
                            {
                                m_GM_productionMap[t_curLeft].firstSet.insert(t_firstStr);//将非终结符first集加入当前左部First集中
                                changedFlag=1;
                            }
                        }
                        if(!epsilonFlag)
                            break;//该非终结符first集合中无epsilon，停止继续遍历候选式
                        epsilonFlag=false;//看下一个字符
                    }
                }
            }
        }
    }
}

/**
 * @brief BNFP::computeFollowSet
 * 计算Follow集
 */
void BNFP::computeFollowSet()
{
    bool changedFlag=true;//follow集产生变化标志
    m_GM_productionMap[m_startChar].followSet.insert("$");//文法开始符号加结束符号
    while(changedFlag)
    {
        changedFlag=false;
        for(int i=0;i<m_nonTmrSet.size();i++)//遍历所有产生式
        {
            QString t_curLeft=m_nonTmrSet[i];//当前左部
            QVector<QStringList> t_candidateList=m_GM_productionMap[t_curLeft].pdnRights;//当前右部
            for(int j=0;j<t_candidateList.size();j++)//遍历每一个候选式
            {
                for(int k=0;k<t_candidateList[j].size();k++)//寻找每个候选式中的每个非终结符
                {
                    QString t_cWord= t_candidateList[j][k];//当前字符
                    if(m_nonTmrSet.contains(t_cWord))//是非终结符
                    {
                        if(k==t_candidateList[j].size()-1)//当前非终结符为候选式中的最后一个字符（情况2）
                        {   //将当前左部follow集加入到 候选式当前字符follow集中
                            for(const QString& t_followStr: m_GM_productionMap[t_curLeft].followSet)
                                if(!m_GM_productionMap[t_cWord].followSet.contains(t_followStr))
                                {
                                    changedFlag=true;
                                    m_GM_productionMap[t_cWord].followSet.insert(t_followStr);
                                }
                        }
                        else
                        {
                            QString t_cNextWord=t_candidateList[j][k+1];//查看下一个字符
                            if(!m_nonTmrSet.contains(t_cNextWord))//若为终结符（情况1）
                            {   //简单更新当前字符follow集
                                if(!m_GM_productionMap[t_cWord].followSet.contains(t_cNextWord))
                                {
                                    m_GM_productionMap[t_cWord].followSet.insert(t_cNextWord);
                                    changedFlag=true;
                                }
                            }
                            else
                            {   //（情况1）将后面字符除eps外的所有first集元素加进来follow集
                                for(const QString& follow: m_GM_productionMap[t_cNextWord].firstSet)
                                {
                                    if(follow=="@")continue;
                                    if(!m_GM_productionMap[t_cWord].followSet.contains(follow))
                                    {
                                        changedFlag=true;
                                        m_GM_productionMap[t_cWord].followSet.insert(follow);
                                    }
                                }
                                bool epsilon=true;
                                for(int x=k+1;x<t_candidateList[j].size();x++)//查看是否后面的字符都含有epsilon（因为如果后面的字符都含epsilon 相当于是最后一个字符）
                                {
                                    QString t_word=t_candidateList[j][x];
                                    if(!m_nonTmrSet.contains(t_word) ||
                                            !m_GM_productionMap[t_word].firstSet.contains("@"))//如果其中一个不含epsilon（情况1，不需要处理）
                                        epsilon=false;
                                }
                                if(epsilon)//如果后面的字符都含epsilon 相当于是最后一个字符（情况2）
                                {   //将当前左部follow集加入到 候选式当前字符follow集中
                                    for(const auto& t_followStr: m_GM_productionMap[t_curLeft].followSet)
                                        if(!m_GM_productionMap[t_cWord].followSet.contains(t_followStr))
                                        {
                                            changedFlag=true;
                                            m_GM_productionMap[t_cWord].followSet.insert(t_followStr);
                                        }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief BNFP::constructLL1ParsingTable
 * 构建LL(1)分析表主函数
 */
void BNFP::constructLL1ParsingTable()
{
    for(int i=0;i<m_nonTmrSet.size();i++)
    {
        QString t_curNT=m_nonTmrSet[i];//当前非终结符
        QVector<QStringList> t_candidateList=m_GM_productionMap[t_curNT].pdnRights;//当前非终结符对应的产生式
        for(int j=0;j<t_candidateList.size();j++)//遍历候选式
        {
            QStringList t_firstCandidate=t_candidateList[j];//当前非终结符对应的产生式中的一个候选式
            QString t_cddStr="";//候选式字符串
            for(int k=0;k<t_firstCandidate.size();k++)
                t_cddStr+=t_firstCandidate[k]+' ';
            t_cddStr.chop(1);//把最后多出来的空格去掉
            QString itemContent=t_curNT+"->"+t_cddStr;//要填进ll1分析表中的内容

            QString t_cFirst=t_firstCandidate[0];//该候选式的第一个字符
            if(!m_nonTmrSet.contains(t_cFirst))//不是非终结符
            {
                if(t_cFirst=="@")//情况2
                {
                    for(const auto& t_follow: m_GM_productionMap[t_curNT].followSet)
                    {
                        if(!m_LL1Table[t_curNT].contains(t_follow))
                            m_LL1Table[t_curNT][t_follow]=itemContent;
                        else
                        {
                            if(m_LL1Table[t_curNT][t_follow].split("->")[1]=="@")//人为修改，
                                m_LL1Table[t_curNT][t_follow]=itemContent;
                        }
                    }
                    continue;
                }
                m_LL1Table[t_curNT][t_cFirst]=itemContent;//存入表中
            }
            else//若为非终结符
            {
                bool epsFlag=false;
                int t_firstCddSize=t_firstCandidate.size();//
                for(int k=0;k<t_firstCddSize;k++)
                {
                    t_cFirst=t_firstCandidate[k];
                    if(!m_nonTmrSet.contains(t_cFirst))//不是非终结符,这里其实是假如第一个非终结符的first集合为空才有可能进来
                    {
                        if(t_cFirst=="@")
                            continue;//epsilon不需加进去，没有候选式的epsilon会出现在中间
                        m_LL1Table[t_curNT][t_cFirst]=itemContent;//填表
                        break;//可以退出循环了
                    }
                    else//非终结符
                    {
                        for(const auto& t_first: m_GM_productionMap[t_cFirst].firstSet)//遍历first集
                        {
                            if(t_first=="@")
                                epsFlag=true;//记录存在epsilon（情况2）
                            else
                                m_LL1Table[t_curNT][t_first]=itemContent;//情况1，直接填入
                        }
                        if(epsFlag)
                        {
                            epsFlag=false;
                            if(k+1==t_firstCddSize)//到最后一个了还是有epsilon就看follow集
                                for(const auto& follow: m_GM_productionMap[t_curNT].followSet)
                                    m_LL1Table[t_curNT][follow]=itemContent;
                        }
                        else break;
                    }
                }
            }
        }
    }
}

void BNFP::LL1Parsing(QTreeWidget *tree, QString progStr)
{
    m_lexPrgStr=progStr;
    decodeLex();//解码源程序


}

