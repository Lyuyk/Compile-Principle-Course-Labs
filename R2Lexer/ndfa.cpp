/****************************************************
 * @Copyright © 2021-2023 Lyuyk. All rights reserved.
 *
 * @FileName: ndfa.cpp
 * @Brief: NFA、DFA类源文件
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

    for(int i=0;i<ARR_MAX_NUM;i++)
    {
        NFAStateArr[i].stateNum=i;
        NFAStateArr[i].val='#';
        NFAStateArr[i].tState=-1;
        NFAStateArr[i].epsToSet.clear();
    }
    for(int i=0;i<ARR_MAX_NUM;i++)
    {
        DFAStateArr[i].stateNum=i;
        DFAStateArr[i].isEnd=false;
        DFAStateArr[i].edgeCount=0;
        DFAStateArr[i].em_closure_NFA.clear();

        for(int j=0;j<DFA_NODE_EDGE_COUNT;j++)
        {
            DFAStateArr[i].edges[j].value='#';
            DFAStateArr[i].edges[j].toState=-1;
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
            mDFANodeArr[i].edges[j].value='#';
            mDFANodeArr[i].edges[j].toState=-1;
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
        NFAStateArr[i].epsToSet.clear();
    }
    for(int i=0;i<ARR_MAX_NUM;i++)
    {
        DFAStateArr[i].stateNum=i;
        DFAStateArr[i].isEnd=false;
        DFAStateArr[i].edgeCount=0;
        DFAStateArr[i].em_closure_NFA.clear();

        for(int j=0;j<DFA_NODE_EDGE_COUNT;j++)
        {
            DFAStateArr[i].edges[j].value='#';
            DFAStateArr[i].edges[j].toState=-1;
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
            mDFANodeArr[i].edges[j].value='#';
            mDFANodeArr[i].edges[j].toState=-1;
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
        for(it=NFAStateArr[row].epsToSet.begin();it!=NFAStateArr[row].epsToSet.end();it++)
        {
            epsSetStr+=QString::number(*it)+",";
        }
        table->setItem(row,epsColN,new QTableWidgetItem(epsSetStr));
        qDebug()<<"NFAStateArr[row].eSUnion:"<<NFAStateArr[row].epsToSet;
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
            int colN=OpStrList.indexOf(DFAStateArr[rowN].edges[i].value);
            int toStateN=DFAStateArr[rowN].edges[i].toState;
            table->setItem(rowN,colN,new QTableWidgetItem(QString::number(toStateN)));
        }

        if(DFAStateArr[rowN].isEnd)
        {
            table->setItem(rowN,colCount-1,new QTableWidgetItem("终态"));
        }

    }
}

void NDFA::printMDFA(QTableWidget *table)
{
    QStringList OpStrList=OpCharSet.values();
    std::sort(OpStrList.begin(),OpStrList.end());
    //OpStrList.push_front("状态集元素");
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

    qDebug()<<"mDS:"<<mDFAStateNum;
    for(int iArr=0;iArr<mDFAStateNum;iArr++)
    {
        int rowN=mDFANodeArr[iArr].stateNum;

        table->setItem(rowN,0,new QTableWidgetItem(QString::number(rowN)));

        //遍历DFA节点出边
        for(int i=0;i<mDFANodeArr[rowN].edgeCount;i++)
        {
            //转到状态的列标
            int colN=OpStrList.indexOf(mDFANodeArr[rowN].edges[i].value);
            qDebug()<<"colN"<<colN;
            int toStateN=mDFANodeArr[rowN].edges[i].toState;
            table->setItem(rowN,colN,new QTableWidgetItem(QString::number(toStateN)));
        }

        if(mDFANodeArr[rowN].isEnd)
        {
            table->setItem(rowN,colCount-1,new QTableWidgetItem("终态"));
        }
    }
}

/**
 * @brief NDFA::insert
 * @param s
 * @param n
 * @param ch
 * 在字符串s的n下标插入新字符ch
 */
