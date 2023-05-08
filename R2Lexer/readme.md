# Compile-Principle-Course



## Task 1 (TINY’s) Regex to Lexer

   The system needs to provide an interface that allows the user to open a text file of all words in a language, convert the resulting lexical analysis program (which needs to be described in C) to compile an executable program from the source program requested above, and test it with a source program in that high-level programming language. A source program in this high-level programming language is tested and the word code of this source program is output. A window needs to be provided so that the user can view the word code.

​        以文本文件的方式输入某一高级程序设计语言的所有单词对应的正则表达式，系统需要提供一个操作界面，让用户打开某一语言的所有单词对应正则表达式文本文件，转换得到的词法分析程序（该分析程序需要用 C 语言描述）对上述要求得到的源程序进行编译生成一个可执行程序，并以该高级程序设计语言的一个源程序进行测试，输出该该源程序的单词编码。需要提供窗口以便用户可以查看该单词编码。



## 项目设计

### 功能结构图

```mermaid
flowchart TB
r2a(Lex生成器)-->e(输入正则表达式)
r2a-->op(打开正则表达式文件)
r2a-->s(保存正则表达式文件)

e-->tr(正则表达式转NFA)
tr-->trr(NFA转DFA)
trr-->trrr(最小化DFA)
trrr-->trrrr(DFA生成Lexer源程序)
tr-->显示NFA
trr-->显示DFA
trrr-->显示最小化DFA
trrrr-->trrrrr(输出代码的单词编码)
trrrr-->显示生成Lexer代码
```



### 正则转NFA

- 连接运算的处理

  ​      连接运算在正则表达式中的表示中是不需要字符的，也即是说没有运算符。如果不对这种情况进行处理，则会导致识别转换过程中出现问题：运算符栈没有运算符放入的话，出栈的时候字符栈有些字符就不能够出栈。我们这里使用&字符处理两个相连接的字母。

- Thompson方法将正则表达式转换为NFA：

  ​      为每个基本正则表达式构造一个基本的NFA子图，尔后根据正则表达式将各子NFA连接起来

  以下为基本的正则表达式以及其NFA子图：

  - digit/letter/$\epsilon$

    ```mermaid
    flowchart LR
    start-->s((0))--a-->ss(((1)))
    ```

  - 连接

    ```mermaid
    flowchart LR
    subgraph NFA1
    s(( ))-."r".-ss(( ))
    end
    subgraph NFA2
    ss--ɛ-->ss3(( ))
    ss3-."s".-ss4((( )))
    end
    
    ```

  - 选择 |

    ```mermaid
    flowchart LR
    st(( ))--ɛ-->s
    st--ɛ-->ss3
    
    subgraph NFA1
    s(( ))-."r".-ss(( ))
    end
    
    subgraph NFA2
    ss3(( ))-."s".-ss4((( )))
    end
    
    ss4--ɛ-->ss5((( )))
    ss--ɛ-->ss5
    ```

  - 闭包 *

    ```mermaid
    flowchart LR
    st(( ))--ɛ-->s
    
    
    subgraph " "
    ss--ɛ-->s
    s(( ))-."r".-ss(( ))
    end
    
    
    
    ss--ɛ-->ss5((( )))
    st--ɛ-->ss5
    ```

  - 正闭包 +

    ```mermaid
    flowchart LR
    st(( ))--ɛ-->s
    
    
    subgraph " "
    ss--ɛ-->s
    s(( ))-."r".-ss(( ))
    end
    
    
    
    ss--ɛ-->ss5((( )))
    
    ```

    

  - 可选 ?

    ```mermaid
    flowchart LR
    st(( ))--ɛ-->s
    
    subgraph " "
    s(( ))-."r".-ss(( ))
    end
    
    ss--ɛ-->ss5((( )))
    st--ɛ-->ss5
    ```

    

