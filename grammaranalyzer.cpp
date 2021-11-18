#include "grammaranalyzer.h"

GrammarAnalyzer::GrammarAnalyzer()
{
    parser = new GrammarParser();
    endTerm = new Terminal();
    endTerm->setName("end");
    blankTerm = new Terminal();
    blankTerm->setName("");
}

GrammarAnalyzer::~GrammarAnalyzer()
{
    resetGrammar();
    resetAnalyTable();
    if(parser != nullptr) {
        delete  parser;
        parser = nullptr;
    }
    lex = nullptr;
    if(endTerm != nullptr) {
        delete  endTerm;
        endTerm = nullptr;
    }
    if(blankTerm != nullptr) {
        delete blankTerm;
        blankTerm = nullptr;
    }
}

void GrammarAnalyzer::setLexAnalyzer(LexAnalyzer *lexPtr)
{
    this->lex = lexPtr;
}

bool GrammarAnalyzer::initGrammarAnalyzer()
{
    errMsg.clear();
    if(parser == nullptr)
    {
        sendErrorMsg("未能建立语法解析器");
        return false;
    }

    if(lex == nullptr) {
        sendErrorMsg("未连接词法分析器");
        return false;
    }

    if(!(parser->initParser())) {
        sendErrorMsg("无法打开语法描述文件");
        return false;
    }
    return true;
}

bool GrammarAnalyzer::parseGrammarFile()
{
    if(!(errMsg.isEmpty())) {return false;}
    parser->startParser();
    parser->getGrammar(grammar);
    if(grammar.size() <= 0) {
        sendErrorMsg("语法解析错误");
        return false;
    }
    startNonTerm = grammar[0]->getLhs();
    return true;
}

void GrammarAnalyzer::establishGrammar()
{
    resetGrammar();
    removeLeftRecursion();
    factoringProduction();
    recordSymbols();
}

void GrammarAnalyzer::establishAnalyTable()
{
    reScaleAnalyTable();
    for(int i = 0; i < grammar.size(); i++) {
        // 根据 First 集填入表达式 根据 Synch 集填入 Synch 符号
        fillTableProd(grammar[i]);
        // 其他位置默认为空
    }
}

void GrammarAnalyzer::resetGrammar()
{
    recordSymbols();
    for(int i = 0; i < nonTermimals.size(); i++) {
        delete nonTermimals[i];
    }
    for(int i = 0; i < termimals.size(); i++) {
        delete  termimals[i];
    }
    for(int i = 0; i < grammar.size(); i++) {
        delete  grammar[i];
    }
    nonTermimals.clear();
    termimals.clear();
    grammar.clear();
    startNonTerm = nullptr;
}

int GrammarAnalyzer::grammarAnalyStep()
{
    if(isEnd) {return 1; }
    int lexStatus = getLexInput();

    if(lexStatus < 0) {
        sendErrorMsg("词法分析器错误");
        return -1;
    } else if(lexStatus == 1) {
        lexName = "end";
        lexContent = "";
        isEnd = true;
        return analyseHandler();
    } else {
        return analyseHandler();
    }
}

void GrammarAnalyzer::resetAnalyStatus()
{
    analyStack.clear();
    isNeedNextInput = false;
    isEnd = false;
    outMsgList.clear();
    outMsg = OutMsg();
}

QString GrammarAnalyzer::getErrMsg()
{
    return errMsg;
}

GrammarAnalyzer::OutMsg GrammarAnalyzer::getOutputMsg()
{
    return outMsg;
}

void GrammarAnalyzer::sendAnalyOutput(QString action)
{
    outMsg.stackTop = printStackTop();
    outMsg.inputType = lexName;
    outMsg.inputCont = lexContent;
    outMsg.action = action;
    outMsgList.push_back(outMsg);
}

void GrammarAnalyzer::resetGrammarAnalyzer()
{
    if(parser) { parser->reset(); }
    errMsg.clear();
    resetGrammar();
    resetAnalyTable();
    resetAnalyStatus();
}