void NDFA::insert(QString &s, int n, QChar ch)
{
    s += '#';

    for(int i = s.size() - 1; i > n; i--)
    {
        s[i] = s[i - 1];
    }

    s[n] = ch;
}

/**
 * @brief NDFA::preProcess
 * @param str
 * @return s
 * 正则表达式增加对连接操作识别的预处理，对源字符串str返回处理后的的字符串s
 */
QString NDFA::preProcess(QString str)
{
    int i = 0 , length = str.size();
    QString s=str;

    while(i < length)
    {
        if((s.at(i).isLower()) || (s.at(i) == '*') || (s.at(i) == ')'))
        {
            if((s[i + 1].isLower()) || s[i + 1] == '(')
            {
                s=s.insert(i+1,'&');
                //insert(s, i+1 , '&');
                length ++;
            }
        }
        i++;
    }
    return s;
}

int NDFA::priority(QChar ch)
{

    if(ch == '*')
    {
        return 3;
    }

    if(ch == '?')
    {
        return 3;
    }

    if(ch == '+')
    {
        return 3;
    }

    if(ch == '&')
    {
        return 2;
    }

    if(ch == '|')
    {
        return 1;
    }

    if(ch == '(')
    {
        return 0;
    }

    return -1;
}

QString NDFA::in2Suffix(QString s)
{
    preProcess(s);			/*对字符串进行预处理*/

    QString str;		    /*要输出的后缀字符串*/
    QStack<QChar> opStack;	/*运算符栈*/

    for(int i = 0; i < s.size(); i++)
    {
        //操作数不处理
        if(s.at(i).isLower())
        {
            str += s.at(i);
        }
        else  /*遇到运算符时*/
        {
            if(s.at(i) == '(')		/*遇到左括号压入栈中*/
            {
                opStack.push(s.at(i));
            }
            else if(s.at(i) == ')')	/*遇到右括号时*/
            {
                QChar ch = opStack.top();
                while(ch != '(')	/*将栈中元素出栈，直到栈顶为左括号*/
                {
                    str += ch;
                    opStack.pop();
                    ch = opStack.top();
                }
                opStack.pop();		/*最后将左括号出栈*/
            }
            else					/*遇到其他操作符时*/
            {
                if(!opStack.empty())			/*如果栈不为空*/
                {
                    QChar ch = opStack.top();
                    while(priority(ch) >= priority(s.at(i)))	/*弹出栈中优先级大于等于当前运算符的运算符*/
                    {

                        str +=	ch;
                        opStack.pop();

                        if(opStack.empty())	/*如果栈为空则结束循环*/
                        {
                            break;
                        }
                        else ch = opStack.top();
                    }

                    opStack.push(s.at(i));		/*再将当前运算符入栈*/
                }

                else				/*如果栈为空，直接将运算符入栈*/
                {
                    opStack.push(s.at(i));
                }
            }
        }
    }

    /*最后如果栈不为空，则出栈并输出到字符串*/
    while(!opStack.empty())
    {

        QChar ch = opStack.top();
        opStack.pop();

        str += ch;
    }

    return str;
}

/**
 * @brief NDFA::strToNfa
 * 将正则表达式转换为NFA
 * @param s
 * @return
 */
