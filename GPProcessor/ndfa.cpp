#include "ndfa.h"
#include <QDebug>

NDFA::NDFA()
{
    NFAStateNum=0;//状态创建计数
    DFAStateNum=0;
    mDFAStateNum=0;
    OpCharSet.clear();
    dividedSet->clear();

    NFAG.startNode=NULL;
    NFAG.endNode=NULL;
    DFAG.endCharSet.clear();
    DFAG.endStates.clear();
    mDFAG.endCharSet.clear();
    mDFAG.endStates.clear();

    for(int i=0;i< ARR_MAX_NUM;i++)
    {
        NFAStateArr[i].stateNum=i;
        NFAStateArr[i].val='#';
        NFAStateArr[i].tState=-1;
        NFAStateArr[i].eSUnion.clear();
    }
    for(int i=0;i<ARR_MAX_NUM;i++)
    {
        DFAStateArr[i].stateNum=i;
        DFAStateArr[i].isEnd=false;
        DFAStateArr[i].edgeCount=0;
        DFAStateArr[i].em_closure_NFA.clear();

        for(int j=0;j<DFA_NODE_EDGE_COUNT;j++)
        {
            DFAStateArr[i].edges[j].input='#';
            DFAStateArr[i].edges[j].tgtState=-1;
        }
    }
    for(int i=0;i<ARR_MAX_NUM;i++)
    {
        mDFANodeArr[i].stateNum=i;
        mDFANodeArr[i].isEnd=false;
        mDFANodeArr[i].edgeCount=0;
        mDFANodeArr[i].em_closure_NFA.clear();

        for(int j=0;j<DFA_NODE_EDGE_COUNT;j++)
        {
            mDFANodeArr[i].edges[j].input='#';
            mDFANodeArr[i].edges[j].tgtState=-1;
        }
    }
}

void NDFA::init()
{
    NFAStateNum=0;//状态创建计数
    DFAStateNum=0;
    mDFAStateNum=0;
    OpCharSet.clear();
    dividedSet->clear();

    NFAG.startNode=NULL;
    NFAG.endNode=NULL;
    DFAG.endCharSet.clear();
    DFAG.endStates.clear();
    mDFAG.endCharSet.clear();
    mDFAG.endStates.clear();

    for(int i=0;i< ARR_MAX_NUM;i++)
    {
        NFAStateArr[i].stateNum=i;
        NFAStateArr[i].val='#';
        NFAStateArr[i].tState=-1;
        NFAStateArr[i].eSUnion.clear();
    }
    for(int i=0;i<ARR_MAX_NUM;i++)
    {
        DFAStateArr[i].stateNum=i;
        DFAStateArr[i].isEnd=false;
        DFAStateArr[i].edgeCount=0;
        DFAStateArr[i].em_closure_NFA.clear();

        for(int j=0;j<DFA_NODE_EDGE_COUNT;j++)
        {
            DFAStateArr[i].edges[j].input='#';
            DFAStateArr[i].edges[j].tgtState=-1;
        }
    }
    for(int i=0;i<ARR_MAX_NUM;i++)
    {
        mDFANodeArr[i].stateNum=i;
        mDFANodeArr[i].isEnd=false;
        mDFANodeArr[i].edgeCount=0;
        mDFANodeArr[i].em_closure_NFA.clear();

        for(int j=0;j<DFA_NODE_EDGE_COUNT;j++)
        {
            mDFANodeArr[i].edges[j].input='#';
            mDFANodeArr[i].edges[j].tgtState=-1;
        }
    }
}