void GrammarAnalyzer::removeLeftRecursion()
{
    int grammarNum = grammar.size();
    for(int i = 0; i < grammarNum; i++) {
         for( int j = 0; j < i - 1; j++) {
             Production* target = grammar[i];
             Production* replace = grammar[j];
             replaceIndirectLeftRecursion(target, replace);
             if(target->isLeftRecursion()) {
                 replaceDirectLeftRecursion(target);
             }
         }
    }
    establishLhsProdRelation();
    simplifyUnusedProduction();
    establishLhsProdRelation();

    // TODO 需要移除语法式内相同的产生式
    for(int i = 0; i < grammar.size(); i++) {
        mergeSameProd(grammar[i]);
    }
}

void GrammarAnalyzer::replaceDirectLeftRecursion(Production *target)
{
    NonTerminal* lhsPrime = new NonTerminal();
    lhsPrime->setName(target->lhs->getName() + "'");
    lhsPrime->refInc();

    Production* newProd = new Production();
    newProd->setLhs(lhsPrime);
    if(newProd != nullptr) { grammar.push_back(newProd); }

    int prodNum = target->rhs.size();
    for(int i = 0; i < prodNum; i++) {
        if(target->rhs[i].size() <= 0) { continue; }
        if(target->lhs->getName() == target->rhs[i][0]->getName()) {
            // P -> PA create P' -> AP'
            // No Memory Leak
            QVector<Symbol*> temp = target->rhs[i];
            temp.removeAt(0);
            temp.push_back(lhsPrime);
            refArrayInc(temp);
            newProd->insertProduction(temp);

            // and remove P -> PA
            refArrayDec(target->rhs[i]);
            target->rhs.removeAt(i);
            prodNum--;
            i--;
        } else {
            // P -> B to P -> BP'
            lhsPrime->refInc();
            target->rhs[i].push_back(lhsPrime);
        }
    }
    // Add epsilon
    newProd->newProduction();
}

void GrammarAnalyzer::replaceIndirectLeftRecursion(Production *target, Production *replace)
{
    // replace Pi -> PjA , Pj ->B to Pi -> BA
    // No Memory Leak
     int prodNum = target->rhs.size();
    for(int i = 0; i < prodNum; i++) {
        if(target->rhs[i].size() <= 0) { continue; }
        if(target->rhs[i][0] == replace->lhs) {
            // Temp Pi -> PjA to Pi -> A
            QVector<Symbol*> dst = target->rhs[i];
            dst.removeAt(0);

            // Every Pj ->B to Pi -> BA
            for(int j = 0; j < replace->rhs.size(); j++) {
                QVector<Symbol*> temp = replace->rhs[j] + dst;
                refArrayInc(temp);
                // Every Pi -> BA  pushed
                target->rhs.push_back(temp);
            }

            // remove original Pi -> PjA
            refArrayDec(target->rhs[i]);
            target->rhs.removeAt(i);
            prodNum--;
            i--;
        }
    }
}

void GrammarAnalyzer::simplifyUnusedProduction()
{
    QVector<bool> usedProd(grammar.size(), false);

    markUsedSymbol(usedProd);
    freeUnusedSymbol(usedProd);

    for(int i = grammar.size() - 1; i >= 0; i--) {
        if(usedProd[i] == false) {
            grammar.removeAt(i);
            i--;
        }
    }
}

void GrammarAnalyzer::markUsedSymbol(QVector<bool> & usedProd)
{
    QVector<NonTerminal*> bfsQeuee;
    bfsQeuee.push_back(startNonTerm);
    while(!(bfsQeuee.isEmpty())) {
        NonTerminal* nonTermTemp = bfsQeuee.front();
        bfsQeuee.pop_front();
        int indexOfProd = nonTermTemp->getIndexOfProd();
        usedProd[indexOfProd] = true;
        for(int i = 0; i < grammar[indexOfProd]->rhs.size(); i++) {
            for(int j = 0; j < grammar[indexOfProd]->rhs[i].size(); j++) {
                if(grammar[indexOfProd]->rhs[i][j]->getType() == Symbol::Type::NONTERMINAL) {
                    if(!usedProd[((NonTerminal*)(grammar[indexOfProd]->rhs[i][j]))->getIndexOfProd()])
                    { bfsQeuee.push_back((NonTerminal*)grammar[indexOfProd]->rhs[i][j]); }
                }
            }
        }
    }
}