NDFA::NFAGraph NDFA::strToNfa(QString s)
{
    s=preProcess(s);
    //存NFA的栈
    QStack<NFAGraph> NfaStack;
    //符号栈
    QStack<QChar> charStack;

    for(int i=0;i<s.size();i++)
    {
        QChar cur=s.at(i);
        if(cur.isLower())//操作数
        {
            OpCharSet.insert(cur);
            NFAGraph n=createNFA(NFAStateNum);
            NFAStateNum+=2;

            add(n.startNode, n.endNode, cur);

            NfaStack.push(n);
        }
        else if(cur=='(')
        {
            charStack.push(cur);
        }
        else if(cur=='*')
        {

        }
    }

//    for(int i = 0; i < s.size(); i++)
//    {
//        //操作数的处理
//        if(s.at(i).isLower())
//        {
//            OpCharSet.insert(s.at(i));
//            NFAGraph n = createNFA(NFAStateNum);
//            NFAStateNum += 2;//开始与结束节点

//            add(n.startNode, n.endNode, s.at(i));//NFA的头指向尾，弧上的值为s.at(i)

//            NfaStack.push(n);
//        }
//        else if(s.at(i) == '*')		//闭包运算处理
//        {

//            NFAGraph n1 = createNFA(NFAStateNum);
//            NFAStateNum += 2;

//            NFAGraph n2 = NfaStack.top();
//            NfaStack.pop();

//            add(n2.endNode, n2.startNode);
//            add(n2.endNode, n1.endNode);
//            add(n1.startNode, n2.startNode);
//            add(n1.startNode, n1.endNode);

//            //新NFA入栈
//            NfaStack.push(n1);
//        }
//        else if(s.at(i) == '+')//正闭包的处理
//        {

//            NFAGraph n1 = createNFA(NFAStateNum);
//            NFAStateNum += 2;

//            NFAGraph n2 = NfaStack.top();
//            NfaStack.pop();

//            add(n2.endNode, n2.startNode);
//            add(n2.endNode, n1.endNode);
//            add(n1.startNode, n2.startNode);
//            //与*相比只少了一条从n1.startNode->n1.endNode的epsilon边
//            NfaStack.push(n1);
//        }
//        else if(s.at(i) == '?')//可选运算符的处理
//        {

//            NFAGraph n1 = createNFA(NFAStateNum);
//            NFAStateNum += 2;

//            NFAGraph n2 = NfaStack.top();
//            NfaStack.pop();

//            add(n2.endNode, n1.endNode);
//            add(n1.startNode, n2.startNode);
//            add(n1.startNode, n1.endNode);

//            NfaStack.push(n1);
//        }
//        else if(s.at(i) == '|')		/*遇到或运算符*/
//        {

//            NFAGraph n1, n2;							/*从栈中弹出两个NFA，栈顶为n2，次栈顶为n1*/
//            n2 = NfaStack.top();
//            NfaStack.pop();

//            n1 = NfaStack.top();
//            NfaStack.pop();

//            NFAGraph n = createNFA(NFAStateNum);
//            NFAStateNum +=2;

//            add(n.startNode, n1.startNode);
//            add(n.startNode, n2.startNode);
//            add(n1.endNode, n.endNode);
//            add(n2.endNode, n.endNode);

//            NfaStack.push(n);					/*最后将新生成的NFA入栈*/
//        }
//        else if(s.at(i) == '&')//连接运算的处理
//        {

//            NFAGraph n1, n2, n;

//            n2 = NfaStack.top();
//            NfaStack.pop();

//            n1 = NfaStack.top();
//            NfaStack.pop();

//            add(n1.endNode, n2.startNode);

//            n.startNode = n1.startNode;
//            n.endNode = n2.endNode;

//            NfaStack.push(n);
//        }
//    }
    return NfaStack.top();
}

/**
 * @brief NDFA::createNFA
 * @param stateN
 * @return n
 * 建立一个初始化的NFA子图，包含开始和结尾两NFA节点
 */