void NDFA::lGrammarToDFA(QMap<QChar, QSet<QString> > productionMap, QSet<QChar> tmrSet, QSet<QChar> nonTmrSet, QChar start)
{
    for(const QChar &c: tmrSet)
    {
        OpCharSet.insert(QString(c));
    }


    int i=0;
    for(const QChar &c: nonTmrSet)
    {
        c2iMap.insert(c,i);
        i2cMap.insert(i,c);
        i++;
    }
    c2iMap.insert('%',i);
    i2cMap.insert(i,'%');
    qDebug()<<i2cMap;

    //终态为文法开始符号 我这里用%表示
    //A->Ba形式
    //A->a 初态--a-->A
    //A->eps 初态--eps-->A
    for(const auto &productionL: productionMap.keys())
    {
        QSet<QString> productionRSet = productionMap[productionL];

        for(const auto &productionR: productionRSet)
        {
            if(productionR.isEmpty())
            {   /*A->eps(start--eps->A)*/
                NFAGraph n = createNFA('%',productionL);//A->eps 初态--eps-->A
                add(n.startNode,n.endNode);
            }
            else if(productionR.size()==1)
            {
                if(productionR.isLower())
                {
                    /*A->a 初态-eps->i-a->A*/
                    i++;
                    NFAGraph n1 = createNFA(c2iMap[start],i);
                    add(n1.startNode,n1.endNode);

                    NFAGraph n2 = createNFA(i,c2iMap[productionL]);
                    add(n2.startNode,n2.endNode,productionR.at(0));
                }
                else
                    qDebug()<<"error";
            }
            else
            {
                /*A->Ba B-eps->i-a->A*/
                i++;
                NFAGraph n1 = createNFA(c2iMap[productionR.at(0)],i);
                add(n1.startNode,n1.endNode);

                NFAGraph n2 = createNFA(i,c2iMap[productionL]);
                add(n2.startNode,n2.endNode,productionR.at(1));
            }
        }

    }

    NFAG.startNode=&NFAStateArr[c2iMap['%']];//初态
    NFAG.endNode=&NFAStateArr[c2iMap[start]];//终态
    NFAStateNum=i+1;

    NFA2DFA();
    DFA2mDFA();
}

void NDFA::rGrammarToDFA(QMap<QChar, QSet<QString> > productionMap,QSet<QChar> tmrSet,QSet<QChar> nonTmrSet,QChar start)
{
    for(const QChar &c: tmrSet)
    {
        OpCharSet.insert(QString(c));
    }

    int i=0;
    for(const QChar &c: nonTmrSet)
    {
        c2iMap.insert(c,i);
        i2cMap.insert(i,c);
        i++;
    }
    c2iMap.insert('$',i);
    i2cMap.insert(i,'$');
    qDebug()<<i2cMap;

    //初态为文法开始符号，我这里用'$'表示
    //A->aB形式
    //A->a A--a-->终态
    //A->eps A--eps-->终态
    for(const auto&productionL: productionMap.keys())
    {
        QSet<QString> productionRSet = productionMap[productionL];

        for(const auto &productionR: productionRSet)
        {
            if(productionR.isEmpty())
            {
                /*A->eps(A--eps->$)*/
                NFAGraph n = createNFA(productionL,'$');
                add(n.startNode,n.endNode);
            }
            else if(productionR.size()==1)
            {
                //qDebug()<<"size=1"<<productionR;
                if(productionR.at(0).isLower())
                {
                    /*A->a(A--a->$)*/
                    NFAGraph n = createNFA(productionL,'$');
                    add(n.startNode,n.endNode,productionR.at(0));
                }
                else
                    qDebug()<<"error";
            }
            else
            {
                //qDebug()<<"size=="<<productionR.size()<<productionR;

                /*L-eps->i-x->R[1]*/
                i++;
                NFAGraph n1 = createNFA(c2iMap[productionL],i);
                add(n1.startNode,n1.endNode);

                NFAGraph n2 = createNFA(i,c2iMap[productionR.at(1)]);
                add(n2.startNode,n2.endNode,productionR.at(0));

            }
        }
    }
    NFAG.startNode=&NFAStateArr[c2iMap[start]];qDebug()<<"start"<<start;//初态
    NFAG.endNode=&NFAStateArr[c2iMap['$']];//终态
    NFAStateNum=i+1;

    NFA2DFA();
    DFA2mDFA();
}

