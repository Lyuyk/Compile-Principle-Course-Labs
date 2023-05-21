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

    QString t_grammarString=s;
    QStringList t_productionList = t_grammarString.split("\n");//分割出一条条产生式
    QStringList t_productionR_list;//暂存储右部

    m_startChar=t_productionList[0].split("->").at(0);//开始符号，为第一条产生式左部

    //扫描每一行
    for(const auto &t_subProduction: qAsConst(t_productionList))
    {
        if(t_subProduction.simplified().isEmpty())continue;//略去空行

        QStringList t_productionLRList = t_subProduction.split("->");//分离左右部
        QString t_productionL = t_productionLRList[0].simplified();//左部
        m_nonTmrSet.push_back(t_productionL);//左部全部为非终结符，加入

        t_productionR_list=t_productionLRList[1].split("|");//右部
    }


    for(const auto &t_productionL:m_nonTmrSet)
    {
        //遍历右部每一条候选式
        for(const auto &t_productionR: qAsConst(t_productionR_list))
        {
            QList<QString> t_candidateV=t_productionR.split(' ');//每一条候选式分开

            for(const auto&t_candidateStr: t_candidateV)//对每一条候选式字符串，遍历每一个操作符
            {
                QList<QString> t_candidateCharV=t_candidateStr.split(' ');
                m_GM_productionMap[t_productionL].pdnRights.push_back(t_candidateCharV);//加入一条候选式

                for(const auto &t_cChar:t_candidateCharV)
                    //右部如果不是非终结符，则全部当成终结符加入
                    if(not isTerminator(t_cChar))
                        m_tmrSet.insert(t_cChar);

            }

        }
    }
    //SimplifyGrammar();
}

/**
 * @brief BNFP::isTerminator
 * @param s
 * @return
 * 判断是否为终结符
 */
bool BNFP::isTerminator(QString s)
{
    return m_tmrSet.contains(s);
}

/**
 * @brief BNFP::isNonTerminator
 * @param s
 * @return
 * 判断是否为非终结符
 */
bool BNFP::isNonTerminator(QString s)
{
    return m_nonTmrSet.contains(s);
}




/**
 * @brief BNFP::SimplifyGrammar
 * 化简文法主函数
 */