NDFA::NFAGraph NDFA::createNFA(int stateN)
{
    NFAGraph n;

    n.startNode=&NFAStateArr[stateN];
    n.endNode=&NFAStateArr[stateN+1];

    return n;
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
 * NFA节点n1与n2间添加一条非epsilon边，操作符为ch
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
 * NFA节点n1与n2间添加一条epsilon边，信息记录于n1节点中
 */
void NDFA::add(NDFA::NFANode *n1, NDFA::NFANode *n2)
{
    n1->epsToSet.insert(n2->stateNum);
}


/**
 * @brief NDFA::e_closure
 * 求一个状态集的ε-cloure
 * @param s
 * @return
 */
QSet<int> NDFA::e_closure(QSet<int> s)
{
    QSet<int> newSet;
    QStack<int> eStack;

    for(const auto &iter:s)
    {
        newSet.insert(iter);
        eStack.push(iter);
    }

    while(!eStack.isEmpty())
    {
        int OpTmp=eStack.top(); //从栈中弹出一个元素
        eStack.pop();

        for(const auto &iter : qAsConst(NFAStateArr[OpTmp].epsToSet))//遍历它能够通过epsilon转换到的状态集合
        {
            if(!newSet.contains(iter))/*若当前的元素没有在集合中出现，则将其加入集合中*/
            {
                newSet.insert(iter);
                eStack.push(iter);//同时压入栈中
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

/**
 * @brief MainWindow::findSetNum
 * 当前的划分总数为count，返回状态n所属于的状态集合标号i
 * @param count
 * @param n
 * @return i (-2 means error occured)
 */
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

                DFAStateArr[num].edges[DFAStateArr[num].edgeCount].value=*it;
                DFAStateArr[num].edges[DFAStateArr[num].edgeCount].toState=DFAStateNum;
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
                        DFAStateArr[num].edges[DFAStateArr[num].edgeCount].value=*it;
                        DFAStateArr[num].edges[DFAStateArr[num].edgeCount].toState=i;
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

/**
 * @brief NDFA::DFA2mDFA
 * DFA的最小化
 */
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
                        if(DFAStateArr[*iter].edges[j].value==*it)
                        {
                            epFlag=false; //则标志为false

                            //计算该状态转换到的状态集的标号
                            int tranSetNum=findSetNum(mDFAStateNum,DFAStateArr[*iter].edges[j].toState);

                            int curSetNum=0;//遍历缓冲区，查找是否存在到达这个标号的状态集
                            while((tmpStSet[curSetNum].toStateSetNum!=tranSetNum)&&(curSetNum<cacheSetNum))
                            {
                                curSetNum++;
                            }

                            if(curSetNum==cacheSetNum)
                            {
                                //缓冲区中新建一个状态集
                                tmpStSet[cacheSetNum].toStateSetNum=tranSetNum;//将该状态集所能转换到的状态集标号为tranSetNum
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
                        while((tmpStSet[curSetNum].toStateSetNum!=-1)&&(curSetNum<cacheSetNum))
                        {
                            curSetNum++;
                        }

                        if(curSetNum==cacheSetNum)//若不存在这样的状态集
                        {
                            tmpStSet[cacheSetNum].toStateSetNum=-1;//将该状态集转移到状态集标号-1
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
                    if(dividedSet[t].contains(DFAStateArr[*itr].edges[j].toState))
                    {
                        bool hadEdge=false;
                        for(int l=0;l<mDFANodeArr[i].edgeCount;l++)
                        {
                            if((mDFANodeArr[i].edges[l].value==DFAStateArr[*itr].edges[j].value)&&(mDFANodeArr[i].edges[l].toState==t))
                            {
                                hadEdge=true;//标志为真
                            }
                        }

                        if(!hadEdge)//若该弧不存在，则创建一条新的弧
                        {
                            mDFANodeArr[i].edges[mDFANodeArr[i].edgeCount].value=DFAStateArr[*itr].edges[j].value;
                            mDFANodeArr[i].edges[mDFANodeArr[i].edgeCount].toState=t;//该弧转移到的状态为这个状态集合的标号

                            mDFAG.tranArr[i][DFAStateArr[*itr].edges[j].value.toLatin1()-'a']=t; //更新转移矩阵

                            mDFANodeArr[i].edgeCount++; //该状态的弧的计数增加
                        }
                        break;
                    }
                }
            }
        }
    }
}