void NDFA::printLinearGrammar(QTableWidget *table)
{
    QStringList OpStrList=OpCharSet.values();
    std::sort(OpStrList.begin(),OpStrList.end());

    OpStrList.push_front("状态集号");
    OpStrList.push_back("初/终态");

    int rowCount=mDFAStateNum;
    int colCount=OpStrList.size();
    //设置表格行列
    table->setColumnCount(colCount);
    table->setRowCount(rowCount);
    //设置表头
    table->setHorizontalHeaderLabels(OpStrList);

    //输出表格内容
    table->setHorizontalHeaderLabels(OpStrList);
    table->setItem(mDFAG.startState,colCount-1,new QTableWidgetItem("初态"));
    //qDebug()<<"mDFAstartState:"<<mDFAG.startState;

//    qDebug()<<"mDS:"<<mDFAStateNum;
    for(int iArr=0;iArr<mDFAStateNum;iArr++)
    {
        int rowN=mDFANodeArr[iArr].stateNum;

        table->setItem(rowN,0,new QTableWidgetItem(QString::number(rowN)));qDebug()<<"rowN"<<rowN;

        //遍历DFA节点出边
        for(int i=0;i<mDFANodeArr[rowN].edgeCount;i++)
        {
            //转到状态的列标
            int colN=OpStrList.indexOf(mDFANodeArr[rowN].edges[i].input);
//            qDebug()<<"colN"<<colN;
            int toStateN=mDFANodeArr[rowN].edges[i].tgtState;
            table->setItem(rowN,colN,new QTableWidgetItem(toStateN));
        }

        if(mDFANodeArr[rowN].isEnd)
        {
            table->setItem(rowN,colCount-1,new QTableWidgetItem("终态"));
        }

    }
}

void NDFA::printNFA(QTableWidget *table)
{
    int rowCount=NFAStateNum;//记录NFA状态数
    int epsColN=OpCharSet.size()+1;//最后一列 epsilon 列号
    int colCount=OpCharSet.size()+3;
    QStringList OpStrList=OpCharSet.values();
    std::sort(OpStrList.begin(),OpStrList.end());
    OpStrList.push_front("状态号");
    OpStrList.push_back("epsilon");
    OpStrList.push_back("初/终态");

    //ui->plainTextEdit_console->insertPlainText("NFA states' count:"+QString::number(NFAStateNum)+'\n');
    //ui->plainTextEdit_console->insertPlainText("Operators' count:"+QString::number(OpCharSet.size())+'\n');

    qDebug()<<OpStrList;
    table->setRowCount(rowCount);
    table->setColumnCount(colCount);
    /*设置表头 行*/
    table->setHorizontalHeaderLabels(OpStrList);
    /*设置表头 列*/
    for(int row=0;row<rowCount;row++)
    {
        //状态号
        table->setItem(row,0,new QTableWidgetItem(i2cMap[row]));
        //epsilon转换集合
        QString epsSetStr="";
        QSet<int>::iterator it;
        for(it=NFAStateArr[row].eSUnion.begin();it!=NFAStateArr[row].eSUnion.end();it++)
        {
            epsSetStr+=QString::number(*it)+",";
        }
        table->setItem(row,epsColN,new QTableWidgetItem(epsSetStr));
        qDebug()<<"NFAStateArr[row].eSUnion:"<<NFAStateArr[row].eSUnion;
        qDebug()<<"eps"+epsSetStr;

        //非epsilon转换
        int colN=OpStrList.indexOf(NFAStateArr[row].val);
        //qDebug()<<colN;
        if(colN != -1)
        {
            table->setItem(row,colN,new QTableWidgetItem(QString::number(NFAStateArr[row].tState)));
        }

        if(NFAStateArr[row].stateNum==NFAG.startNode->stateNum)
        {
            table->setItem(row,epsColN+1,new QTableWidgetItem("初态"));
        }

        if(NFAStateArr[row].stateNum==NFAG.endNode->stateNum)
        {
            table->setItem(row,epsColN+1,new QTableWidgetItem("终态"));
        }


        //控制台输出
        //ui->plainTextEdit_console->insertPlainText(QString::number(row)+'\n');
    }
}