void GrammarAnalyzer::recordSymbols()
{
    for(int i = 0; i < grammar.size(); i++) {
        recordNonTerm(grammar[i]->getLhs());
        for(int j = 0; j < grammar[i]->rhs.size(); j++) {
            for(int k = 0; k < grammar[i]->rhs[j].size(); k++) {
                if(!(grammar[i]->rhs[j][k])) {continue; }
                if(grammar[i]->rhs[j][k]->getType() == Symbol::Type::NONTERMINAL) {
                    recordNonTerm((NonTerminal *)grammar[i]->rhs[j][k]);
                } else if(grammar[i]->rhs[j][k]->getType() == Symbol::Type::TERMINAL) {
                    recordTerm((Terminal*)grammar[i]->rhs[j][k]);
                }
            }
        }
    }
    recordTerm(endTerm);
}

void GrammarAnalyzer::recordNonTerm(NonTerminal *nonTerm)
{
    bool isDuplicated = false;
    for(int i = 0; i < nonTermimals.size(); i++) {
        if(nonTermimals[i]->getName() == nonTerm->getName()) {
            isDuplicated = true;
            break;
        }
    }
    if(!isDuplicated) {
        nonTermimals.push_back(nonTerm);
    }
}

void GrammarAnalyzer::recordTerm(Terminal *term)
{
    if(term == blankTerm) {return; }
    bool isDuplicated = false;
    for(int i = 0; i < termimals.size(); i++) {
        if(termimals[i]->getName() == term->getName()) {
            isDuplicated = true;
        }
    }
    if(!isDuplicated) {
        termimals.push_back(term);
    }
}


void GrammarAnalyzer::factoringProduction()
{
    // 注意：调用前需要重新登记非终结符的对应产生式编号 且后续新增产生式也要添加 indexOfProd
    // 注意：注意语法式数量变化以及 first follow 集的更新
    for(int i = 0; i < grammar.size(); i++) {
        // 计算该语法的所有的候选首符交集
        QVector<QVector<int>> prodIntersect = calcuProdIntersect(grammar[i]);
        // 若某产生式首字符为非终结符且有候选首符交集 则进行迭代替换直到候选首符均为终结符/空串
        replaceFirstNonTerm(grammar[i], prodIntersect);

        // 合并该语法式中所有的相同产生式
        mergeSameProd(grammar[i]);

        // 计算所有的候选首符交集 本次计算结果可以直接用于提取公因子
        // 提公因子创建新的产生式 合并旧产生式内的同类项 合并空串等相同产生式
        // 由于每次都会改变产生式集 所以需要多次调用交集求解函数 每次仅返回一个交集
        int factorCnt = 0;
        prodIntersect = calcuProdIntersect(grammar[i], false);
        while(!prodIntersect.isEmpty()) {
            factorCnt++;
            createFactorProd(factorCnt++, grammar[i], prodIntersect[0]);
            prodIntersect = calcuProdIntersect(grammar[i], false);
        }
    }
}

void GrammarAnalyzer::replaceFirstNonTerm(Production *target, QVector<QVector<int>> & intersect)
{
    QVector<bool> visited(target->rhs.size(), false);
    QSet<int> removeSet;
    //若计算得到的含有公因子产生式首符为非终结符
    // 反复替换直到终结符或空串表明达到了可以进行公因式提取的状态

    for(int i = 0; i < intersect.size(); i++) {
        for(int j = 0; j < intersect[i].size(); j++) {
            int prodIndex = intersect[i][j];
            if(visited[prodIndex]) {continue; }
            else {
                visited[prodIndex] = true;
                if(target->rhs[prodIndex].size() <= 0) { continue; }
                else if(target->rhs[prodIndex][0]->getType() == Symbol::Type::TERMINAL) {continue; }
                else {
                    removeSet.insert(prodIndex);
                    QVector<QVector<Symbol*>> replaceArray = genPreFactorProd(target->rhs[prodIndex]);
                    for(int i = 0; i < replaceArray.size(); i++) {
                        refArrayInc(replaceArray[i]);
                    }
                    target->rhs += replaceArray;
                }
            }
        }
    }

    // 可以删除原有的产生式
    QList<int> removeList = removeSet.values();
    std::sort(removeList.begin(), removeList.end());
    for(int i = removeList.size() - 1; i >= 0; i--) {
        int removeIndex = removeList[i];
        refArrayDec(target->rhs[removeIndex]);
        target->rhs[removeIndex];
    }
}

