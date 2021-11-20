# LLGrammarAnalyzer 语法分析器



## 项目背景

:books: 本项目是编译原理的课程实验作业之一

:rocket: 使用 LL(1) 分析方法进行自上而下的语法分析

:call_me_hand: 只有所在班级的课程实验均为单人完成，所以舍弃了更复杂、效果更好的自底向上方法。



## 开发环境

### 硬件 :computer:

- 型号：Dell Inspiration 7590；
- CPU：Intel Core i7-9750H；
- 内存：16GB 2666MHz；



### 软件 :floppy_disk:

- 系统：Windows 10 Home Basic 64-bit 20H1；
- GUI库：Qt 5.15.2 Open Source Version；
- GUI设计：Qt Creator 5.0.0；
- 编译器：MingGW 8.1.0 64-bit；
- 构建工具：Cmake 3.21.0；
- 版本控制：Git 2.32.0；
- 进程封装：Enigma Virtual Box 9.70；



## 项目功能



### 词法识别项

本分析器结合了先前写成的预处理器以及词法分析器功能，能够识别的词法单元如下：

|              **词法单元**               | **种别码** | **助记符** |  **内码**  |
| :-------------------------------------: | :--------: | :--------: | :--------: |
|                  void                   |     1      |   $void    |     -      |
|                   int                   |     2      |    $int    |     -      |
|                  long                   |     3      |   $long    |     -      |
|                  float                  |     4      |   $float   |     -      |
|                 double                  |     5      |  $double   |     -      |
|                  bool                   |     6      |   $bool    |     -      |
|                 string                  |     7      |  $string   |     -      |
|                   if                    |     8      |    $if     |     -      |
|                  elif                   |     9      |   $elif    |     -      |
|                  else                   |     10     |   $else    |     -      |
|                 return                  |     11     |  $return   |     -      |
|                  while                  |     12     |   $while   |     -      |
|                   for                   |     13     |    $for    |     -      |
|                  break                  |     14     |   $break   |     -      |
|                continue                 |     15     | $continue  |     -      |
|                 switch                  |     16     |  $switch   |     -      |
|                  case                   |     17     |   $case    |     -      |
|                 default                 |     18     |  $default  |     -      |
|                  true                   |     19     |   $true    |     -      |
|                  false                  |     20     |   $false   |     -      |
|            alpb (alpb num)*             |     21     | 符号表索引 |            |
|        (ε \| + \| -) $num($num)*        |     22     |  $integer  | 常数表索引 |
| (ε \| + \| -) $num($num)* . $num($num)* |     23     | $floatnum  | 常数表索引 |
|                 “any\*"                 |     24     |    $str    | 常数表索引 |
|                    =                    |     25     |  $assign   |     -      |
|                    +                    |     26     |   $plus    |     -      |
|                    -                    |     27     |    $sub    |     -      |
|                    *                    |     28     |    $mul    |     -      |
|                    /                    |     29     |    $div    |     -      |
|                    %                    |     30     |    %mod    |     -      |
|                   ==                    |     31     |    $eq     |     -      |
|                    >                    |     32     |    $gre    |     -      |
|                   >=                    |     33     |    $geq    |     -      |
|                    <                    |     34     |    $les    |     -      |
|                   <=                    |     35     |    $leq    |     -      |
|                   !=                    |     36     |    $neq    |     -      |
|                    &                    |     37     |    $and    |     -      |
|                   \|                    |     38     |    $or     |     -      |
|                    !                    |     39     |    $not    |     -      |
|                    ;                    |     40     |    $smc    |     -      |
|                    ,                    |     41     |    $cma    |     -      |
|                    (                    |     42     |   $lpar    |     -      |
|                    )                    |     43     |   $rpar    |     -      |
|                    {                    |     44     |   $lbrc    |     -      |
|                    }                    |     45     |   $rbrc    |     -      |
|                    [                    |     46     |   $lsbrc   |     -      |
|                    ]                    |     47     |   $rsbrc   |     -      |
|                    :                    |     48     |   $colon   |     -      |
|                  null                   |     49     |   $null    |     -      |
|           [a-z] \| [A-Z] \| _           |   基本字   |   $alpb    |     -      |
|                  [0-9]                  |   基本字   |    $num    |     -      |
|                    #                    | 预处理内容 |    $prc    |     -      |
|       (\t \|   \r \| \l \| 空格)*       | 预处理内容 |    $ws     |     -      |
|                 include                 | 预处理内容 |   $inclu   |     -      |
|                 define                  | 预处理内容 |  $define   |     -      |
|              /* (any)\* */              | 预处理内容 |   $cmtm    |     -      |
|                   //                    | 预处理内容 |   $cmts    |     -      |