void NDFA::printDFA(QTableWidget *table)
{
    QStringList OpStrList=OpCharSet.values();
    std::sort(OpStrList.begin(),OpStrList.end());
    OpStrList.push_front("状态号");
    OpStrList.push_back("初/终态");

    int rowCount=DFAStateNum;
    int colCount=OpStrList.size();
    //设置表格行列
    table->setColumnCount(colCount);
    table->setRowCount(rowCount);

    //水平表头
    table->setHorizontalHeaderLabels(OpStrList);
    table->setItem(DFAG.startState,colCount-1,new QTableWidgetItem("初态"));

    for(int iArr=0;iArr<DFAStateNum;iArr++)
    {
        int rowN=DFAStateArr[iArr].stateNum;
        table->setItem(rowN,0,new QTableWidgetItem(QString::number(rowN)));
        //遍历DFA节点出边
        for(int i=0;i<DFAStateArr[rowN].edgeCount;i++)
        {
            //转到状态的列标
            int colN=OpStrList.indexOf(DFAStateArr[rowN].edges[i].input);
            int toStateN=DFAStateArr[rowN].edges[i].tgtState;
            table->setItem(rowN,colN,new QTableWidgetItem(QString::number(toStateN)));
        }

        if(DFAStateArr[rowN].isEnd)
        {
            table->setItem(rowN,colCount-1,new QTableWidgetItem("终态"));
        }

    }
}



NDFA::NFAGraph NDFA::createNFA(QChar start,QChar end)
{
    NFAGraph n;

    n.startNode = &NFAStateArr[c2iMap[start]];
    n.endNode = &NFAStateArr[c2iMap[end]];

    return n;
}

NDFA::NFAGraph NDFA::createNFA(int start, int end)
{
    NFAGraph n;

    n.startNode = &NFAStateArr[start];
    n.endNode = &NFAStateArr[end];

    return n;
}

/**
 * @brief NDFA::add
 * @param n1
 * @param n2
 * @param ch
 * n1--ch-->n2
 */
void NDFA::add(NDFA::NFANode *n1, NDFA::NFANode *n2, QChar ch)
{
    n1->val = ch;
    n1->tState = n2->stateNum;
}

/**
 * @brief NDFA::add
 * @param n1
 * @param n2
 * n1--eps->n2
 */
void NDFA::add(NDFA::NFANode *n1, NDFA::NFANode *n2)
{
    n1->eSUnion.insert(n2->stateNum);
}

QSet<int> NDFA::e_closure(QSet<int> s)
{
    QSet<int> newSet;
    for(QSet<int>::iterator iter=s.begin();iter!=s.end();iter++)
    {
        newSet.insert(*iter);
    }

    QStack<int> eStack;

    QSet<int>::iterator it;
    for(it=newSet.begin();it!=newSet.end();it++)
    {
        eStack.push(*it);
    }

    while(!eStack.isEmpty())
    {
        int OpTmp=eStack.top(); //从栈中弹出一个元素
        eStack.pop();

        QSet<int>::iterator iter;
        for(iter=NFAStateArr[OpTmp].eSUnion.begin();iter!=NFAStateArr[OpTmp].eSUnion.end();iter++)//遍历它能够通过epsilon转换到的状态集合
        {
            if(!newSet.contains(*iter))/*若当前的元素没有在集合中出现，则将其加入集合中*/
            {
                newSet.insert(*iter);
                eStack.push(*iter);//同时压入栈中
            }
        }
    }
    return newSet;//最后得到的集合即为ε-cloure
}