void GrammarAnalyzer::mergeSameProd(Production *target)
{
    for(int i = target->rhs.size() - 1; i >= 0 ; i--) {
        for(int j = target->rhs.size() - 1; j > i; j--) {
            if (target->rhs[i].size() != target->rhs[j].size()) {
                continue;
            }
            else if(target->rhs[i].size() == 0 && target->rhs[j].size() == 0) {
                target->rhs.remove(j);
            }
            else {
                bool isProdSame = true;
                for(int k = 0; k < target->rhs[i].size(); k++) {
                    if(target->rhs[i][k] != target->rhs[j][k]) { isProdSame = false; break; }
                }
                if(!isProdSame) { continue; }
                else {
                    // 移除位置靠后的相同产生式 这样就不同手动递减控制变量
                    refArrayDec(target->rhs[j]);
                    target->rhs.remove(j);
                }
            }
        }
    }
}

// 计算当前语法产生式的候选首集的交集对应右部的索引 直到最后一个被找出后返回空集
QVector<QVector<int>> GrammarAnalyzer::calcuProdIntersect(Production *target, bool isGroup)
{
    // 利用多重 Map 求得拥有某一特定公共候选符的所有产生式索引
    QMultiMap<Symbol*, int> multiMap = QMultiMap<Symbol*, int>();
    QVector<QVector<int>> indexList;
    for(int i = 0; i < target->rhs.size(); i++) {
        QSet<Symbol*> temp = calcuFirst(target->rhs[i]);
        for(QSet<Symbol*>::Iterator iter = temp.begin(); iter != temp.end(); iter++) {
            multiMap.insert(*iter, i);
        }
    }
    QList<Symbol*> keyList = multiMap.uniqueKeys();
    for(int i = 0; i < keyList.size(); i++) {
        QVector<int> temp = multiMap.values(keyList[i]).toVector();
        // 确保是公有而非仅出现一次
        if(temp.size() >= 2) {
            std::sort(temp.begin(), temp.end());
            indexList.push_back(temp);
            if(!isGroup) { return indexList; }
        }
    }
    return indexList;
}

int GrammarAnalyzer::maxIntersectLength(Production *target, QVector<int> &intersect)
{
    int factorLength = 0;
    QVector<Symbol*> peekSymList = target->rhs[intersect[0]];

    for(int i = 0; i < peekSymList.size(); i++) {
        bool isSymbolSame = true;
        for(int j = 1; j < intersect.size(); j++) {
            if(i >= target->rhs[j].size() || peekSymList[i] != target->rhs[j][i])  {
                isSymbolSame = false;
                break;
            }
        }
        if(!isSymbolSame) {
            factorLength = i;
            break;
        }
    }
    return factorLength;
}

void GrammarAnalyzer::createFactorProd(int factorCnt, Production * target, QVector<int> & intersect)
{
    if(intersect.size() <= 1) {return;}
    NonTerminal* newTerm = (NonTerminal*) new NonTerminal();
    Production* newProd = (Production*) new Production();
    if(newTerm == nullptr || newProd == nullptr) { return; };

    // 找出指定索引产生式中的最长公因子
    int sameLength = maxIntersectLength(target, intersect);
    QVector<Symbol*> factor(target->rhs[intersect[0]].mid(0, sameLength));
    for(int i = 0; i < intersect.size() - 1; i++) {
        refArrayDec(factor);
    }

    // 创建新的产生式并加入语法集中
    newTerm->setName(target->lhs->getName() + "." + factorCnt);
    newTerm->refInc();
    newProd->setLhs(newTerm);
    for(int i = 0; i < intersect.size(); i++) {
        QVector<Symbol*> temp = target->rhs[intersect[i]].mid(sameLength, -1);
        newProd->rhs.push_back(temp);
    }
    newTerm->setIndexOfProd(grammar.size());
    grammar.push_back(newProd);

    // 更改已经经过提公因子的原有产生式
    factor.push_back(newTerm);
    newTerm->refInc();
    for(int i = intersect.size() - 1; i >= 0; i--) {
        target->rhs.removeAt(intersect[i]);
    }
    target->rhs.push_back(factor);
}

