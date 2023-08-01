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
    m_grammarStr = ""; // 暂存整个文法
    m_startChar = "";

    m_nonTmrSet.clear();
    m_tmrSet.clear();

    m_GM_productionMap.clear();
    m_LL1Table.clear();
    m_programCode.clear();
    m_parseTreeRoot = nullptr;
    m_parseTreeRootI = nullptr;
}

/**
 * @brief BNFP::InitGrammar
 * 文法初始化，BNF文法的解析与存储
 */
void BNFP::initGrammar(QString s)
{
    init();

    if (s.at(s.size() - 1) == '\n')
        s.chop(1);
    QStringList t_pdnList = s.split("\n"); // 分割出一条条产生式

    m_startChar = t_pdnList[0].split("->")[0].trimmed(); // 开始符号，为第一条产生式左部
    // qDebug()<<"m_startChar:"<<m_startChar;

    // 扫描每一行，先存左部非终结符，方便后面判断
    for (const auto &t_line : t_pdnList)
    {
        if (t_line.simplified().isEmpty())
            continue; // 略去空行

        QString t_pdnL = t_line.split("->")[0].trimmed(); // 左部
        m_nonTmrSet.append(t_pdnL);
    }

    for (const auto &t_line : t_pdnList)
    {
        if (t_line.isEmpty())
            continue;                                         // 去掉空行
        QString t_leftStr = t_line.split("->")[0].trimmed();  // 左部
        QString t_rightStr = t_line.split("->")[1].trimmed(); // 右部字符串
        QStringList t_candidateList = t_rightStr.split('|');  // 暂存储右部列表

        // qDebug()<<"t_leftStr:"<<t_leftStr;
        // 遍历右部每一条候选式字符串
        for (const auto &t_cddStr : t_candidateList)
        {
            QList<QString> t_cddList = t_cddStr.trimmed().split(' '); // 每一个单词分开
            // qDebug()<<"t_cddList:"<<t_cddList;

            m_GM_productionMap[t_leftStr].pdnRights.append(t_cddList); // 加入调
            for (const auto &t_cWord : t_cddList)                      // 对每一条候选式单词
            {
                if (!m_nonTmrSet.contains(t_cWord))
                    m_tmrSet.insert(t_cWord); // 右部如果不是非终结符，则全部当成终结符加入
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
    bool changedFlag = true;                       // 用于判断文法中的产生式是否有变化
    QSet<QString> t_reachNoEndSet = {m_startChar}; // 暂时可到达但不可终止的非终结符
    QSet<QString> t_reachEndSet = {};              // 可终止且可到达的非终结符
    while (changedFlag)
    {
        changedFlag = false;
        foreach (const QString &t_RneNonTmr, t_reachNoEndSet) // 暂时可达不可终止非终结符
        {
            // qDebug()<<"t_RneNonTmr:"<<t_RneNonTmr;
            // 遍历该产生式的所有候选式
            for (const QStringList &t_candidateList : m_GM_productionMap[t_RneNonTmr].pdnRights)
            {
                // qDebug()<<t_RneNonTmr;
                bool allEndFlag = true; // 当前候选式中的所有字符是否可终止标志

                // 遍历该候选式中的每个单词
                for (const auto &t_cWord : t_candidateList)
                {
                    // 若在非终结符集 且不在 可达&可终止集合中
                    if (m_nonTmrSet.contains(t_cWord) && !t_reachEndSet.contains(t_cWord))
                    {
                        allEndFlag = false;                     // 发现有不可终止的
                        if (!t_reachNoEndSet.contains(t_cWord)) // 如果它不在 可达&不可终止集合中，则加入
                        {
                            t_reachNoEndSet.insert(t_cWord);
                            changedFlag = true; // 产生式已经变更
                        }
                    }
                }

                if (allEndFlag)
                {

                    // 若候选式中所有单词都可达且可终止，则将当前非终结符移至 可达可终止集合
                    t_reachEndSet.insert(t_RneNonTmr);
                    t_reachNoEndSet.remove(t_RneNonTmr);
                    changedFlag = true;
                }
            }
        }
    }

    QSet<QString> t_nonTmrSet(m_nonTmrSet.begin(), m_nonTmrSet.end()); // 原来的非终结符集，用于判断
    // 若两集合不相同，则说明原来的非终结符集合有不是非终结符的，我们通过循环判断删除
    if (t_nonTmrSet != t_reachEndSet)
    {
        QSet<QString> t_deletedSet = t_nonTmrSet - t_reachEndSet; // 即将删除的非终结符集

        for (const auto &t_word : t_deletedSet)
        {
            m_nonTmrSet.remove(m_nonTmrSet.indexOf(t_word)); // 移除掉该“非终结符”
            m_GM_productionMap.remove(t_word);               // 移除该“非终结符”的产生式
        }

        // 遍历可达可终结集合
        for (const auto &t_cWord : t_reachEndSet)
        {
            QSet<QStringList> t_deletedListSet = {};
            // 遍历该产生式的所有候选式
            for (const auto &t_candidateList : m_GM_productionMap[t_cWord].pdnRights)
                for (const auto &t_candidateWord : t_candidateList)                                      // 遍历该候选式中的每个单词
                    if (t_nonTmrSet.contains(t_candidateWord) && !m_nonTmrSet.contains(t_candidateWord)) // 若当中有符号原来是在非终结符集但现在被被删除掉了
                    {
                        QStringList t_deletedList = t_candidateList;
                        t_deletedListSet.insert(t_deletedList); // 这种情况我们将这个单条候选式放入删除列表中
                    }
            // 遍历删除列表，将需要被删除的候选式删除
            for (const auto &t_delList : t_deletedListSet)
                m_GM_productionMap[t_cWord].pdnRights.removeOne(t_delList);
        }
    }

    // 防止提取左公因子多次代入后产生如X->@xxx..这样的产生式
    for (const auto &t_NT : m_GM_productionMap.keys())
    {
        for (const auto &t_cdd : m_GM_productionMap[t_NT].pdnRights)
        {
            if (t_cdd[0] == "@" && t_cdd.size() > 1)
            {
                QStringList tmp = t_cdd;
                tmp.removeFirst();

                m_GM_productionMap[t_NT].pdnRights.removeOne(t_cdd);
                m_GM_productionMap[t_NT].pdnRights.append(tmp);
            }
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

    // 遍历所有左部
    const QList<QString> &t_productionLList = m_GM_productionMap.keys();
    for (const QString &t_productionL : t_productionLList)
    {
        // 记录右部
        QString t_productionRStr;
        // 每一条候选式
        for (const auto &t_productionR : qAsConst(m_GM_productionMap[t_productionL].pdnRights))
        {
            t_productionRStr += " | ";
            // 每一个操作符
            for (const auto &t_candidate : t_productionR)
            {
                t_productionRStr += t_candidate + ' ';
            }
        }
        QString t_Line = QString(t_productionL) + " -> " + t_productionRStr.remove(0, 2); // 字符串处理去掉首个右部'|'
        grammarString += (t_Line + '\n');
    }

    e->insertPlainText(grammarString);
}

/**
 * @brief BNFP::printSet
 * @param table
 * @param isFirst
 * 输出First/Follow集到表格
 */
void BNFP::printSet(QTableWidget *table, bool isFirst)
{
    QStringList headerList = {"符号", "集合"};
    table->setRowCount(m_nonTmrSet.size()); // 行数
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels(headerList);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    if (isFirst)
    {
        for (int i = 0; i < m_nonTmrSet.size(); i++)
        {
            QString setStr = "{ ";
            for (const auto &t_first : m_GM_productionMap[m_nonTmrSet[i]].firstSet)
                setStr += t_first + " ";
            setStr += "}";
            table->setItem(i, 1, new QTableWidgetItem(setStr));         // 集合
            table->setItem(i, 0, new QTableWidgetItem(m_nonTmrSet[i])); // 非终结符
        }
    }
    else
    {
        for (int i = 0; i < m_nonTmrSet.size(); i++)
        {
            QString setStr = "{ ";
            for (const auto &t_first : m_GM_productionMap[m_nonTmrSet[i]].followSet)
                setStr += t_first + " ";
            setStr += " }";
            table->setItem(i, 1, new QTableWidgetItem(setStr));         // 集合
            table->setItem(i, 0, new QTableWidgetItem(m_nonTmrSet[i])); // 非终结符
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
    table->setRowCount(m_nonTmrSet.size());     // 行数
    table->setColumnCount(m_tmrSet.size() + 2); // 列数：空、终结符、结束符

    QStringList headerList = m_tmrSet.values(); // 终结符
    headerList.push_front("M[N,T]");
    headerList.push_back("$"); // 结束符号
    table->setHorizontalHeaderLabels(headerList);

    // 遍历非终结符
    for (int i = 0; i < m_nonTmrSet.size(); i++)
    {
        QString t_curNonTmr = m_nonTmrSet[i]; // 当前非终结符
        table->setItem(i, 0, new QTableWidgetItem(t_curNonTmr));

        // 遍历终结符及$
        int j = 1, colN = 1;
        for (; j < headerList.size(); j++)
        {
            QString t_tStr = headerList[j];
            if (m_LL1Table[t_curNonTmr].contains(t_tStr))
            {
                QStringList t_sList = m_LL1Table[t_curNonTmr][t_tStr];
                QString str = t_curNonTmr + "->";
                foreach (const auto &s, t_sList)
                    str += s + ' ';
                str.chop(1);

                table->setItem(i, colN, new QTableWidgetItem(str));
            }
            colN++;
        }
    }
}

QTreeWidgetItem *BNFP::getChildItem(parseTreeNode *parentNode, QTreeWidgetItem *parentItem)
{
    if (parentNode->children.isEmpty())
        return parentItem;

    QList<QTreeWidgetItem *> childItemList;
    foreach (const auto &child, parentNode->children) // 当前节点孩子
    {
        childItemList.append(new QTreeWidgetItem({child->value}));
        QTreeWidgetItem *childItem = getChildItem(child, parentItem);
    }
    parentItem->addChildren(childItemList);
    return parentItem;
}

/**
 * @brief BNFP::genTINYSyntaxTree
 * @param root
 * @return
 * 将分析树机械转换为语法树
 */
syntaxTreeNode *BNFP::genTINYSyntaxTree(parseTreeNode *root)
{
    if (root == nullptr)
        return nullptr;

    syntaxTreeNode *currentNode = nullptr;
    if (root->value == "program") // program->stmt-sequence
    {
        //        qDebug()<<root->value<<root->children.size();

        currentNode = new syntaxTreeNode("start");
        currentNode->addChild(genTINYSyntaxTree(root->children[0]));
    }
    else if (root->value == "stmt-sequence") // stmt-sequence->statement stmt-sequence'
    {
        //        qDebug()<<root->value<<root->children.size();

        currentNode = new syntaxTreeNode("");
        currentNode->flag = true;                                    // 要把该节点孩子提上来，先做标记后续再处理
        currentNode->addChild(genTINYSyntaxTree(root->children[0])); // statement
        currentNode->addChild(genTINYSyntaxTree(root->children[1])); // stmt-sequence'
    }
    else if (root->value == "statement") // statement->identifier := exp|write exp|repeat stmt-sequence until exp|read identifier|if exp then stmt-sequence statement'
    {
        //        qDebug()<<root->value<<root->children.size();

        if (root->children.size() == 2 && root->children[0]->value == "read") // read identifier情况
        {
            currentNode = new syntaxTreeNode("read");
            currentNode->nodeValue = root->children[1]->value; // 把identifier的值赋给该节点即可
            //            qDebug()<<root->children[1]->value;
        }
        else if (root->children.size() == 2 && root->children[0]->value == "write") // write exp情况
        {
            currentNode = new syntaxTreeNode("write");
            currentNode->addChild(genTINYSyntaxTree(root->children[1]));
        }
        else if (root->children.size() == 3) // identifier := exp情况
        {
            currentNode = new syntaxTreeNode("assign");
            currentNode->nodeValue = root->children[0]->value;
            currentNode->addChild(genTINYSyntaxTree(root->children[2]));
        }
        else if (root->children.size() == 4) // repeat stmt-sequence until exp
        {
            currentNode = new syntaxTreeNode("repeat");
            currentNode->addChild(genTINYSyntaxTree(root->children[1]));
            currentNode->addChild(genTINYSyntaxTree(root->children[3]));
        }
        else if (root->children.size() == 5) // if exp then stmt-sequence statement'
        {
            currentNode = new syntaxTreeNode("if");
            currentNode->addChild(genTINYSyntaxTree(root->children[1]));
            currentNode->addChild(genTINYSyntaxTree(root->children[3]));
            currentNode->addChild(genTINYSyntaxTree(root->children[4]));
        }
    }
    else if (root->value == "exp") // exp->( exp ) term' simple-exp' exp'|number term' simple-exp' exp'|identifier term' simple-exp' exp'
    {
        //        qDebug()<<root->value<<root->children.size();

        currentNode = new syntaxTreeNode("");
        currentNode->flag = true;       // 要把该节点孩子提上来，先做标记后续再处理
        if (root->children.size() == 6) //( exp ) term' simple-exp' exp'情况
        {
            currentNode->addChild(genTINYSyntaxTree(root->children[1]));
            currentNode->addChild(genTINYSyntaxTree(root->children[3]));
            currentNode->addChild(genTINYSyntaxTree(root->children[4]));
            currentNode->addChild(genTINYSyntaxTree(root->children[5]));
        }
        else // identifier term' simple-exp' exp'情况
        {
            currentNode->addChild(genTINYSyntaxTree(root->children[0]));
            currentNode->addChild(genTINYSyntaxTree(root->children[1]));
            currentNode->addChild(genTINYSyntaxTree(root->children[2]));
            currentNode->addChild(genTINYSyntaxTree(root->children[3]));
        }
    }
    else if (root->value == "simple-exp") // simple-exp->term simple-exp'
    {
        //        qDebug()<<root->value<<root->children.size();

        currentNode = new syntaxTreeNode("");
        currentNode->flag = true;                                    // 要把该节点孩子提上来，先做标记后续再处理
        currentNode->addChild(genTINYSyntaxTree(root->children[0])); // term
        currentNode->addChild(genTINYSyntaxTree(root->children[1])); // simple-exp'
    }
    else if (root->value == "term") // term->factor term'
    {
        //        qDebug()<<root->value<<root->children.size();

        currentNode = new syntaxTreeNode("");
        currentNode->flag = true;                                    // 要把该节点孩子提上来，先做标记后续再处理
        currentNode->addChild(genTINYSyntaxTree(root->children[0])); // factor
        currentNode->addChild(genTINYSyntaxTree(root->children[1])); // term'
    }
    else if (root->value == "factor") // factor->( exp )|number|identifier
    {
        //        qDebug()<<root->value<<root->children.size();

        if (root->children.size() == 1) // number identifier
            currentNode = genTINYSyntaxTree(root->children[0]);
        else
            currentNode = genTINYSyntaxTree(root->children[1]); // ( exp )
    }
    else if (root->value == "stmt-sequence'") // stmt-sequence'->; statement stmt-sequence'|@
    {
        //        qDebug()<<root->value<<root->children.size();

        if (!root->children.size())
            return nullptr; //@
        currentNode = new syntaxTreeNode("");
        currentNode->flag = true;                                    // 要把该节点孩子提上来，先做标记后续再处理
        currentNode->addChild(genTINYSyntaxTree(root->children[1])); // statement
        currentNode->addChild(genTINYSyntaxTree(root->children[2])); // stmt-sequence'
    }
    else if (root->value == "simple-exp'") // simple-exp'->@|+ term simple-exp'|- term simple-exp'
    {
        //        qDebug()<<root->value<<root->children.size();

        if (!root->children.size())
            return nullptr; //@
        currentNode = new syntaxTreeNode("op");
        currentNode->nodeValue = root->children[0]->value;
        currentNode->addChild(genTINYSyntaxTree(root->children[1])); // term
        currentNode->addChild(genTINYSyntaxTree(root->children[2])); // simple-exp'
    }
    else if (root->value == "term'") // term'->@ | * factor term' | / factor term'
    {
        //        qDebug()<<root->value<<root->children.size();

        if (root->children.size() == 0)
            return nullptr; //@
        currentNode = new syntaxTreeNode("op");
        currentNode->nodeValue = root->children[0]->value;
        currentNode->addChild(genTINYSyntaxTree(root->children[1])); // factor
        currentNode->addChild(genTINYSyntaxTree(root->children[2])); // term'
    }
    else if (root->value == "exp'") // exp'->@|= simple-exp|< simple-exp
    {
        //        qDebug()<<root->value<<root->children.size();

        if (root->children.size() == 0)
            return nullptr; //@
        currentNode = new syntaxTreeNode("op");
        currentNode->nodeValue = root->children[0]->value;
        currentNode->addChild(genTINYSyntaxTree(root->children[1])); // simple-exp
    }
    else if (root->value == "statement'") // statement'->end|else stmt-sequence end
    {
        //        qDebug()<<root->value<<root->children.size();

        if (root->children.size() == 1) // 处理空串
            return nullptr;             // end
        currentNode = genTINYSyntaxTree(root->children[1]);
    }
    else if (root->type == "identifier" || root->type == "number")
    {
        //        qDebug()<<root->value<<root->children.size();

        currentNode = new syntaxTreeNode(root->type);
        currentNode->nodeValue = root->value;
    }
    qDebug() << currentNode->nodeType << ":" << currentNode->nodeValue;
    return currentNode;
}

/**
 * @brief BNFP::reformTINYSyntaxTree
 * @param root
 * 维护TINY语法树，对初转换得的TINY语法树进行
 */
void BNFP::reformTINYSyntaxTree(syntaxTreeNode *root)
{
    bool isEnd = false;
    while (!isEnd)
    {
        isEnd = true;
        for (int i = 0; i < root->children.size(); i++) // 遍历所有孩子
        {
            //            if(root->children.at(i)==nullptr)
            //            {
            //                root->children.removeAt(i);//剪枝
            //                continue;
            //            }

            syntaxTreeNode *currentChild = root->children.at(i);
            if (currentChild->isDeleted)
            {
                continue; // 节点已经被假删除了
            }
            qDebug() << "for()";
            qDebug() << currentChild->flag << currentChild->nodeValue;
            if (currentChild->flag || currentChild->nodeValue == "") // 如果该节点需要把孩子提上来
            {
                bool isOp = false;                                      // 记录是否有孙子是运算符
                for (int j = 0; j < currentChild->children.size(); j++) // 遍历所有孙子
                {

                    syntaxTreeNode *currentGrandChild = currentChild->children.at(j);        // 当前孙子节点
                    if (currentGrandChild != nullptr && currentGrandChild->nodeType == "op") // 判断是否有孙子节点为运算符 假如是运算符就把运算符提上来且把孩子节点的flag设为false，而后再将该孙子节点的flag设为true
                    {
                        // qDebug()<<"opX:"<<currentGrandChild->nodeType<<currentGrandChild->nodeValue;
                        currentChild->nodeType = "op";
                        currentChild->nodeValue = currentGrandChild->nodeValue;
                        currentChild->flag = false;

                        currentGrandChild->nodeType = "";
                        currentGrandChild->nodeValue = "";
                        currentGrandChild->flag = false;
                        currentGrandChild->isDeleted = true;

                        isOp = true;
                    }
                }
                if (isOp)
                    continue;

                isEnd = false;
                for (int j = 0; j < currentChild->children.size(); j++) // 遍历所有孙子，把他们都交给爷爷
                {
                    root->addChild(currentChild->children[j]);
                }
                currentChild->isDeleted = true; // 被假删除了
                break;                          // QList中的内容已修改，跳出循环重新判断
            }
        }
    }
    for (int i = 0; i < root->children.size(); i++)  // 遍历根节点的所有孩子
        if (!root->children[i]->isDeleted)           // 若该孩子没有被假删除（即未处理完成）
            reformTINYSyntaxTree(root->children[i]); // 递归到孩子节点
}

QTreeWidgetItem *BNFP::exchangeTree(syntaxTreeNode *root)
{
    if (root == nullptr)
    {
        return nullptr;
    }

    QHash<syntaxTreeNode *, QTreeWidgetItem *> nodeMap;
    QQueue<syntaxTreeNode *> q; // 存储语法树节点的队列

    QTreeWidgetItem *copyRootI = new QTreeWidgetItem({m_syntaxTreeRoot->nodeValue}); // 复制根节点
    nodeMap[m_syntaxTreeRoot] = copyRootI;                                           // 构建映射
    q.push_back(m_syntaxTreeRoot);

    while (!q.empty())
    {
        syntaxTreeNode *curNode = q.front(); // 当前遍历到的语法树节点
        q.pop_front();
        QTreeWidgetItem *curCopyNodeI = nodeMap[curNode]; // 当前待拷贝的节点

        // 遍历当前语法树节点的所有孩子
        for (syntaxTreeNode *child : curNode->children)
        {
            // qDebug()<<"---------------------------";
            if (child->nodeValue == "" && curNode->children.size() > 1)
                continue;

            QTreeWidgetItem *copyChildI = new QTreeWidgetItem({child->nodeValue}); // 复制当前孩子节点
            // qDebug()<<child->nodeType<<child->nodeValue;
            curCopyNodeI->addChild(copyChildI); // 加入当前语法树节点的孩子中

            nodeMap[child] = copyChildI; // 将孩子节点加入映射中，等待后续处理
            q.push_back(child);          // 将孩子放入队列中，等待后续处理
        }
    }
    return copyRootI;
}

/**
 * @brief BNFP::printParseTree
 * @param t
 * 输出高级语言（TINY/MiniC）源程序分析树
 */
void BNFP::printParseTree(QTreeWidget *t)
{
    t->headerItem()->setHidden(true);
    t->clear();

    t->addTopLevelItem(m_parseTreeRootI);
}

/**
 * @brief BNFP::printTINYAST
 * @param t 语法分析树
 * 输出语法分析树
 */
void BNFP::printAST(QTreeWidget *t, QString language)
{

    m_syntaxTreeRootI = nullptr;
    t->headerItem()->setHidden(true);
    t->clear();

    genAST(language);

    if (language == "TINY")
    {
        m_syntaxTreeRootI = exchangeTree(m_syntaxTreeRoot);
    }

    t->addTopLevelItem(m_syntaxTreeRootI);
}

/**
 * @brief BNFP::eliminateLRecursion
 * @param index
 * @param updatedL
 * 消除左递归子函数
 */
void BNFP::eliminateLRecursion(int index, QSet<QString> &newNonTmrSet)
{
    QString t_left = m_nonTmrSet[index]; // 左部非终结符
    QList<QStringList> t_newRightList;   // 产生式右部

    // 遍历该产生式的所有候选式
    for (const auto &t_candidateList : m_GM_productionMap[t_left].pdnRights)
    {
        bool rcFlag = 0;                       // 是否左递归标志
        QString t_cFirst = t_candidateList[0]; // 候选式首个单词
        if (newNonTmrSet.contains(t_cFirst))
        { // 若暂存终结符集中也出现自己该其则说明含有左递归
            rcFlag = 1;
            for (const auto &t_cFirst_candidateList : m_GM_productionMap[t_cFirst].pdnRights)
            {
                QStringList t_cddList = t_candidateList;
                t_cddList.removeFirst();                        // 把第一个元素删掉
                t_cddList = t_cFirst_candidateList + t_cddList; // 用t_t...换掉本身的第一个元素，拼起来
                t_newRightList.append(t_cddList);               // 更新候选式（消除左递归）
            }
        }
        if (!rcFlag)                                // 若没有左递归
            t_newRightList.append(t_candidateList); // 则将该候选式加回新的暂存列表中
    }
    m_GM_productionMap[t_left].pdnRights = t_newRightList; // 覆写m_nonTmrSet[index]右部

    // 直接左递归消除
    for (const auto &t_candidateList : m_GM_productionMap[t_left].pdnRights)
    {
        // 遍历该产生式的所有候选式
        QString t_cFirst = t_candidateList[0];
        if (t_cFirst == t_left) // 相同即出现A->A...直接左递归
        {
            QString newLeft = getNewTmr(t_left);                               // 申请新的非终结符
            QList<QStringList> t_curElrRights;                                 // 存储当前非终结符消除直接左递归后的候选式
            QList<QStringList> t_newNTRights;                                  // 存储新非终结符候选式
            for (const auto &t_cddList : m_GM_productionMap[t_left].pdnRights) // 遍历该产生式的所有候选式
            {
                QString t_cFirst = t_cddList[0]; // 候选式首词
                if (t_cFirst == t_left)          // 左递归出现
                {
                    QStringList t_cList = t_candidateList;
                    t_cList.removeFirst();         // 删除首词
                    t_cList.append(newLeft);       // 新的非终结符
                    t_newNTRights.append(t_cList); // 加入到新非终结符候选式列表中
                }
                else
                {
                    QStringList t_cList = t_cddList; // 没左递归直接加入另外一个集合中
                    t_cList.append(newLeft);
                    t_curElrRights.append(t_cList);
                }
            }
            t_newNTRights.append({"@"});                           // 加入eps
            m_GM_productionMap[t_left].pdnRights = t_curElrRights; // 更新当前非终结符的候选式
            // 向文法中添加新非终结符
            m_nonTmrSet.append(newLeft);
            m_GM_productionMap[newLeft].pdnRights = t_newNTRights; // 添加新非终结符的右部
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
    // qDebug()<<m_tmrSet;
    QSet<QString> t_newNonTmrSet; // 存储处理后的非终结符
    for (int i = 0; i < m_nonTmrSet.size(); i++)
    {
        QString t_nonTmr = m_nonTmrSet[i]; // 按一定的顺序遍历非终结符，先保存当前非终结符“快照”
        eliminateLRecursion(i, t_newNonTmrSet);
        t_newNonTmrSet.insert(t_nonTmr); // 处理完后放回集合中
    }
}

/**
 * @brief BNFP::getNewTmr
 * @return
 * 申请新的终结符
 */
QString BNFP::getNewTmr(QString curTmr)
{
    return curTmr + "'";
}

/**
 * @brief BNFP::findL
 * @param newSet
 * @param Temp
 * @return
 * 防止产生相同的候选式
 */
QString BNFP::findL(QMap<QString, QVector<QStringList>> newSet, QList<QStringList> Temp)
{
    QSet<QStringList> tempSet;
    foreach (const auto &t, Temp)
    {
        tempSet.insert(t);
    }
    foreach (const auto &s, m_nonTmrSet)
    {
        QSet<QStringList> rTempSet;
        foreach (const auto &t, m_GM_productionMap[s].pdnRights.toList())
        {
            rTempSet.insert(t);
        }
        if (rTempSet == tempSet)
            return s;
    }
    foreach (QString left, newSet.keys())
    {
        QSet<QStringList> lTempSet;
        foreach (const auto &t, newSet[left].toList())
        {
            lTempSet.insert(t);
        }
        if (lTempSet == tempSet)
            return left;
    }

    return "";
}

/**
 * @brief BNFP::decodeLex
 * 解码词法分析程序输出的源程序代码
 */
void BNFP::decodeLex(QString language)
{
    QStringList t_lexPrgList = m_lexPrgStr.split('\n');
    // qDebug()<<t_lexPrgList;
    for (const auto &line : t_lexPrgList)
    {
        if (line.trimmed().isEmpty())
            continue;
        qDebug() << line;
        QList<QString> t_wordList = line.simplified().split(' '); // 分开单词
        // qDebug()<<t_wordList;
        for (int i = 0; i < t_wordList.size(); i++)
        {
            QString t_word = t_wordList[i];
            if (!t_word.contains("ID") && !t_word.contains("Digit") && !t_word.contains("Keyword"))
            {
                // qDebug()<<t_word;
                m_programCode.append(t_word);
                continue;
            }
            QString t_wordType = t_word.split(':').at(0);
            QString t_wordContent = t_word.split(':').at(1);

            if (t_wordType == "Keyword")
            {
                m_programCode.append(t_wordContent);
            }
            else if (t_wordType == "ID")
            {
                if (language == "TINY")
                    m_programCode.append("identifier");
                else if (language == "MiniC")
                    m_programCode.append("ID");
                else
                    m_programCode.append(t_wordType);

                m_programCode.append(t_wordContent);
            }
            else if (t_wordType == "Digit")
            {
                if (language == "TINY")
                    m_programCode.append("number");
                else if (language == "MiniC")
                    m_programCode.append("NUM");
                else
                    m_programCode.append(t_wordType);

                m_programCode.append(t_wordContent);
            }
        }
    }
    m_programCode.append("$"); // 规定结束符为$
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
    int t_count = count + 1;
    for (const auto &t_pdnR : list)
    {
        if (t_count > t_pdnR.size() || count >= pdnR.size() || t_pdnR.at(count) != pdnR.at(count))
            return;
    }
    count = t_count;
    lFactorCount(list, pdnR, count); // 递归调用
}

/**
 * @brief BNFP::eliminateLCommonFactor
 * 提取左公因子
 */
void BNFP::eliminateLCommonFactor()
{
    int rcFlag = 4; // 递归深度
    bool Flag = 1;  // 继续下一轮代入提取左公因子标志

    while (rcFlag-- && Flag)
    {
        int t_nTSetSize = m_nonTmrSet.size(); // 非终结符集合大小

        Flag = 0;
        QMap<QString, QSet<QStringList>> deletedSetMap = {};  // 记录非终结符要被移除的候选式
        QMap<QString, QSet<QStringList>> appendSetMap = {};   // 记录要恢复的候选式
        QMap<QString, QVector<QStringList>> t_newPdnMap = {}; // 新构造的产生式映射

        // 遍历完再删 否则容易导致下标越界
        for (int i = 0; i < t_nTSetSize; i++) // 遍历所有产生式
        {
            QString t_curNonTmr = m_nonTmrSet.at(i); // 当前非终结符

            // 遍历非终结符
            for (int i = 0; i < t_nTSetSize; i++)
            {
                QString t_nT = m_nonTmrSet.at(i);

                if (m_GM_productionMap[t_nT].pdnRights.size() > 1)
                {
                    for (int j = 0; j < m_GM_productionMap[t_nT].pdnRights.size(); j++)
                    {
                        QStringList t_jCdd = m_GM_productionMap[t_nT].pdnRights.at(j);
                        QString t_c0Word = t_jCdd.at(0);
                        if (m_nonTmrSet.contains(t_c0Word))
                        {
                            QStringList t_delCdd = t_jCdd;
                            for (int k = 0; k < m_GM_productionMap[t_c0Word].pdnRights.size(); k++)
                            {
                                QStringList t_kCdd = m_GM_productionMap[t_c0Word].pdnRights.at(k);

                                appendSetMap[t_nT].insert(t_kCdd + t_jCdd.mid(1));
                            }

                            deletedSetMap[t_nT].insert(t_delCdd);
                        }
                        else if (t_c0Word == "@" && t_jCdd.size() > 1)
                        {
                            QStringList t_delCdd = t_jCdd;
                            appendSetMap[t_nT].insert(t_jCdd.mid(1));
                            deletedSetMap[t_nT].insert(t_delCdd);
                        }
                    }
                }
                if (deletedSetMap.size() || appendSetMap.size())
                    Flag = true;
            }

            for (const QString &delKey : deletedSetMap.keys())
                for (const QStringList &delTmp : deletedSetMap[delKey])
                    m_GM_productionMap[delKey].pdnRights.removeOne(delTmp);

            for (QString &t_cdd : appendSetMap.keys())
                for (const QStringList &appendTmp : appendSetMap[t_cdd])
                    m_GM_productionMap[t_cdd].pdnRights.append(appendTmp);

            deletedSetMap.clear();
            appendSetMap.clear();

            QSet<QString> ePrefix = {}; // 记录已被提取的左公因子

            for (int j = 0; j < m_GM_productionMap[t_curNonTmr].pdnRights.size(); j++)
            {
                QList<QStringList> t_eCddList = {}; // 记录被提取的候选式

                if (ePrefix.contains(m_GM_productionMap[t_curNonTmr].pdnRights.at(j).at(0))) // 若前缀已被提取过，下次不从这里开始
                    continue;
                QStringList t_curCddList = m_GM_productionMap[t_curNonTmr].pdnRights.at(j); // 当前非终结符第j条候选式
                QString t_curCddPrefix = t_curCddList.at(0);                                // （假设公共前缀）当前非终结符第一条候选式的第一个单词

                // 遍历当前非终结符对应的候选式
                for (int k = 0; k < m_GM_productionMap[t_curNonTmr].pdnRights.size(); k++)
                {
                    QStringList t_cdd = m_GM_productionMap[t_curNonTmr].pdnRights.at(k);
                    if (t_cdd.at(0) == t_curCddPrefix && !t_eCddList.contains(t_cdd)) // 判断该候选式是否有相同左公因子
                        t_eCddList.append(t_cdd);                                     // 有就放入提取列表
                }

                // 不一定需要整个右部都被提取，有两条及以上就可以了
                if (t_eCddList.count() > 1)
                {
                    ePrefix.insert(t_curCddPrefix); // 记录本次提取前缀

                    int t_lFCount = 1;
                    lFactorCount(t_eCddList, t_curCddList, t_lFCount); // 记录最长左公因子个数
                    for (const auto &t_delECdd : t_eCddList)
                    {
                        QStringList t_tdelECdd = t_delECdd;

                        deletedSetMap[t_curNonTmr].insert(t_tdelECdd); // 记录将被移除的候选式
                    }
                    foreach (const auto &t_eCdd, t_eCddList)
                    {
                        // 形成新产生式的右部
                        t_eCddList.removeOne(t_eCdd); // 一条条更新候选式
                        if (t_lFCount != t_eCdd.size())
                            t_eCddList.append(t_eCdd.mid(t_lFCount));
                        else if (!t_eCddList.contains({"@"}))
                            t_eCddList.append({"@"});
                    }
                    // 给新的产生式赋值
                    QString left = findL(t_newPdnMap, t_eCddList);
                    if (left == "")
                    {

                        left = getNewTmr(t_curNonTmr); // 申请新的非终结符
                        while (t_newPdnMap.contains(left) || m_nonTmrSet.contains(left))
                            left = getNewTmr(left);
                        t_newPdnMap[left] = t_eCddList.toVector();
                    }

                    // 将化简后的候选式加入该产生式
                    QStringList leftFactor = t_curCddList.mid(0, t_lFCount);
                    leftFactor.append(left); // 拼接

                    appendSetMap[t_curNonTmr].insert(leftFactor);
                }
            }
        }
        if (deletedSetMap.size() || appendSetMap.size() || t_newPdnMap.size())
            Flag = true;

        for (const auto &nTmr : deletedSetMap.keys())
            for (const auto &delCdd : deletedSetMap[nTmr])
                m_GM_productionMap[nTmr].pdnRights.removeOne(delCdd); // 移除候选式

        for (const auto &nTmr : appendSetMap.keys())
            for (const auto &appendTmp : appendSetMap[nTmr])
                m_GM_productionMap[nTmr].pdnRights.append(appendTmp); // 增添候选式

        for (const auto &left : t_newPdnMap.keys())
        {
            m_nonTmrSet.append(left);
            m_GM_productionMap[left].pdnRights = t_newPdnMap[left]; // 增添新候选式
        }
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
    bool changedFlag = true; // 标记此轮是否发生了更新，即First是否出现更新
    while (changedFlag)      // 直到first集不变
    {
        changedFlag = false;
        for (int i = 0; i < m_nonTmrSet.size(); i++) // 遍历所有产生式
        {
            QString t_curLeft = m_nonTmrSet[i];                                             // 当前左部
            QVector<QStringList> t_candidateList = m_GM_productionMap[t_curLeft].pdnRights; // 当前右部
            // 遍历当前右部
            for (int j = 0; j < t_candidateList.size(); j++)
            {
                QString t_cWord = t_candidateList[j][0];              // 候选式首字符
                if (!m_nonTmrSet.contains(t_cWord) || t_cWord == "@") // 终结符和epsilon（情况1，直接将其加入First集中）
                {
                    if (!m_GM_productionMap[t_curLeft].firstSet.contains(t_cWord)) // first集合里是否有t_First
                    {
                        m_GM_productionMap[t_curLeft].firstSet.insert(t_cWord);
                        changedFlag = 1; // first集求解情况变更
                    }
                }
                else
                {
                    // t_cFirst为非终结符，将其First集加入左部First集
                    bool epsilonFlag = false;                           // 是否出现epsilon标志
                    for (int k = 0; k < t_candidateList[j].size(); k++) // 遍历单条候选式N->C1C2...Ck
                    {
                        t_cWord = t_candidateList[j][k];
                        if (!m_nonTmrSet.contains(t_cWord)) // 若遇到终结符
                        {
                            if (!m_GM_productionMap[t_curLeft].firstSet.contains(t_cWord)) // first集合里是否有该字符
                            {
                                m_GM_productionMap[t_curLeft].firstSet.insert(t_cWord); // 无则加，实际上就是插入操作
                                changedFlag = true;
                            }
                            break; // 遇到了终结符可以直接结束了
                        }

                        // 查该非终结符的first集合
                        for (const auto &t_firstStr : m_GM_productionMap[t_cWord].firstSet)
                        {
                            if (t_firstStr == "@") // 情况2，该情况需要进入下一轮判断，查看规则下一个单词Cx
                                epsilonFlag = true;
                            if (!m_GM_productionMap[t_curLeft].firstSet.contains(t_firstStr)) // first集合里是否有tmp
                            {
                                m_GM_productionMap[t_curLeft].firstSet.insert(t_firstStr); // 将非终结符first集加入当前左部First集中
                                changedFlag = 1;
                            }
                        }
                        if (!epsilonFlag)
                            break;           // 该非终结符first集合中无epsilon，停止继续遍历候选式
                        epsilonFlag = false; // 看下一个字符
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
    bool changedFlag = true;                               // follow集产生变化标志
    m_GM_productionMap[m_startChar].followSet.insert("$"); // 文法开始符号加结束符号
    while (changedFlag)
    {
        changedFlag = false;
        for (int i = 0; i < m_nonTmrSet.size(); i++) // 遍历所有产生式
        {
            QString t_curLeft = m_nonTmrSet[i];                                             // 当前左部
            QVector<QStringList> t_candidateList = m_GM_productionMap[t_curLeft].pdnRights; // 当前右部
            for (int j = 0; j < t_candidateList.size(); j++)                                // 遍历每一个候选式
            {
                for (int k = 0; k < t_candidateList[j].size(); k++) // 寻找每个候选式中的每个非终结符
                {
                    QString t_cWord = t_candidateList[j][k]; // 当前字符
                    if (m_nonTmrSet.contains(t_cWord))       // 是非终结符
                    {
                        if (k == t_candidateList[j].size() - 1) // 当前非终结符为候选式中的最后一个字符（情况2）
                        {                                       // 将当前左部follow集加入到 候选式当前字符follow集中
                            for (const QString &t_followStr : m_GM_productionMap[t_curLeft].followSet)
                                if (!m_GM_productionMap[t_cWord].followSet.contains(t_followStr))
                                {
                                    changedFlag = true;
                                    m_GM_productionMap[t_cWord].followSet.insert(t_followStr);
                                }
                        }
                        else
                        {
                            QString t_cNextWord = t_candidateList[j][k + 1]; // 查看下一个字符
                            if (!m_nonTmrSet.contains(t_cNextWord))          // 若为终结符（情况1）
                            {                                                // 简单更新当前字符follow集
                                if (!m_GM_productionMap[t_cWord].followSet.contains(t_cNextWord))
                                {
                                    m_GM_productionMap[t_cWord].followSet.insert(t_cNextWord);
                                    changedFlag = true;
                                }
                            }
                            else
                            { // （情况1）将后面字符除eps外的所有first集元素加进来follow集
                                for (const QString &follow : m_GM_productionMap[t_cNextWord].firstSet)
                                {
                                    if (follow == "@")
                                        continue;
                                    if (!m_GM_productionMap[t_cWord].followSet.contains(follow))
                                    {
                                        changedFlag = true;
                                        m_GM_productionMap[t_cWord].followSet.insert(follow);
                                    }
                                }
                                bool epsilon = true;
                                for (int x = k + 1; x < t_candidateList[j].size(); x++) // 查看是否后面的字符都含有epsilon（因为如果后面的字符都含epsilon 相当于是最后一个字符）
                                {
                                    QString t_word = t_candidateList[j][x];
                                    if (!m_nonTmrSet.contains(t_word) ||
                                        !m_GM_productionMap[t_word].firstSet.contains("@")) // 如果其中一个不含epsilon（情况1，不需要处理）
                                        epsilon = false;
                                }
                                if (epsilon) // 如果后面的字符都含epsilon 相当于是最后一个字符（情况2）
                                {            // 将当前左部follow集加入到 候选式当前字符follow集中
                                    for (const auto &t_followStr : m_GM_productionMap[t_curLeft].followSet)
                                        if (!m_GM_productionMap[t_cWord].followSet.contains(t_followStr))
                                        {
                                            changedFlag = true;
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
    for (int i = 0; i < m_nonTmrSet.size(); i++)
    {
        QString t_curNT = m_nonTmrSet[i];                                             // 当前非终结符
        QVector<QStringList> t_candidateList = m_GM_productionMap[t_curNT].pdnRights; // 当前非终结符对应的产生式
        for (int j = 0; j < t_candidateList.size(); j++)                              // 遍历候选式
        {
            QStringList t_candidate = t_candidateList[j]; // 当前非终结符对应的产生式中的一个候选式

            //            QString t_cddStr="";//候选式字符串
            //            for(int k=0;k<t_candidate.size();k++)
            //                t_cddStr+=t_candidate[k]+' ';
            //            t_cddStr.chop(1);//把最后多出来的空格去掉
            //            QString itemContent=t_curNT+"->"+t_cddStr;//要填进ll1分析表中的内容

            QString t_cFirst = t_candidate[0];   // 该候选式的第一个字符
            if (!m_nonTmrSet.contains(t_cFirst)) // 不是非终结符
            {
                if (t_cFirst == "@") // 情况2
                {
                    foreach (const auto &t_follow, m_GM_productionMap[t_curNT].followSet)
                    {
                        if (!m_LL1Table[t_curNT].contains(t_follow))
                            m_LL1Table[t_curNT][t_follow] = t_candidate;
                        else
                        {
                            if (m_LL1Table[t_curNT][t_follow][0] == "@") // 人为修改，
                                m_LL1Table[t_curNT][t_follow] = t_candidate;
                        }
                    }
                    continue;
                }
                m_LL1Table[t_curNT][t_cFirst] = t_candidate; // 存入表中
            }
            else // 若为非终结符
            {
                bool epsFlag = false;
                int t_1CddSize = t_candidate.size(); //
                for (int k = 0; k < t_1CddSize; k++)
                {
                    t_cFirst = t_candidate[k];
                    if (!m_nonTmrSet.contains(t_cFirst)) // 不是非终结符,这里其实是假如第一个非终结符的first集合为空才有可能进来
                    {
                        if (t_cFirst == "@")
                            continue;                                // epsilon不需加进去，没有候选式的epsilon会出现在中间
                        m_LL1Table[t_curNT][t_cFirst] = t_candidate; // 填表
                        break;                                       // 可以退出循环了
                    }
                    else // 非终结符
                    {
                        for (const auto &t_first : m_GM_productionMap[t_cFirst].firstSet) // 遍历first集
                        {
                            if (t_first == "@")
                                epsFlag = true; // 记录存在epsilon（情况2）
                            else
                                m_LL1Table[t_curNT][t_first] = t_candidate; // 情况1，直接填入
                        }
                        if (epsFlag)
                        {
                            epsFlag = false;
                            if (k == t_1CddSize - 1) // 到最后一个了还是有epsilon就看follow集
                                for (const auto &follow : m_GM_productionMap[t_curNT].followSet)
                                    m_LL1Table[t_curNT][follow] = t_candidate;
                        }
                        else
                            break;
                    }
                }
            }
        }
    }
}

/**
 * @brief BNFP::LL1Parsing
 * @param tree
 * @param progStr 待解码lex编码
 * @param console 控制台输出
 * @param language 解码语言类型（TINY/MiniC）
 * LL1分析主函数
 */
bool BNFP::LL1Parsing(QString progStr, QPlainTextEdit *console, QString language)
{
    // 分析、查表、替换 23重复直至结束或错误
    m_lexPrgStr.clear();
    m_programCode.clear();
    m_lexPrgStr = progStr; // 保存lexProgStr
    decodeLex(language);   // 解码源程序并存入m_programCode中

    QStack<QString> parseStk;
    parseStk.push("$");
    parseStk.push(m_startChar);

    QStack<parseTreeNode *> pTreeNodeStk;
    QStack<QTreeWidgetItem *> qTreeStk;
    parseTreeNode *root = new parseTreeNode(m_startChar);
    QTreeWidgetItem *treeI = new QTreeWidgetItem({m_startChar});
    pTreeNodeStk.push(root);
    qTreeStk.push(treeI);

    int i = 0; // Code index

    while (i < m_programCode.size())
    {
        if (parseStk.isEmpty())
        {
            printInfo("LL1 Parser: err01", console);
            printInfo("Info: parseStack is empty.", console);
            return false;
        }
        QString token = m_programCode[i];
        QString parseStr = parseStk.pop();
        if (token == parseStr && token == "$")
            break; // end标志

        if (pTreeNodeStk.isEmpty())
        {
            // qDebug()<<parseStr<<token;
            printInfo("LL1 Parser: err02", console);
            printInfo("Info: parseStr:" + parseStr + ", token:" + token, console);
            return false;
        }

        parseTreeNode *curNode = pTreeNodeStk.pop();
        QTreeWidgetItem *curNodeI = qTreeStk.pop();

        if (m_nonTmrSet.contains(parseStr)) // 非终结符
        {
            if (!m_LL1Table[parseStr].contains(token))
            {
                printInfo("LL1 Parser: err03", console);
                printInfo("Info: parseStr:" + parseStr + ", token:" + token, console);
                return false;
            }
            QStringList pdnR = m_LL1Table[parseStr][token];

            for (int j = pdnR.size() - 1; j >= 0; j--)
            {
                if (pdnR[j] == "@")
                {
                    curNodeI->addChild(new QTreeWidgetItem({"ɛ"})); // 仅显示，不保留
                    continue;                                       // 略去eps
                }

                parseStk.push(pdnR[j]);

                parseTreeNode *newTNode = new parseTreeNode(pdnR[j]);
                QTreeWidgetItem *newChild = new QTreeWidgetItem({pdnR[j]});

                pTreeNodeStk.push(newTNode);
                qTreeStk.push(newChild);

                curNode->children.insert(curNode->children.begin(), newTNode);
                curNodeI->addChild(newChild);
            }
        }
        else
        {
            if (parseStr != token)
            {
                printInfo("LL1 Parser: err04", console);
                printInfo("Info: parseStr:" + parseStr + ", token:" + token, console);
                return false;
            }
            if (language == "TINY")
            {
                if (token == "identifier" || token == "number")
                {
                    curNode->type = token; // 注明类型
                    token = m_programCode[++i];
                }
            }
            else if (language == "MiniC")
            {
                if (token == "ID" || token == "NUM")
                {
                    curNode->type = token; // 注明类型
                    token = m_programCode[++i];
                }
            }
            curNode->value = token;
            curNodeI->setText(0, token);
            i++;
        }
    }
    m_parseTreeRoot = root;
    m_parseTreeRootI = treeI;
    printInfo("语法正确", console);

    return true;
}

/**
 * @brief BNFP::genAST
 * @param language 生成语法树的高级语言类型
 * 生成语法树
 */
void BNFP::genAST(QString language)
{
    if (language == "TINY")
    {
        m_syntaxTreeRoot = genTINYSyntaxTree(m_parseTreeRoot);
        reformTINYSyntaxTree(m_syntaxTreeRoot);
    }
}