QSet<int> NDFA::move_e_cloure(QSet<int> s, QChar ch)
{
    QSet<int> sMoveSet;

    QSet<int>::iterator it;
    for(it=s.begin();it!=s.end();it++)//遍历当前集合s中的每一个元素
    {
        if(NFAStateArr[*it].val==ch)//若对应转换弧上的值与ch相同
        {
            sMoveSet.insert(NFAStateArr[*it].tState);
        }
    }

    sMoveSet=e_closure(sMoveSet);//求该集合的epsilon闭包
    return sMoveSet;
}

bool NDFA::isEnd(NDFA::NFAGraph n, QSet<int> s)
{
    QSet<int>::iterator it;
    for(it = s.begin();it!=s.end();it++)/*遍历该状态所包含的NFA状态集*/
    {
        if(*it==n.endNode->stateNum)
        {
            return true;//如果包含NFA的终态，则该状态为终态，返回true
        }
    }
    return false;
}

int NDFA::findSetNum(int count, int n)
{
    for(int i=0;i<count;i++)
    {
        if(dividedSet[i].contains(n))
        {
            return i;
        }
    }
    return -2;
}

void NDFA::NFA2DFA()
{
    QSet<QSet<int>> EStatesSet;

    memset(DFAG.tranArr,-1,sizeof(DFAG.tranArr));

    QSet<QString>::iterator t;
    for(t=OpCharSet.begin();t!=OpCharSet.end();t++)
    {
        if(t->at(0).toLatin1()>='a' && t->at(0).toLatin1()<='z')//check***改全部字符
        {
            DFAG.endCharSet.insert(t->at(0).toLatin1());
        }
    }
    qDebug()<<DFAG.endCharSet;
    DFAG.startState=0;//DFA的初态为0

    QSet<int> tmpSet;
    tmpSet.insert(NFAG.startNode->stateNum);

    DFAStateArr[0].em_closure_NFA=e_closure(tmpSet);
    DFAStateArr[0].isEnd=isEnd(NFAG, DFAStateArr[0].em_closure_NFA);

    DFAStateNum++;//DFA计数加一

    QQueue<int> q;
    q.push_back(DFAG.startState);//将DFA的初态存入队列中

    while(!q.isEmpty())
    {
        int num=q.front();
        q.pop_front();//pop掉队头元素

        QSet<QChar>::iterator it;
        for(it=DFAG.endCharSet.begin();it!=DFAG.endCharSet.end();it++)
        {

            QSet<int> tmpS=move_e_cloure(DFAStateArr[num].em_closure_NFA,*it);

            if(!EStatesSet.contains(tmpS) && !tmpS.empty())
            {
                EStatesSet.insert(tmpS);

                DFAStateArr[DFAStateNum].em_closure_NFA=tmpS;

                DFAStateArr[num].edges[DFAStateArr[num].edgeCount].input=*it;
                DFAStateArr[num].edges[DFAStateArr[num].edgeCount].tgtState=DFAStateNum;
                DFAStateArr[num].edgeCount++;

                DFAG.tranArr[num][it->toLatin1()-'a']=DFAStateNum;//更新状态转移矩阵

                DFAStateArr[DFAStateNum].isEnd=isEnd(NFAG, DFAStateArr[DFAStateNum].em_closure_NFA);

                q.push_back(DFAStateNum);//将新的状态号加入队列中

                DFAStateNum++;//DFA总状态数加一
            }
            else //求出的状态集合与之前求出的某个状态集合相同
            {
                for(int i=0;i<DFAStateNum;i++)
                {

                    if(tmpS==DFAStateArr[i].em_closure_NFA)
                    {
                        DFAStateArr[num].edges[DFAStateArr[num].edgeCount].input=*it;
                        DFAStateArr[num].edges[DFAStateArr[num].edgeCount].tgtState=i;
                        DFAStateArr[num].edgeCount++;

                        DFAG.tranArr[num][it->toLatin1()-'a']=i;//更新转移矩阵

                        break;
                    }
                }
            }
        }
    }

    //求出DFA的终态集合
    for(int i=0;i<DFAStateNum;i++)//遍历该DFA的所有状态
    {
        if(DFAStateArr[i].isEnd==true) //若其为终态则加入到集合中
        {
            DFAG.endStates.insert(i);
        }
    }
}