QSet<Symbol*> GrammarAnalyzer::calcuFirst(Symbol *sym)
{
    QSet<Symbol*> first;
    if(sym->getType() == Symbol::Type::TERMINAL) {
        first.insert(sym);
        return first;
    }
    NonTerminal* nonTerm = (NonTerminal*) sym;
    if(!(nonTerm->first.isEmpty())) { return nonTerm->first; }

    Production* symProd = grammar[nonTerm->getIndexOfProd()];
    for(int i = 0; i < symProd->rhs.size(); i++) {
        // 空产生式
        if(symProd->rhs[i].size() <= 0) {
             first.insert(blankTerm);
        }
        // 产生式第一个为终结符
        else if(symProd->rhs[i][0]->getType() == Symbol::Type::TERMINAL) {
                first.insert(symProd->rhs[i][0]);
        }
        // 产生式第一个为非终结符
        else {
            QSet<Symbol*> subFirst;
            for(int j = 0; j < symProd->rhs[i].size(); j++) {
                // 递归调用
                 subFirst.unite(calcuFirst(symProd->rhs[i][j]));
                 // 当前终结符First不含空 则不继续求解
                 if(!subFirst.contains(blankTerm)) {
                     first.unite(subFirst); break;
                 }
                 // 当前终结符First 含空 继续求解下一个终结符
                 else {
                     // 求解到最后一个非终结符 强制结束
                     if(j == symProd->rhs[i].size() - 1) {
                         first.unite(subFirst); break;
                     } else {
                         subFirst.remove(blankTerm);
                     }
                 }
            }
            nonTerm->first = first;
        }
    }
    return first;
}

QSet<Symbol *> GrammarAnalyzer::calcuFirst(QVector<Symbol *> &symbolList)
{
    QSet<Symbol*> first;
    // 输入为空产生式 直接返回 first 集为 epsilon
    if(symbolList.size() <= 0) {
        first.insert(blankTerm);
        return first;
    }

    for(int i = 0; i < symbolList.size(); i++) {
        first.unite(calcuFirst(symbolList[i]));
        // 结果中不含空串 停止搜索 返回
        if(!first.contains(blankTerm)) {
            break;
        }
        // 含空串则并入除了空串的其他终结符 并继续搜索
        else {
            // 搜索到字符串最后 强制结束
            if(i == symbolList.size() - 1) {
                break;
            }
            first.remove(blankTerm);
        }
    }
    return first;
}

QSet<Symbol *> GrammarAnalyzer::calcuFollow(NonTerminal *nonTerm)
{
    QSet<Symbol*> follow;
    if(!(nonTerm->follow.isEmpty())) { return nonTerm->follow; }
    // 文法开始符号 添加 #
    if(nonTerm == startNonTerm) {
        follow.insert(endTerm);
    }

    for(int i = 0; i < grammar.size(); i++) {
        for(int j = 0; j < grammar[i]->rhs.size(); j++) {
            QVector<Symbol*>::Iterator iter = grammar[i]->rhs[j].begin();
            for(int k = 0 ; grammar[i]->rhs[j].size(); k++, iter++) {
                if(nonTerm == grammar[i]->rhs[j][k]) {
                    // 目标非终结符为产生式的最后一个符号 时 直接添加 FOLLOW(B)
                    if(k ==  grammar[i]->rhs[j].size() - 1 && grammar[i]->rhs[j][k] != grammar[i]->lhs ) {
                        follow.unite(calcuFollow(grammar[i]->lhs));
                        continue;
                    }
                    // 目标非终结符为其他位置的符号时 计算 First(beta)
                    QVector<Symbol*> temp(iter + 1, grammar[i]->rhs[j].end());
                    QSet<Symbol*> subFirst = calcuFirst(temp);
                    // 如果First(beta)含空即 beta 能多步推出空串 则额外添加 FOLLOW(B)
                    if(subFirst.contains(blankTerm) && grammar[i]->rhs[j][k] != grammar[i]->lhs) {
                        follow.unite(calcuFollow(grammar[i]->lhs));
                    }
                    // 默认添加 First(beta) - epsilon
                    subFirst.remove(blankTerm);
                    follow.unite(subFirst);
                }
            }
        }
    }
    nonTerm->follow = follow;
    return follow;
}

