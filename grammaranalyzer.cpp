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

void GrammarAnalyzer::initGrammarAnalyzer()
{
    errMsg.clear();
    if(parser == nullptr)
    {
        sendErrorMsg("未能建立语法解析器");
        return;
    }

    if(lex == nullptr) {
        sendErrorMsg("未连接词法分析器");
        return;
    }

    if(!(parser->initParser())) {
        sendErrorMsg("无法打开语法描述文件");
        return;
    }

}

void GrammarAnalyzer::parseGrammarFile()
{
    if(!(errMsg.isEmpty())) {return ;}
    parser->startParser();
    parser->getGrammar(grammar);
    if(grammar.size() <= 0) {
        sendErrorMsg("语法解析错误");
        return;
    }
    startNonTerm = grammar[0]->getLhs();
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
            lhsPrime->refInc();
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
    QVector<bool> usedProd;
    usedProd.resize(grammar.size());

    markUsedSymbol(usedProd);
    freeUnusedSymbol(usedProd);

    int removeCnt = 0;
    for(int i = 0; i < grammar.size(); i++) {
        if(usedProd[i + removeCnt] == false) {
            grammar.removeAt(i);
            removeCnt = removeCnt + 2;
            i--;
        }
    }
}

void GrammarAnalyzer::markUsedSymbol(QVector<bool> & usedProd)
{
    for(int i = 0; i < usedProd.size(); i++) {
        usedProd[i] = false;
    }

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
                    bfsQeuee.push_back((NonTerminal*)grammar[indexOfProd]->rhs[i][j]);
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
    if(!nonTerm->follow.isEmpty()) { return nonTerm->follow; }
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

void GrammarAnalyzer::clearSetCalcuResult()
{
    for(int i = 0; i < grammar.size(); i++) {
        grammar[i]->lhs->first.clear();
        grammar[i]->lhs->follow.clear();
        grammar[i]->lhs->synch.clear();
    }
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
                freeList.push_back(grammar[i]->getLhs());
            }

            for(int j = 0; j < grammar[i]->rhs.size(); j++) {
                for(int k = 0; k < grammar[i]->rhs[j].size(); k++) {
                    grammar[i]->rhs[j][k]->refDec();
                    if(grammar[i]->rhs[j][k]->canDestroy()) {
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