### 语法产生式



> - 所有以 `$` 起始的符号均为上文中词法识别结果作为终结符使用；
> - 普通单词表示非终结符，其中第一个文法产生式的 LHS 符号为文法起始符号；
> - 文法使用 BNF ， `->` 表示推出。



|                      LL(1) 语言生成文法                      |
| :----------------------------------------------------------: |
| Program -> Type ID \$lpar InnerVarDeclare \$rpar SyntaxBlock |
|                    Type -> \$int \|\$void                    |
|  SyntaxBlock -> \$lbrc OuterVarDeclare SyntaxCluster \$rbrc  |
|                  InnerVarDeclare -> \$blank                  |
| InnerVarDeclareEnd -> \$blank \|\$cma VariableDeclare InnerVarDeclareEnd |
| OuterVarDeclare -> \$blank \|VariableDeclare \$smc OuterVarDeclare |
|                  VariableDeclare -> Type ID                  |
|       SyntaxCluster -> Syntax \$smc SyntaxClusterLoop        |
| SyntaxClusterLoop -> \$blank \|Syntax \$smc SyntaxClusterLoop |
|                         Syntax -> If                         |
|               Assign -> ID \$assign Expression               |
|           Return -> \$return \|\$return Expression           |
|    While -> \$while \$lpar Expression \$rpar SyntaxBlock     |
|     If -> \$if \$lpar Expression \$rpar SyntaxBlock Else     |
|             Else -> \$blank \|\$else SyntaxBlock             |
|             Expression -> AddExpr ExpressionLoop             |
|                   ExpressionLoop -> $blank                   |
|                    Relop -> \$eq \|\$gre                     |
|                 AddExpr -> Item AddExprLoop                  |
|       AddExprLoop -> \$blank \|$plus Item AddExprLoop        |
|                   Item -> Factor ItemLoop                    |
|          ItemLoop -> \$blank \|$mul Factor ItemLoop          |
|                         Factor -> ID                         |
|                          ID -> \$id                          |
|                Num -> \$integer \|\$floatnum                 |



### 代码分析

以下示例代码为错误参考，但是错误处理机制能够使分析器继续运行：

```c
/* This is a comment */

#define ZERO 0

// The  float  is a redundant return type grammar fault 
// The last comma in program's parameter list  is a grammar fault
int float program ( int a, int b, int c ,)
{
	int i;
	int j;
	i = ZERO;
	// void type variable declaration grammar fault
	void;

	if ( a > ( b + c ) ) {
		j = a + ( b * c + 1 );
	} else {
		j = a;
	};

	while( i <= 100 ) {
		i = j * 2;
	}; ;	// Redundant simecolon grammar fault
	return i;
}
```



## 运行效果

### 单步调试效果

![](https://raw.githubusercontent.com/xw1216/ImageHosting/main/img/%E5%B1%8F%E5%B9%95%E6%88%AA%E5%9B%BE%202021-11-20%20201118.png)



### 全部调试错误处理效果

![](https://raw.githubusercontent.com/xw1216/ImageHosting/main/img/%E5%B1%8F%E5%B9%95%E6%88%AA%E5%9B%BE%202021-11-20%20201749.png)



## 项目总结

:pensive: 经此一役，越来越觉得自己的效率似乎在朝着低于预期的某个值不断收敛。其实期望能够尽快写完这些，但是还是花了差不多6天的时间。

理论上后端的写法能够接受用户去改变语法产生式，只要在词法范围内任意的LL(1)均受支持。但是由于测试仓促，实际上也还是建议使用默认的文法。

其中 `First`， `Follow` 两个重要集合的求法使用的是递归形式，文法的左递归， 直接右递归能够在正常的处理过程中被消除，但是间接右递归可能会造成计算函数无限递归。如果有可能的话应该修改成迭代且懒加载的逼近方式计算。

大概就写到这里吧~ :happy: 