QSet<Symbol *> GrammarAnalyzer::calcuSynch(NonTerminal *&nonTerm)
{
    if(nonTerm->synch.isEmpty()) {
        nonTerm->synch = calcuFollow(nonTerm);
    }
    return nonTerm->synch;
}

void GrammarAnalyzer::clearSetCalcuResult()
{
    for(int i = 0; i < grammar.size(); i++) {
        grammar[i]->lhs->first.clear();
        grammar[i]->lhs->follow.clear();
        grammar[i]->lhs->synch.clear();
    }
}

void GrammarAnalyzer::reScaleAnalyTable()
{
    if(termimals.size() <= 0 || nonTermimals.size() <= 0) { return; }
    analyTable.resize(nonTermimals.size());
    for(int i = 0; i < analyTable.size(); i++) {
        analyTable[i].resize(termimals.size());
        for(int j = 0; j < analyTable[i].size(); j++ ) {
            analyTable[i][j].type = AnalyTableItem::Type::BLANK;
            analyTable[i][j].prod = nullptr;
        }
    }
}

void GrammarAnalyzer::fillTableProd(Production *prod)
{
    for(int  i = 0; i < prod->rhs.size(); i++) {
           QSet<Symbol *> first = calcuFirst(prod->rhs[i]);
           QSet<Symbol *> follow = calcuFollow(prod->lhs);
           bool isFirstContainsBlank = first.contains(blankTerm);
           first.remove(blankTerm);
           // 若 epsilon 在 First  集中 则对应 Follow集中除 epsilon 的所有终结符置表项
           // 若 epsilon 不在 First  集中 则对应 Follow集中除 epsilon 的所有终结符置同步符号项
           Production* singleProd = (Production*) new  Production();
           singleProd->setLhs(prod->getLhs());
           singleProd->rhs.push_back(prod->rhs[i]);
           for(QSet<Symbol*>::Iterator iter = follow.begin(); iter != follow.end(); iter++) {
               if(isFirstContainsBlank) {
                   setAnalyTableItem(prod->lhs, ((Terminal*)(*iter)), AnalyTableItem::Type::PROD, singleProd);
               } else {
                   setAnalyTableItem(prod->lhs, ((Terminal*)(*iter)), AnalyTableItem::Type::SYNCH);
               }
           }

           for(QSet<Symbol*>::Iterator iter = first.begin(); iter != first.end(); iter++) {
               setAnalyTableItem(prod->lhs, ((Terminal*)(*iter)), AnalyTableItem::Type::PROD, singleProd);
           }
    }
}

bool GrammarAnalyzer::setAnalyTableItem(NonTerminal *lhs, Terminal *term, AnalyTableItem::Type type, Production *prod)
{
    int x = -1, y = -1;
    getTableItemPos(lhs, term, x, y);
    if(x < 0 || y < 0 || !analyTable[x][y].isBlank()) {
        sendErrorMsg("分析表建立错误：表项不存在或入口冲突");
        return false;
    }
    if(type == AnalyTableItem::Type::PROD && prod != nullptr) {
        analyTable[x][y].type = type;
        for(int i = 0 ; i < prod->rhs.size(); i++) {

        }
        analyTable[x][y].prod = prod;
    } else if(type == AnalyTableItem::Type::SYNCH) {
        analyTable[x][y].type = type;
        analyTable[x][y].prod = nullptr;
    }
    return true;
}

void GrammarAnalyzer::getTableItemPos(NonTerminal *lhs, Terminal *term, int &x, int &y)
{
    x = nonTermimals.indexOf(lhs);
    y = termimals.indexOf(term);
}

void GrammarAnalyzer::resetAnalyTable()
{
    for(int i = 0; i < analyTable.size(); i++) {
        for(int j = 0; j < analyTable[i].size(); j++) {
            if(analyTable[i][j].prod != nullptr) {
                delete analyTable[i][j].prod;
            }
        }
        analyTable[i].clear();
    }
    analyTable.clear();
}

void GrammarAnalyzer::initAnalyStack()
{
    resetAnalyStatus();
    analyStack.push_back(endTerm);
    analyStack.push_back(startNonTerm);
    isNeedNextInput = true;
    isEnd = false;
}