void BNFP::simplifyGrammar()
{
    bool changedFlag=1;//用于判断文法中的产生式是否有变化
    QSet<QString> temp={m_startChar};//暂时可到达但不可终止的非终结符
    QSet<QString> Temp={};//可终止且可到达的非终结符
    while(changedFlag)
    {
        changedFlag=0;
        foreach(const QString& str,temp)
        {
            for (const QStringList &Ts : m_GM_productionMap[str].pdnRights){//遍历该产生式的所有候选值
                int Flag=1;//判断当前候选值中的所有字符是否可终止
                for(const QString &t :Ts){//遍历该候选式中的每个单词
                    if(m_nonTmrSet.contains(t)&&!Temp.contains(t)){
                        Flag=0;
                        if(!temp.contains(t)){
                            temp.insert(t);
                            changedFlag=1;
                        }
                    }
                }
                if(Flag){
                    Temp.insert(str);
                    temp.remove(str);
                    changedFlag=1;
                };
            }
        }
    }

    QSet<QString> Tmp(m_nonTmrSet.begin(),m_nonTmrSet.end());//原来的非终结符集
    if(Tmp!=Temp)
    {
        QSet<QString> del=Tmp-Temp;//即将删除的非终结符集

        for(const auto &tmp: del){
            m_nonTmrSet.remove(m_nonTmrSet.indexOf(tmp));
            m_GM_productionMap.remove(tmp);
        }

        for(const auto& str: Temp)
        {
            QSet<QStringList> deleteSet={};
            for (const auto &Ts : m_GM_productionMap[str].pdnRights)//遍历该产生式的所有候选值
                for(const auto &t :Ts)//遍历该候选式中的每个单词
                    if(Tmp.contains(t)&&!m_nonTmrSet.contains(t))//原来是非终结符但被删除掉了
                    {
                        QStringList delTmp=Ts;
                        deleteSet.insert(delTmp);
                    }
            for(const auto& delTmp: deleteSet)
                m_GM_productionMap[str].pdnRights.removeOne(delTmp);
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
            if(t_productionR.isEmpty())t_productionRStr += "| @ ";
            else
            {
                t_productionRStr+=" | ";
                //每一个操作符
                for(const auto &t_candidate: t_productionR)
                {
                    t_productionRStr += t_candidate+' ';
                }
            }
        }
        QString genLine=QString(t_productionL)+" -> "+t_productionRStr.remove(0,2);//字符串处理去掉首个右部'|'
        grammarString += (genLine+'\n');
    }

    e->insertPlainText(grammarString);
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
    table->verticalHeader()->setHidden(true);//隐藏序号列

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
void BNFP::eliminateLRecursion(int index, QSet<QString> &updatedL)
{
    QString left=m_nonTmrSet[index];
    QSet<QString>newRight={};
    QVector<QStringList> newR;

    for (const QStringList &Ts : m_GM_productionMap[left].pdnRights){//遍历该产生式的所有候选值
        int flag=0;//判断是不是左递归
        QString right=Ts[0];
        if(updatedL.contains(right)){
            flag=1;
            for (const QStringList &ts : m_GM_productionMap[left].pdnRights){
                QStringList s=Ts;
                s.removeFirst();//把第一个元素删掉
                s=ts+s;//用ts换掉本身的第一个元素
                newR.append(s);
            }
        }
        if(!flag)newR.append(Ts);
    }
    m_GM_productionMap[left].pdnRights=newR;

    //直接左递归
    for (const QStringList &Ts : m_GM_productionMap[left].pdnRights)
    {
        //遍历该产生式的所有候选值
        QString right=Ts[0];
        if(right==left){
            QString newLeft=left+'\'';//新非终结符
            QVector<QStringList>NR_1;//存储当前非终结符消除直接左递归后的候选式
            QVector<QStringList>NR_2;//存储新非终结符候选式
            for (const QStringList &ts : m_GM_productionMap[left].pdnRights){//遍历该产生式的所有候选值
                QString r=ts[0];
                if(r==left){
                    QStringList s=Ts;
                    s.removeFirst();
                    s.append(newLeft);
                    NR_2.append(s);
                }
                else{
                    QStringList s=ts;
                    s.append(newLeft);
                    NR_1.append(s);
                }
            }
            NR_2.append({"@"});
            m_GM_productionMap[left].pdnRights=NR_1;//更新当前非终结符的候选式
            //向文法中添加新非终结符
            m_nonTmrSet.append(newLeft);
            m_GM_productionMap[newLeft].pdnRights=NR_2;
            break;
        }
    }
}



/**
 * @brief BNFP::eliminateLRecursion
 * 消除文法左递归
 */
void BNFP::eliminateLRecursion()
{
    QSet<QString> t_newNonTmrSet;//存储处理后的非终结符
    for(int i=0;i<m_nonTmrSet.size();i++)
    {
        QString nTmrStr=m_nonTmrSet[i];
        eliminateLRecursion(i,t_newNonTmrSet);
        t_newNonTmrSet.insert(nTmrStr);
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
    int flag=4;
    int Flag=1;
    int size=m_nonTmrSet.size();
    while(Flag&&flag--){
    Flag=0;
    QMap<QString,QSet<QStringList>> delSet={};
    QMap<QString,QSet<QStringList>> appendSet={};
    QMap<QString,QVector<QStringList>> newSet={};
    //遍历完再删 否则容易导致越界
        for(int i=0;i<size;i++)//遍历所有产生式
        {
            QString s=m_nonTmrSet.at(i);

            QStringList Ts=m_GM_productionMap[s].pdnRights.at(0);
            //QSet<QStringList> Temp={};//用于记录被提取的候选式 用Set不知道为什么有可能会越界
            QList<QStringList> Temp={};//用于记录被提取的候选式
            QString tmp=Ts.at(0);
            for(int k=0;k<m_GM_productionMap[s].pdnRights.size();k++){
                QStringList ts=m_GM_productionMap[s].pdnRights.at(k);
//                if(ts.at(0)==tmp)Temp.insert(ts);//判断该候选式是否有相同左公因子
                if(ts.at(0)==tmp)Temp.append(ts);//判断该候选式是否有相同左公因子
            }
            if(Temp.count()>1&&Temp.count()==m_GM_productionMap[s].pdnRights.size())
            {
                int cnt=1;
                lFactorCount(Temp,Ts,cnt);//记录最长左公因子个数
                for (const QStringList &del : Temp){
                    QStringList delTmp=del;

                    //delSet.insert(delTmp);
                    delSet[s].insert(delTmp);
                    //G.ntMap[s].right.remove(G.ntMap[s].right.indexOf(delTmp));//将Temp中的候选式从该产生式右部去除
                }
                for(const QStringList& STR: Temp){//形成新产生式的右部
//                    Temp.remove(STR);
//                    if(cnt!=STR.size())Temp.insert(STR.mid(cnt));
//                    else Temp.insert({"@"});
                    Temp.removeOne(STR);
                    if(cnt!=STR.size())Temp.append(STR.mid(cnt));
                    else if(!Temp.contains({"@"}))Temp.append({"@"});
                }
                //给新的产生式赋值
                QString left=s+'\'';
                newSet[left]=Temp.toVector();

                //将化简后的候选式加入该产生式
                QStringList leftFactor=Ts.mid(0,cnt);
                leftFactor.append(left);

                appendSet[s].insert(leftFactor);
            }
            if(delSet.size()||appendSet.size()||newSet.size())Flag=1;
        }
        for(QString s: delSet.keys())
            for(const QStringList& delTmp: delSet[s])
                m_GM_productionMap[s].pdnRights.removeOne(delTmp);
        for(QString s: appendSet.keys())
            for(const QStringList& appendTmp: appendSet[s])
                m_GM_productionMap[s].pdnRights.append(appendTmp);
        for(QString left: newSet.keys()){
            m_nonTmrSet.append(left);
            m_GM_productionMap[left].pdnRights=newSet[left];
        }


        delSet.clear();
        appendSet.clear();
        for(int i=0;i<size;i++)
        {
            QString s=m_nonTmrSet.at(i);

            if(m_GM_productionMap[s].pdnRights.size()>1)
                for(int j=0;j<m_GM_productionMap[s].pdnRights.size();j++){
                    QStringList Ts=m_GM_productionMap[s].pdnRights.at(j);
                    QString tmp=Ts.at(0);
                    if(m_nonTmrSet.contains(tmp)){
                        QStringList del=Ts;
                        for(int k=0;k<m_GM_productionMap[tmp].pdnRights.size();k++){
                            QStringList ts=m_GM_productionMap[tmp].pdnRights.at(k);

                            appendSet[s].insert(ts+Ts.mid(1));
                        }

                        //delSet.insert(del);
                        delSet[s].insert(del);
                    }
                    else if(tmp=="@"&&Ts.size()>1){
                       QStringList del=Ts;
                        appendSet[s].insert(Ts.mid(1));
                        delSet[s].insert(del);
                    }
                }
            if(delSet.size()||appendSet.size())Flag=1;
        }
        for(QString s: delSet.keys())
            for(const QStringList& delTmp: delSet[s])
                m_GM_productionMap[s].pdnRights.removeOne(delTmp);
        for(QString s: appendSet.keys())
            for(const QStringList& appendTmp: appendSet[s])
                m_GM_productionMap[s].pdnRights.append(appendTmp);
    }


    simplifyGrammar();
}

/**
 * @brief BNFP::computeFirstSet
 * 计算First集
 */
void BNFP::computeFirstSet()
{
    bool flag=1; // 标记此轮是否发生了更新
    while(flag)//直到first集不变
    {
        flag=false;
        for(int i=0;i<m_nonTmrSet.size();i++)//遍历所有产生式
        {
            QString s=m_nonTmrSet[i];
            QVector<QStringList> Production=m_GM_productionMap[s].pdnRights;
            for(int j=0;j<Production.size();j++)
            {
                QString tmp= Production[j][0];
                if(!m_nonTmrSet.contains(tmp)||tmp == "@")//终结符和epsilon
                {
                    if(!m_GM_productionMap[s].firstSet.contains(tmp))//first集合里是否有tmp
                    {
                      m_GM_productionMap[s].firstSet.insert(tmp);
                      flag=1;
                    }
                }
                else//非终结符
                {
                    bool epsilon=false;//是否出现epsilon
                    for(int k=0;k<Production[j].size();k++)
                    {
                        tmp=Production[j][k];
                        if(!m_nonTmrSet.contains(tmp))
                        {
                            if(!m_GM_productionMap[s].firstSet.contains(tmp))//first集合里是否有tmp
                            {
                              m_GM_productionMap[s].firstSet.insert(tmp);
                              flag=1;
                            }
                            break;//遇到了终结符可以直接结束了
                        }

                        for(const QString& str: m_GM_productionMap[tmp].firstSet)//查该非终结符的first集合
                        {
                            if(str=="@")epsilon=1;
                            if(!m_GM_productionMap[s].firstSet.contains(str))//first集合里是否有tmp
                            {
                              m_GM_productionMap[s].firstSet.insert(str);
                              flag=1;
                            }
                        }
                        if(!epsilon)break;//该非终结符first集合中无epsilon
                        epsilon=0;//继续找下一个字符
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
    bool flag=1;
    m_GM_productionMap[m_startChar].followSet.insert("$");//文法开始符号加结束符号
    while(flag)
    {
        flag=0;
        for(int i=0;i<m_nonTmrSet.size();i++)//遍历所有产生式
        {
            QString s=m_nonTmrSet[i];
            QVector<QStringList> Production=m_GM_productionMap[s].pdnRights;
            for(int j=0;j<Production.size();j++)//遍历每一个候选式
            {
                for(int k=0;k<Production[j].size();k++)//寻找每个候选式中的每个非终结符
                {
                    QString tmp= Production[j][k];
                    if(m_nonTmrSet.contains(tmp))//是非终结符
                    {
                        if(k+1==Production[j].size())//当前非终结符为候选式中的最后一个字符
                        {
                            foreach(const QString& follow,m_GM_productionMap[s].followSet)
                                if(!m_GM_productionMap[tmp].followSet.contains(follow))
                                {
                                    flag=1;
                                    m_GM_productionMap[tmp].followSet.insert(follow);
                                }
                        }
                        else
                        {
                            QString Tmp=Production[j][k+1];//查看下一个字符
                            if(!m_nonTmrSet.contains(Tmp))//终结符
                            {
                                if(!m_GM_productionMap[tmp].followSet.contains(Tmp))
                                {
                                    flag=1;
                                    m_GM_productionMap[tmp].followSet.insert(Tmp);
                                }
                            }
                            else
                            {
                                for(const QString& follow: m_GM_productionMap[Tmp].firstSet)
                                {
                                    if(follow=="@")continue;
                                    if(!m_GM_productionMap[tmp].followSet.contains(follow))
                                    {
                                        flag=1;
                                        m_GM_productionMap[tmp].followSet.insert(follow);
                                    }
                                }
                                bool epsilon=1;
                                for(int x=k+1;x<Production[j].size();x++)//查看是否后面的字符都含有epsilon
                                {
                                    QString nt=Production[j][x];
                                    if(!m_nonTmrSet.contains(nt)||!m_GM_productionMap[nt].firstSet.contains("@"))//如果其中一个不含epsilon
                                        epsilon=0;
                                }
                                if(epsilon)//如果后面的字符都含epsilon 相当于是最后一个字符
                                {
                                    for(const auto& follow: m_GM_productionMap[s].followSet)
                                        if(!m_GM_productionMap[tmp].followSet.contains(follow))
                                        {
                                            flag=1;
                                            m_GM_productionMap[tmp].followSet.insert(follow);
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
        QString nt=m_nonTmrSet[i];//当前非终结符
        QVector<QStringList> rights=m_GM_productionMap[nt].pdnRights;//当前非终结符对应的产生式
        for(int j=0;j<rights.size();j++)
        {
            QStringList right=rights[j];//当前非终结符对应的产生式中的一个候选式
            QString contentRight="";
            for(int k=0;k<right.size();k++)contentRight+=right[k]+' ';
            contentRight.chop(1);//把最后多出来的空格去掉
            QString content=nt+"->"+contentRight;//要填进ll1分析表中的内容

            QString tmp= right[0];//该候选式的第一个字符
            if(!m_nonTmrSet.contains(tmp))//不是非终结符
            {
                if(tmp=="@")
                {
                    foreach(const QString& follow,m_GM_productionMap[nt].followSet)
                    {
                        if(!m_LL1Table[nt].contains(follow))
                            m_LL1Table[nt][follow]=content;
                        else{
                            if(m_LL1Table[nt][follow].split("->")[1]=="@")//人工干预
                                m_LL1Table[nt][follow]=content;
                        }
                    }
                    continue;
                }
                m_LL1Table[nt][tmp]=content;//填表
            }
            else//非终结符
            {
                bool flag=0;
                int size=right.size();
                for(int k=0;k<size;k++)
                {
                    tmp=right[k];
                    if(!m_nonTmrSet.contains(tmp))//不是非终结符,这里其实是假如第一个非终结符的first集合为空才有可能进来
                    {
                        if(tmp=="@")continue;//epsilon不加进去。其实到这里也肯定不会有epsilon了，因为应该也没有候选式的epsilon会出现在中间
                        m_LL1Table[nt][tmp]=content;//填表
                        break;//可以退出循环了
                    }
                    else//非终结符
                    {
                        for(const auto& first: m_GM_productionMap[tmp].firstSet)//遍历first集
                        {
                            if(first=="@")flag=1;//记录存在epsilon
                            else m_LL1Table[nt][first]=content;
                        }
                        if(flag)
                        {
                            flag=0;
                            if(k+1==size)//到最后一个了还是有epsilon就看follow集
                                for(const auto& follow: m_GM_productionMap[nt].followSet)
                                    m_LL1Table[nt][follow]=content;
                        }
                        else break;
                    }
                }
            }
        }
    }
}