- 运算符优先级处理

  ​      类似于平时的数学运算法则，正则表达式的符号也有结合的优先级，我们将正则表达式转换为NFA的最后一步就要解决该问题。上学期课堂上老师提示过可以通过开两个栈（符号栈、运算符栈）的方法解决该问题，这里我们可以应用该方法，该方法有以下要点：
  
  1. 规定符号优先级
  
     | 优先级 | 符号    | 含义         |
     | ------ | ------- | ------------ |
     | 1      | (Regex) | 正则表达式块 |
     | 2      | \* ? +  | 单目运算符   |
     | 3      | abc     | 连接运算     |
     | 4      | \|      | 选择         |
  
  2. 生成NFA规则
  
     扫描正则表达式：
  
     - 遇到字符a：直接生成字符a的NFA并压入字符栈
     - 遇到“(”号：直接压入符号栈
     - 遇到“)”号：需要符号栈一直弹出至“(”，每次弹出一个运算符的同时，从字符
       栈中弹出他需要的个数的NFA，进行运算。当弹出到“(”时，直接丢弃“)”号，进入下一轮判断
     - 遇到“\*”、“?”、“+”号：从字符栈中弹出一个元素A，计算A*/A?/A+后压回字符栈中
     - 遇到“&”号：从字符栈中先后弹出两个元素A、B，计算B&A后压回栈中
     - 遇到“|”号：
       - 若符号栈为空：直接压入符号栈
       - 若符号栈非空：从字符栈中先后弹出两个元素A、B，计算B|A后压回字符栈
  
     结束条件：
  
     - 扫描结束且符号栈为空
  
     字符栈栈顶运算即为最终运算结果。
  
- 



### NFA转DFA

DFA与NFA的主要区别在于DFA只有一个初态且遇到一个终结符后只会转移到一个确定的状态，所以NFA转换为NFA主要解决的问题是:消除导致转移到的状态不确定的ε转换和多重转换。

消除ε转换比较简单，将某状态和其能通过0-n条ε边转换的状态合并为一个状态。完成消除ε转换后，只能有一个初态的问题也同时解决了。求某状态和其能通过0-n条ε边转换的状态，就是求某状态的ε闭包的过程，算法的具体实现就是一个图的搜索的过程，实现中选用广度优先搜索(BFS)完成。

消除多重转换则是将某个状态(集)遇到某个终结符后能转移到的所有状态合并为一个状态即可。即，先找出某状态遇到某终结符后能转移到的状态，再求所有能转移到的状态的ε闭包即可。实现中，先遍历状态集，将状态集中各个状态通过某终结符能转换到的状态放到一个集合中，再将集合拿去上面写好的求ε闭包的函数中求ε闭包即可。

最后，我们还需要做的事是找出DFA中的终态，DFA中的终态的求解的原理是：如果DFA中某个状态集包含NFA的终态 则它是DFA中的一个终态。我们只需要遍历DFA结点的状态集,看其中是否有状态在NFA中是终态，如果是，那么该DFA结点是DFA中的终态。



这里需要注意的是，一开始不要在原集合上划分，而是开一个临时的划分集去尝试划分，如果确实需要划分才去划分(开一个临时变量存储划分数，遍历带划分集合中的所有状态，如果待划分集合中的某状态遇到当前划分依据的终结符后转换到一个与之前转换到的状态不同的状态时，划分数就加1。遍历完成后，如果临时变量划分数大于1则说明原本在同一集合中的状态 遇到同一个终结符后 转换成的状态的种类不同(>1)，那么真的需要进行划分)。否则如果后来发现不需要划分，再去回溯会比较麻烦。如果一次划分没有产生新划分，那么之后肯定也不会再产生新划分了，此时算法结束。

划分完成后，每一个划分就是最小DFA中的一个结点，我们还需要根据原DFA的信息完善新生成的最小DFA。最小DFA某节点状态集中的状态若包含原DFA的初态/终态，那么它就是最小DFA中的初态/终态。而对于最小DFA中的边，我们需要双层循环遍历各个最小DFA结点的状态集，看它们两两之间在原DFA中的连接情况，进行边的连接。为了求解方便，我们为最小化DFA的划分中划分出来的状态集专门设计一个存储结构，里面存着该划分的状态集，和它们遇到划分依据的字符后转移到的状态。



NFA转DFA求解流程如下示：

```mermaid
flowchart LR
s((开始))-->初始化-->求解初态的ɛ闭包--加入初态到搜索队列中-->BFS后节点建立DFA-->计算DFA终态集合
```



### 最小化DFA

​      手工做法是先划分初态和终态，再找出相同的状态合并；然而在代码角度用找出不同的状态来划分的角度来处理，代码会更方便些（这样对于终止条件的定义和处理会简单很多）。因为用划分的角度来看的话，从划分终态和非终态开始我们每次做的就是类似的操作。



最小化DFA求解流程

```mermaid
flowchart TB
start((开始))-->init(初始化)-->div(终态与非终态划分)-->for(遍历所有划分)-->ffor(遍历所有终结符)--对于-->if{是否产生新划分}
if--是-->for
if--否-->ediv(划分结束)--此时有最小化DFA-->p(处理DFA初态与终态)-->pfor(遍历DFA的所有节点)-->pffor(遍历节点的所有状态集)-->e[为最小DFA节点间构造边]
```

### 由DFA生成Lexer源代码及源程序