bool GrammarAnalyzer::grammarAnalyse()
{
    if(isEnd) {return true;}
    int lexStatus = 0;
    while(lexStatus == 0) {
        lexStatus = getLexInput();
        if(lexStatus < 0) {
            sendErrorMsg("词法分析器错误");
            return false;
        } else if(lexStatus == 1) {
            lexName = "end";
            lexContent = "";
            isEnd = true;
        }

        int analyStatus = analyseHandler();
        if(analyStatus < 0) {
            return false;
        }
        else if(analyStatus > 0) {
            return true;
        }
    }
    return false;
}


int GrammarAnalyzer::analyseHandler()
{
    Terminal* term = findTerminal(lexName);
    if(!term) { sendErrorMsg("语法不支持该符号"); return -1; }

    // 栈顶为非终结符 遇到空则产生错误或跳过 遇到同步符号则弹出 遇到产生式则弹出并倒序压栈产生式右部
    if(analyStack.back()->getType() == Symbol::Type::NONTERMINAL) {
        NonTerminal* nonTerm = (NonTerminal*)analyStack.back();
        int x = -1, y = -1;
        getTableItemPos(nonTerm, term, x, y);
        // 遇到空
        if(analyTable[x][y].isBlank()) {
//            sendErrorMsg("访问了错误的表项"); return -1;
            sendAnalyOutput("访问了错误的表项，跳过输入符号" + lexName);
            isNeedNextInput = true;
        }
        // 遇到同步符号
        else if(analyTable[x][y].isSynch()) {
            sendAnalyOutput("Synch 同步，弹出" + analyStack.back()->getName());
            analyStack.pop_back();
        }
        // 弹出并倒序压栈产生式右部
        else {
            Production* prod = analyTable[x][y].prod;
            sendAnalyOutput(printProduction(prod));
            analyStack.pop_back();
            for(int i = prod->rhs[0].size() - 1; i >= 0; i--) {
                analyStack.push_back(prod->rhs[0][i]);
            }
        }
    }
    // 栈顶为终结符但不是 # end 符号 则弹出并获取新的输入符号
    else if(analyStack.back()->getType() == Symbol::Type::TERMINAL
              && analyStack.back()->getName() != "end") {
            sendAnalyOutput("弹出" + analyStack.back()->getName() + " , 输入前进");
            analyStack.pop_back();
            isNeedNextInput = true;
            return 0;
    }
    // 分析成功
    else if (analyStack.back()->getName() == "end"){
        sendAnalyOutput("语法分析成功");
        return 1;
    }
    return -1;
}

Terminal *GrammarAnalyzer::findTerminal(QString &termName)
{
    for(int i = 0; i < termimals.size(); i++) {
        if(termimals[i]->getName() == termName) {
            return termimals[i];
        }
    }
    return nullptr;
}

int GrammarAnalyzer::getLexInput()
{
    int lexStatus = 0;
    if(isNeedNextInput) {
        lexStatus = lex->lexAnalyByStep(lexName, lexContent);
        if(!lexName.isEmpty()) {
            lexName.remove(0,1);
            isNeedNextInput = false;
        }
    }
    return lexStatus;
}

QString GrammarAnalyzer::printProduction(Production *prod)
{
    QString prodStr;
    prodStr.push_back(prod->lhs->getName());
    prodStr.push_back(" -> ");
    for(int i = 0 ; i < prod->rhs.size(); i++) {
        if(prod->rhs[i].size() <= 0) {
            prodStr.push_back("$blank");
        } else {
            for(int j = 0; j < prod->rhs[i].size(); j++) {
                if(prod->rhs[i][j]->getType() == Symbol::Type::TERMINAL) {
                    prodStr.push_back(" $" + prod->rhs[i][j]->getName() + " ");
                } else {
                    prodStr.push_back(" " + prod->rhs[i][j]->getName() + " ");
                }
            }
        }
        if(i != prod->rhs.size() - 1) {
            prodStr.push_back(" | ");
        }
    }
    return prodStr;
}

QString GrammarAnalyzer::printStackTop()
{
    if(analyStack.last()->getType() == Symbol::Type::TERMINAL) {
        return "$" + analyStack.last()->getName();
    } else {
        return analyStack.last()->getName();
    }
}