void NDFA::DFA2mDFA()
{
    mDFAG.endCharSet=DFAG.endCharSet;//将DFA的终结符集合赋给mDFA
    //qDebug()<<mDFAG.endCharSet;

    memset(mDFAG.tranArr,-1,sizeof(mDFAG.tranArr)); //初始化mDFA的转移矩阵

    //首次划分，将终态与非终态分开
    bool allEndFlag = true;//DFA所有状态是否全为状态的标志
    for(int i=0;i<DFAStateNum;i++)
    {
        if(DFAStateArr[i].isEnd==false)//若该DFA状态非终态
        {
            allEndFlag=false;
            mDFAStateNum=2;

            dividedSet[1].insert(DFAStateArr[i].stateNum); //将该状态的状态号加入dividedSet[1]集合中
        }
        else
        {
            dividedSet[0].insert(DFAStateArr[i].stateNum); //将该状态的状态号加入dividedSet[0]集合中
        }
    }

    if(allEndFlag) //若真则所有DFA状态都为终态
    {
        mDFAStateNum=1;//第一次划分结束只有一个集合
    }

    bool newedDivFlag=true; //上一次是否产生新划分标志
    while(newedDivFlag) //只要上一次产生新的划分就继续循环
    {
        int divCount=0;
        for(int i=0;i<mDFAStateNum;i++)
        {
            QSet<QChar>::iterator it;
            for(it=DFAG.endCharSet.begin();it!=DFAG.endCharSet.end();it++)
            {
                int cacheSetNum=0;//当前状态集合个数初始化
                stateSet tmpStSet[20];//划分状态集的 缓冲区

                QSet<int>::iterator iter;
                for(iter=dividedSet[i].begin();iter!=dividedSet[i].end();iter++)
                {
                    bool epFlag=true;//判断该集合是否存在没有该终结符对应的转换弧的状态
                    for(int j=0;j<DFAStateArr[*iter].edgeCount;j++)//遍历该状态的所有边
                    {
                        if(DFAStateArr[*iter].edges[j].input==*it)
                        {
                            epFlag=false; //则标志为false

                            //计算该状态转换到的状态集的标号
                            int tranSetNum=findSetNum(mDFAStateNum,DFAStateArr[*iter].edges[j].tgtState);

                            int curSetNum=0;//遍历缓冲区，查找是否存在到达这个标号的状态集
                            while((tmpStSet[curSetNum].stateSetNum!=tranSetNum)&&(curSetNum<cacheSetNum))
                            {
                                curSetNum++;
                            }

                            if(curSetNum==cacheSetNum)
                            {
                                //缓冲区中新建一个状态集
                                tmpStSet[cacheSetNum].stateSetNum=tranSetNum;//将该状态集所能转换到的状态集标号为tranSetNum
                                tmpStSet[cacheSetNum].DFAStateSet.insert(*iter);//将当前状态添加到该状态集合中

                                cacheSetNum++;//缓冲区状态集计数增加
                            }
                            else //缓冲区中存在到达这个标号的状态集
                            {
                                tmpStSet[curSetNum].DFAStateSet.insert(*iter);//将当前状态集加入到该状态集中
                            }
                        }
                    }

                    //若该状态不存在与该终结符对应的转换弧
                    if(epFlag)
                    {
                        /*寻找缓冲区中是否存在转换到标号为-1的状态集，这里规定如果不存在转换弧，则它所到达的状态集标号为-1*/
                        int curSetNum=0;
                        while((tmpStSet[curSetNum].stateSetNum!=-1)&&(curSetNum<cacheSetNum))
                        {
                            curSetNum++;
                        }

                        if(curSetNum==cacheSetNum)//若不存在这样的状态集
                        {
                            tmpStSet[cacheSetNum].stateSetNum=-1;//将该状态集转移到状态集标号-1
                            tmpStSet[cacheSetNum].DFAStateSet.insert(*iter);

                            cacheSetNum++;//缓冲区状态集计数增加
                        }
                        else
                        {
                            tmpStSet[curSetNum].DFAStateSet.insert(*iter);//将当前状态加入到该状态集中
                        }
                    }
                }

                if(cacheSetNum>1)//r若缓冲区中的状态集个数大于1，集表示同一个状态集中的元素可以转换到不同的状态集，需要进行划分
                {
                    divCount++; //划分计数增加

                    //为每个划分创建新的DFA状态
                    for(int j=1;j<cacheSetNum;j++)
                    {
                        QSet<int>::iterator it;
//                        qDebug()<<tmpStSet[j].DFAStateSet;
                        for(it=tmpStSet[j].DFAStateSet.begin();it!=tmpStSet[j].DFAStateSet.end();it++)
                        {
                            //int tmp=*it;
                            dividedSet[i]-={*it};
                            //dividedSet[i].erase(it);//这一句直接报错非法下标，不清楚原因
                            dividedSet[mDFAStateNum].insert(*it);
                        }
                        mDFAStateNum++;//最小化DFA状态总数增加
                    }
                }
            }
        }

        //若需要划分的次数为0，则表示本次不需要进行划分
        if(divCount==0)
        {
            newedDivFlag=false;
        }
    }

    //遍历每一个划分好的状态集
    for(int i=0;i<mDFAStateNum;i++)
    {
        QSet<int>::iterator itr;
        for(itr=dividedSet[i].begin();itr!=dividedSet[i].end();itr++)//遍历集合中的每一个元素
        {
            //若当前状态为DFA的初态，则该最小化DFA状态亦为初态
            if(*itr==DFAG.startState)
            {
                mDFAG.startState=i;
            }

            if(DFAG.endStates.contains(*itr))
            {
                mDFANodeArr[i].isEnd=true;
                mDFAG.endStates.insert(i);
            }

            //遍历该DFA状态的每条弧，为最小化DFA创建弧
            for(int j=0;j<DFAStateArr[*itr].edgeCount;j++)
            {
                //遍历划分好的状态集合，找出该弧转移到的状态现在属于哪个集合
                for(int t=0;t<mDFAStateNum;t++)
                {
                    if(dividedSet[t].contains(DFAStateArr[*itr].edges[j].tgtState))
                    {
                        bool hadEdge=false;
                        for(int l=0;l<mDFANodeArr[i].edgeCount;l++)
                        {
                            if((mDFANodeArr[i].edges[l].input==DFAStateArr[*itr].edges[j].input)&&(mDFANodeArr[i].edges[l].tgtState==t))
                            {
                                hadEdge=true;//标志为真
                            }
                        }

                        if(!hadEdge)//若该弧不存在，则创建一条新的弧
                        {
                            mDFANodeArr[i].edges[mDFANodeArr[i].edgeCount].input=DFAStateArr[*itr].edges[j].input;
                            mDFANodeArr[i].edges[mDFANodeArr[i].edgeCount].tgtState=t;//该弧转移到的状态为这个状态集合的标号

                            mDFAG.tranArr[i][DFAStateArr[*itr].edges[j].input.toLatin1()-'a']=t; //更新转移矩阵

                            mDFANodeArr[i].edgeCount++; //该状态的弧的计数增加
                        }
                        break;
                    }
                }
            }
        }
    }
}
