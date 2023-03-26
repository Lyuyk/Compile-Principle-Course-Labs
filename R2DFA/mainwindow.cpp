/*====================================================

  Copyright(C) 2022-2023 Lyuyk
  All rights reserved

  文件名称:mainwindow.cpp
  摘要:主窗口源文件
  当前版本号:v1.0
  版本历史信息：无

  created by Lyuyk at 10/18/2021

=====================================================*/
#include "mainwindow.h"
#include "ui_mainwindow.h"
//Qt lib
#include <QHash>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
//std lib
#include <vector>
#include <queue>
#include <set>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /*菜单栏与槽函数连接*/
    connect(ui->action_exit,SIGNAL(triggered()),this,SLOT(exit()));
    connect(ui->action_openFile,SIGNAL(triggered()),this,SLOT(on_pushButton_openRegexFile_clicked()));
    connect(ui->action_saveFile,SIGNAL(triggered()),this,SLOT(on_pushButton_saveRegexToFile_clicked()));
    connect(ui->action_2NFA,SIGNAL(triggered()),this,SLOT(on_pushButton_2NFA_clicked()));
    connect(ui->action_2DFA,SIGNAL(triggered()),this,SLOT(on_pushButton_2DFA_clicked()));
    connect(ui->action_mDFA,SIGNAL(triggered()),this,SLOT(on_pushButton_mDFA_clicked()));

    /*表格属性设置*/
    ui->tableWidget_NFA->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget_DFA->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget_mDFA->setEditTriggers(QAbstractItemView::NoEditTriggers);

    /*界面初始化*/
    ui->tabWidget_Graph->setCurrentIndex(0);

    /*按键操作初始化*/
    ui->pushButton_2DFA->setDisabled(true);
    ui->pushButton_mDFA->setDisabled(true);

    /*一些参数的初始化*/
    regexStr="";
    NFAStateNum=0;//状态创建计数
    DFAStateNum=0;
    mDFAStateNum=0;

    for(int i=0;i< ARR_MAX_NUM;i++)
    {
        NFAStateArr[i].stateNum=i;
        NFAStateArr[i].val='#';
        NFAStateArr[i].tState=-1;
    }
    for(int i=0;i<ARR_MAX_NUM;i++)
    {
        DFAStateArr[i].stateNum=i;
        DFAStateArr[i].isEnd=false;

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

        for(int j=0;j<DFA_NODE_EDGE_COUNT;j++)
        {
            mDFANodeArr[i].edges[j].input='#';
            mDFANodeArr[i].edges[j].tgtState=-1;
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::exit()
{
    window()->close();
}

/**
 * @brief MainWindow::on_pushButton_openRegexFile_clicked
 * 打开正则表达式文件
 */
void MainWindow::on_pushButton_openRegexFile_clicked()
{
    QString srcFilePath=QFileDialog::getOpenFileName(this,"选择正则表达式文件（仅.txt类型）","/", "Files(*.txt)");
    QFile srcFile(srcFilePath);
    if(!srcFile.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QMessageBox::warning(NULL, "文件", "文件打开失败");
        return;
    }
    QTextStream textInput(&srcFile);
    textInput.setEncoding(QStringConverter::Utf8);//设置编码，防止中文乱码

    QString line;
    ui->plainTextEdit_console->insertPlainText("读取正则表达式文件...\n");
    while(!textInput.atEnd())
    {
        line=textInput.readLine().toUtf8();//按行读取文件
        regexStr.append(line.trimmed());
        ui->lineEdit_Regex->insert(line);//一行行显示
        regexStr+=line;//初始化regexStr
    }
    ui->plainTextEdit_console->insertPlainText("读取完成...\n");
    srcFile.close();
}

/**
 * @brief MainWindow::on_pushButton_saveRegexToFile_clicked
 * 保存正则表达式到文件（覆盖写入）
 */
void MainWindow::on_pushButton_saveRegexToFile_clicked()
{
    QString tgtFilePath=QFileDialog::getSaveFileName(this,"选择保存路径","/");
    QFile tgtFile(tgtFilePath);
    if(!tgtFile.open(QIODevice::ReadWrite|QIODevice::Text|QIODevice::Truncate))
    {
        QMessageBox::warning(NULL, "文件", "文件打开/写入失败");
        return;
    }
    QTextStream outputFile(&tgtFile);
    QString tgStr=ui->lineEdit_Regex->text();
    outputFile<<tgStr;
    tgtFile.close();
    QMessageBox::information(NULL,"文件","文件保存成功(saveRegex)");
}


/**
 * @brief createNFA
 * 创建NFA图
 * @return NFAGraph*
 */
NFAGraph MainWindow::createNFA(int stateN)
{
    NFAGraph n;

    n.startNode = &NFAStateArr[stateN];
    n.endNode = &NFAStateArr[stateN + 1];

    return n;
}

void MainWindow::insert(QString &s, int n, QChar ch)
{

    s += '#';

    for(int i = s.size() - 1; i > n; i--)
    {
        s[i] = s[i - 1];
    }

    s[n] = ch;
}

void MainWindow::preProcess(QString &s)
{

    int i = 0 , length = s.size();

    while(i < length)
    {
        if((s.at(i) >= 'a' && s.at(i) <= 'z') || (s.at(i) == '*') || (s.at(i) == ')'))
        {
            if((s[i + 1] >= 'a' && s[i + 1] <= 'z') || s[i + 1] == '(')
            {

                insert(s, i+1 , '&');
                length ++;
            }
        }

        i++;
    }
}

int MainWindow::priority(QChar ch)
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

QString MainWindow::in2Suffix(QString s)
{

    preProcess(s);			/*对字符串进行预处理*/

    QString str;				/*要输出的后缀字符串*/
    QStack<QChar> oper;		/*运算符栈*/

    for(int i = 0; i < s.size(); i++)
    {
        //操作数不处理
        if(s.at(i) >= 'a' && s.at(i) <= 'z')
        {
            str += s.at(i);
        }
        else							/*遇到运算符时*/
        {

            if(s.at(i) == '(')			/*遇到左括号压入栈中*/
            {
                oper.push(s.at(i));
            }

            else if(s.at(i) == ')')	/*遇到右括号时*/
            {

                QChar ch = oper.top();
                while(ch != '(')		/*将栈中元素出栈，直到栈顶为左括号*/
                {

                    str += ch;

                    oper.pop();
                    ch = oper.top();
                }

                oper.pop();				/*最后将左括号出栈*/
            }
            else					/*遇到其他操作符时*/
            {
                if(!oper.empty())			/*如果栈不为空*/
                {

                    QChar ch = oper.top();
                    while(priority(ch) >= priority(s.at(i)))	/*弹出栈中优先级大于等于当前运算符的运算符*/
                    {

                        str +=	ch;
                        oper.pop();

                        if(oper.empty())	/*如果栈为空则结束循环*/
                        {
                            break;
                        }
                        else ch = oper.top();
                    }

                    oper.push(s.at(i));		/*再将当前运算符入栈*/
                }

                else				/*如果栈为空，直接将运算符入栈*/
                {
                    oper.push(s.at(i));
                }
            }
        }
    }

    /*最后如果栈不为空，则出栈并输出到字符串*/
    while(!oper.empty())
    {

        QChar ch = oper.top();
        oper.pop();

        str += ch;
    }

    ui->plainTextEdit_console->insertPlainText("infix regex(origin):"+s+'\n');
    ui->plainTextEdit_console->insertPlainText("suffix(processed):"+str+'\n');

    return str;
}

/*从状态n1到状态n2添加一条弧，弧上的值为ch*/
void MainWindow::add(NFANode *n1, NFANode *n2, QChar ch)
{
    n1->val = ch;
    n1->tState = n2->stateNum;
}

/*从状态n1到状态n2添加一条弧，弧上的值为ε*/
void MainWindow::add(NFANode *n1, NFANode *n2)
{
    n1->eSUnion.insert(n2->stateNum);
    qDebug()<<n2->stateNum<<"fk";
}

NFAGraph MainWindow::strToNfa(QString s)
{
    //存NFA的栈
    QStack<NFAGraph> NfaStack;

    for(int i = 0; i < s.size(); i++)
    {
        //操作数的处理
        if(s.at(i) >= 'a' && s.at(i) <= 'z')
        {
            OpCharSet.insert(s.at(i));
            NFAGraph n = createNFA(NFAStateNum);
            NFAStateNum += 2;//开始与结束节点
            //NFA的头指向尾，弧上的值为s.at(i)
            add(n.startNode, n.endNode, s.at(i));

            NfaStack.push(n);
        }
        else if(s.at(i) == '*')		//闭包运算处理
        {

            NFAGraph n1 = createNFA(NFAStateNum);
            NFAStateNum += 2;

            NFAGraph n2 = NfaStack.top();
            NfaStack.pop();

            add(n2.endNode, n2.startNode);
            add(n2.endNode, n1.endNode);
            add(n1.startNode, n2.startNode);
            add(n1.startNode, n1.endNode);

            //新NFA入栈
            NfaStack.push(n1);
        }
        else if(s.at(i) == '+')//正闭包的处理
        {

            NFAGraph n1 = createNFA(NFAStateNum);
            NFAStateNum += 2;

            NFAGraph n2 = NfaStack.top();
            NfaStack.pop();

            add(n2.endNode, n2.startNode);
            add(n2.endNode, n1.endNode);
            add(n1.startNode, n2.startNode);
            //与*相比只少了一条从n1.startNode->n1.endNode的epsilon边
            NfaStack.push(n1);
        }
        else if(s.at(i) == '?')//可选运算符的处理
        {

            NFAGraph n1 = createNFA(NFAStateNum);
            NFAStateNum += 2;

            NFAGraph n2 = NfaStack.top();
            NfaStack.pop();

            add(n2.endNode, n1.endNode);
            add(n1.startNode, n2.startNode);
            add(n1.startNode, n1.endNode);

            NfaStack.push(n1);
        }
        else if(s.at(i) == '|')		/*遇到或运算符*/
        {

            NFAGraph n1, n2;							/*从栈中弹出两个NFA，栈顶为n2，次栈顶为n1*/
            n2 = NfaStack.top();
            NfaStack.pop();

            n1 = NfaStack.top();
            NfaStack.pop();

            NFAGraph n = createNFA(NFAStateNum);
            NFAStateNum +=2;

            add(n.startNode, n1.startNode);
            add(n.startNode, n2.startNode);
            add(n1.endNode, n.endNode);
            add(n2.endNode, n.endNode);

            NfaStack.push(n);					/*最后将新生成的NFA入栈*/
        }
        else if(s.at(i) == '&')//连接运算的处理
        {

            NFAGraph n1, n2, n;

            n2 = NfaStack.top();
            NfaStack.pop();

            n1 = NfaStack.top();
            NfaStack.pop();

            add(n1.endNode, n2.startNode);

            n.startNode = n1.startNode;
            n.endNode = n2.endNode;

            NfaStack.push(n);
        }
    }
    return NfaStack.top();		/*最后的栈顶元素即为生成好的NFA*/
}



void MainWindow::on_pushButton_2NFA_clicked()
{
    regexStr=ui->lineEdit_Regex->text();//home
    NFAG=strToNfa(in2Suffix(regexStr));

//====================================================
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
    ui->tableWidget_NFA->setRowCount(rowCount);
    ui->tableWidget_NFA->setColumnCount(colCount);
    /*设置表头 行*/
    ui->tableWidget_NFA->setHorizontalHeaderLabels(OpStrList);
    /*设置表头 列*/
    for(int row=0;row<rowCount;row++)
    {
        //状态号
        ui->tableWidget_NFA->setItem(row,0,new QTableWidgetItem(QString::number(row)));
        //epsilon转换集合
        QString epsSetStr="";
        QSet<int>::iterator it;
        for(it=NFAStateArr[row].eSUnion.begin();it!=NFAStateArr[row].eSUnion.end();it++)
        {
            epsSetStr+=QString::number(*it)+",";
        }
        ui->tableWidget_NFA->setItem(row,epsColN,new QTableWidgetItem(epsSetStr));
        qDebug()<<NFAStateArr[row].eSUnion;
        qDebug()<<"eps"+epsSetStr;

        //非epsilon转换
        int colN=OpStrList.indexOf(NFAStateArr[row].val);
        //qDebug()<<colN;
        if(colN != -1)
        {
            ui->tableWidget_NFA->setItem(row,colN,new QTableWidgetItem(QString::number(NFAStateArr[row].tState)));
        }

        if(NFAStateArr[row].stateNum==NFAG.startNode->stateNum)
        {
            ui->tableWidget_NFA->setItem(row,epsColN+1,new QTableWidgetItem("初态"));
        }

        if(NFAStateArr[row].stateNum==NFAG.endNode->stateNum)
        {
            ui->tableWidget_NFA->setItem(row,epsColN+1,new QTableWidgetItem("终态"));
        }


        //控制台输出
        //ui->plainTextEdit_console->insertPlainText(QString::number(row)+'\n');
    }

    ui->pushButton_2DFA->setEnabled(true);//完成转换工作后可以允许NFA到DFA的转换工作
}

/**
 * @brief MainWindow::e_cloure
 * 求一个状态集的ε-cloure
 * @param s
 * @return
 */
QSet<int> MainWindow::e_closure(QSet<int> s)
{
    QSet<int> newSet;
    QStack<int> eStack;

    for(const auto &iter:s)
    {
        newSet.insert(iter);
        eStack.push(iter);
    }

//    QSet<int>::iterator it;
//    for(it=newSet.begin();it!=newSet.end();it++)
//    {
//        eStack.push(*it);
//    }
//    qDebug()<<"e_cl";

    while(!eStack.isEmpty())
    {
        int OpTmp=eStack.top(); //从栈中弹出一个元素
        eStack.pop();

        for(const auto &iter : qAsConst(NFAStateArr[OpTmp].eSUnion))//遍历它能够通过epsilon转换到的状态集合
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

/**
 * @brief MainWindow::move_e_cloure
 * 求一个状态集s的ε-cloure(move(ch))
 * @param s
 * @param ch
 * @return
 */
QSet<int> MainWindow::move_e_cloure(QSet<int> s, QChar ch)
{

    //QSet<int> stdSMoveSet;
    QSet<int> sMoveSet;

    //qDebug()<<"m_e_cl";
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

/**
 * @brief MainWindow::isEnd
 * 判断一个状态集合是否为终态
 * @param n
 * @param s
 * @return
 */
bool MainWindow::isEnd(NFAGraph n, QSet<int> s)
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

void MainWindow::on_pushButton_2DFA_clicked()
{
    //ui->plainTextEdit_console->insertPlainText("***Change NFA to DFA***");
    //DFAGraph DFAG;
    QSet<QSet<int>> EStatesSet;

    memset(DFAG.tranArr,-1,sizeof(DFAG.tranArr));

//    QSet<QString>::iterator t;
    for(const auto &t : qAsConst(OpCharSet))
    {
        if(t.at(0).toLatin1()>='a' && t.at(0).toLatin1()<='z')//check***改全部字符
        {
            DFAG.endCharSet.insert(t.at(0).toLatin1());
        }
        //qDebug()<<DFAG.endCharSet;
    }
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
        //qDebug()<<"1";


        for(const auto &it : DFAG.endCharSet)
        {

            QSet<int> tmpS=move_e_cloure(DFAStateArr[num].em_closure_NFA,it);

            if(!EStatesSet.contains(tmpS) && !tmpS.empty())
            {
                EStatesSet.insert(tmpS);
                //qDebug()<<"OMG";
                //qDebug()<<tmpS;
                DFAStateArr[DFAStateNum].em_closure_NFA=tmpS;
                qDebug()<<"tmpS";
                DFAStateArr[num].edges[DFAStateArr[num].edgeCount].input=it;
                DFAStateArr[num].edges[DFAStateArr[num].edgeCount].tgtState=DFAStateNum;
                DFAStateArr[num].edgeCount++;

                DFAG.tranArr[num][it.toLatin1()-'a']=DFAStateNum;//更新状态转移矩阵

                DFAStateArr[DFAStateNum].isEnd=isEnd(NFAG, DFAStateArr[DFAStateNum].em_closure_NFA);

                q.push_back(DFAStateNum);//将新的状态号加入队列中

                DFAStateNum++;//DFA总状态数加一
            }
            else //求出的状态集合与之前求出的某个状态集合相同
            {
                for(int i=0;i<DFAStateNum;i++)
                {
                        qDebug()<<"3";
                    if(tmpS==DFAStateArr[i].em_closure_NFA)
                    {
                        DFAStateArr[num].edges[DFAStateArr[num].edgeCount].input=it;
                        DFAStateArr[num].edges[DFAStateArr[num].edgeCount].tgtState=i;
                        DFAStateArr[num].edgeCount++;

                        DFAG.tranArr[num][it.toLatin1()-'a']=i;//更新转移矩阵

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

    /*======================显示处理========================*/
    ui->tabWidget_Graph->setCurrentIndex(1);

    QStringList OpStrList=OpCharSet.values();
    std::sort(OpStrList.begin(),OpStrList.end());
    OpStrList.push_front("状态号");
    OpStrList.push_back("初/终态");

    int rowCount=DFAStateNum;
    int colCount=OpStrList.size();
    //设置表格行列
    ui->tableWidget_DFA->setColumnCount(colCount);
    ui->tableWidget_DFA->setRowCount(rowCount);

    //水平表头
    ui->tableWidget_DFA->setHorizontalHeaderLabels(OpStrList);
    ui->tableWidget_DFA->setItem(DFAG.startState,colCount-1,new QTableWidgetItem("初态"));

    for(int iArr=0;iArr<DFAStateNum;iArr++)
    {
        int rowN=DFAStateArr[iArr].stateNum;
        ui->tableWidget_DFA->setItem(rowN,0,new QTableWidgetItem(QString::number(rowN)));
        //遍历DFA节点出边
        for(int i=0;i<DFAStateArr[rowN].edgeCount;i++)
        {
            //转到状态的列标
            int colN=OpStrList.indexOf(DFAStateArr[rowN].edges[i].input);
            int toStateN=DFAStateArr[rowN].edges[i].tgtState;
            ui->tableWidget_DFA->setItem(rowN,colN,new QTableWidgetItem(QString::number(toStateN)));
        }

        if(DFAStateArr[rowN].isEnd)
        {
            ui->tableWidget_DFA->setItem(rowN,colCount-1,new QTableWidgetItem("终态"));
        }

    }

    /*======================交互界面处理========================*/
    //ui->pushButton_2NFA->setDisabled(true);
    ui->pushButton_mDFA->setEnabled(true);//允许最小化DFA
}

/**
 * @brief MainWindow::findSetNum
 * 当前的划分总数为count，返回状态n所属于的状态集合标号i
 * @param count
 * @param n
 * @return i (-2 means error occured)
 */
int MainWindow::findSetNum(int count, int n)
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

/**
 * @brief MainWindow::on_pushButton_mDFA_clicked
 * 最小化DFA并将其状态转换表显示出来
 */
void MainWindow::on_pushButton_mDFA_clicked()
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
    //qDebug()<<"mDFAStateNum"<<mDFAStateNum;

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
                        //qDebug()<<tmpStSet[j].DFAStateSet;
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

    /*==========显示处理=================*/
    //切换表格
    ui->tabWidget_Graph->setCurrentIndex(2);

    QStringList OpStrList=OpCharSet.values();
    std::sort(OpStrList.begin(),OpStrList.end());
    //OpStrList.push_front("状态集元素");
    OpStrList.push_front("状态集号");
    OpStrList.push_back("初/终态");

    int rowCount=mDFAStateNum;
    int colCount=OpStrList.size();
    //设置表格行列
    ui->tableWidget_mDFA->setColumnCount(colCount);
    ui->tableWidget_mDFA->setRowCount(rowCount);
    //设置表头
    ui->tableWidget_mDFA->setHorizontalHeaderLabels(OpStrList);

    //输出表格内容
    ui->tableWidget_mDFA->setHorizontalHeaderLabels(OpStrList);
    ui->tableWidget_mDFA->setItem(mDFAG.startState,colCount-1,new QTableWidgetItem("初态"));

    qDebug()<<"mDS:"<<mDFAStateNum;
    for(int iArr=0;iArr<mDFAStateNum;iArr++)
    {
        int rowN=mDFANodeArr[iArr].stateNum;

        ui->tableWidget_mDFA->setItem(rowN,0,new QTableWidgetItem(QString::number(rowN)));

        //遍历DFA节点出边
        for(int i=0;i<mDFANodeArr[rowN].edgeCount;i++)
        {
            //转到状态的列标
            int colN=OpStrList.indexOf(mDFANodeArr[rowN].edges[i].input);
            qDebug()<<"colN"<<colN;
            int toStateN=mDFANodeArr[rowN].edges[i].tgtState;
            ui->tableWidget_mDFA->setItem(rowN,colN,new QTableWidgetItem(QString::number(toStateN)));
        }

        if(mDFANodeArr[rowN].isEnd)
        {
            ui->tableWidget_mDFA->setItem(rowN,colCount-1,new QTableWidgetItem("终态"));
        }

    }

    /*==========操作处理=================*/
}

void MainWindow::on_pushButton_clearConsole_clicked()
{        
    /*界面初始化*/
    ui->tabWidget_Graph->setCurrentIndex(0);
    ui->plainTextEdit_console->clear();
    ui->lineEdit_Regex->clear();
    ui->tableWidget_NFA->clear();
    ui->tableWidget_DFA->clear();
    ui->tableWidget_mDFA->clear();

    /*按键操作初始化*/
    ui->pushButton_2NFA->setEnabled(true);
    ui->pushButton_2DFA->setDisabled(true);
    ui->pushButton_mDFA->setDisabled(true);

    /*一些参数的初始化*/
    regexStr="";
    NFAStateNum=0;//状态创建计数
    DFAStateNum=0;
    mDFAStateNum=0;
    regexStr.clear();
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