QStringList GrammarAnalyzer::printAnalyStack()
{
    QStringList strList;
    for(int i = 0 ; i < analyStack.size(); i++) {
        if(analyStack[i]->getType() == Symbol::Type::TERMINAL) {
            strList.push_back("$" + analyStack[i]->getName());
        } else {
            strList.push_back(analyStack[i]->getName());
        }
    }
    return strList;
}

QStringList GrammarAnalyzer::printGrammar()
{
    QStringList grammarList;
    for(int i  = 0 ; i < grammar.size(); i++) {
        grammarList.push_back(printProduction(grammar[i]));
    }
    return grammarList;
}

void GrammarAnalyzer::getAnalyTableSize(int &row, int &col)
{
    row = analyTable.size();
    col = analyTable[0].size();
}

QString GrammarAnalyzer::getAnalyTableRowHeader(int row)
{
    if(row >= nonTermimals.size() || row < 0) {
        return "";
    }
    return nonTermimals[row]->getName();
}

QString GrammarAnalyzer::getAnalyTableColHeader(int col)
{
    if(col >= termimals.size() || col < 0) {
        return "";
    }
    return termimals[col]->getName();
}

QString GrammarAnalyzer::getAnalyTableItem(int row, int col)
{
    if(row >= nonTermimals.size() || row < 0 || col >= termimals.size() || col < 0) {
        return "";
    }
    AnalyTableItem item = analyTable[row][col];
    if(item.isBlank()) {
        return "~";
    } else if(item.isSynch()) {
        return "Synch";
    } else {
        return printProduction(item.prod);
    }
}

void GrammarAnalyzer::sendErrorMsg(QString errMsg)
{
    this->errMsg = errMsg;
}

void GrammarAnalyzer::establishLhsProdRelation()
{
    for(int i = 0; i < grammar.size(); i++) {
        grammar[i]->getLhs()->setIndexOfProd(i);
    }
}

void GrammarAnalyzer::refArrayInc(QVector<Symbol *> &array)
{
    for(int i = 0; i < array.size(); i++) {
        array[i]->refInc();
    }
}

void GrammarAnalyzer::refArrayDec(QVector<Symbol *> &array)
{
    for(int i = 0; i < array.size(); i++) {
        array[i]->refDec();
    }
}

void GrammarAnalyzer::freeUnusedSymbol(QVector<bool> & usedProd)
{
    QVector<Symbol*> freeList;
    for(int i = 0; i < usedProd.size(); i++) {
        if(usedProd[i] == false) {
            grammar[i]->getLhs()->refDec();
            if(grammar[i]->getLhs()->canDestroy()) {
                grammar[i]->lhs = nullptr;
                freeList.push_back(grammar[i]->getLhs());
            }

            for(int j = 0; j < grammar[i]->rhs.size(); j++) {
                for(int k = 0; k < grammar[i]->rhs[j].size(); k++) {
                    grammar[i]->rhs[j][k]->refDec();
                    if(grammar[i]->rhs[j][k]->canDestroy()) {
                        grammar[i]->rhs[j][k] = nullptr;
                        freeList.push_back(grammar[i]->rhs[j][k]);
                    }
                }
            }
        }
    }

    for(int i = 0; i < freeList.size(); i++) {
        delete freeList[i];
    }
}

QVector<QVector<Symbol *> > GrammarAnalyzer::genPreFactorProd(QVector<Symbol *> initProd)
{
    QVector<QVector<Symbol*> > replaceArray;
    replaceArray.push_back(initProd);
    // 反复迭代 将非终结符所有的可能均予以替换
    for(int i = 0; i < replaceArray.size(); i++) {
        QVector<Symbol*> remainVec = replaceArray[i];
        if(remainVec.size() <= 0 ) { continue; }
        else if (remainVec[0]->getType() == Symbol::Type::TERMINAL) { continue; }
        else {
            NonTerminal* nonTerm = (NonTerminal*)remainVec[0];
            Production* srcProd = grammar[nonTerm->indexOfProd];
            remainVec.removeFirst();
            for(int j = 0; j < srcProd->rhs.size(); j++) {
                replaceArray.push_back(srcProd->rhs[j] + remainVec);
            }
            replaceArray.removeAt(i);
            i--;
        }
    }
    return replaceArray;
}
