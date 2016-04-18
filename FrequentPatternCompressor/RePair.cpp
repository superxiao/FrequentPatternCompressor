//
//  RePair.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 4/15/16.
//  Copyright © 2016 Xiaojian Wang. All rights reserved.
//

#include "RePair.hpp"
#include <unordered_set>

bool compare_pair_node::operator()(PairList* const& n1, PairList* const& n2) const
{
    return n1->count < n2->count;
}

vector<string> RePair::getPhrases(const vector<string> &strings) {
    initRePair(strings);
    vector<string> phrases;
    string phrase = getNextPair(strings);
    while(!phrase.empty()) {
        phrases.push_back(phrase);
        phrase = getNextPair(strings);
    }
    return phrases;
}

void RePair::initRePair(const vector<string> & strings) {
    threadedArray.resize(strings.size());
    for (int i = 0; i < strings.size(); i++) {
        threadedArray[i].resize(strings[i].size());
        for (int j = 0; j < strings[i].size() - 1; j++) {
            ThreadedCell& node = threadedArray[i][j];
            node.i = i;
            node.j = j;
            node.symbolHead = &node;
            appendPairCell(strings, &node, 1, 1);
        }
    }
    for (auto& kv : pairTable) {
        kv.second.qhandle = pq.push(&kv.second);
        kv.second.inQueue = true;
    }
}

PairList* RePair::appendPairCell(const vector<string> & strings, ThreadedCell *cell, int firstSymbolLen, int secondSymbolLen) {
    string pairPhrase = strings[cell->i].substr(cell->j, firstSymbolLen + secondSymbolLen);
    PairList* list = &pairTable[pairPhrase];
    cell->list = list;
    if (!list->head) {
        list->head = cell;
        list->tail = cell;
        list->count++;
        list->pairPhrase = pairPhrase;
    }
    else {
        list->tail->nextPair = cell;
        cell->prevPair = list->tail;
        list->tail = cell;
        list->count++;
    }
    return list;
}

void RePair::removePairCell(ThreadedCell *cell) {
    if (!cell->nextPair) {
        cell->list->tail = cell->prevPair;
    }
    if (!cell->prevPair) {
        cell->list->head = cell->nextPair;
    }
    if (cell->prevPair) {
        cell->prevPair->nextPair = cell->nextPair;
    }
    if (cell->nextPair) {
        cell->nextPair->prevPair = cell->prevPair;
    }
    cell->list->count--;
    if(cell->list->inQueue) {
        if (cell->list->count == 1) {
            pq.erase(cell->list->qhandle);
            cell->list->inQueue = false;
        }
        else if(cell->list->count > 1) {
            pq.decrease(cell->list->qhandle);
        }
    }
    cell->prevPair = NULL;
    cell->nextPair = NULL;
    cell->list = NULL;
}


string RePair::getNextPair(const vector<string> & strings) {
    if (pq.size() == 0) {
        return "";
    }
    PairList* maxPair = pq.top();
    if (maxPair->count <= 1) {
        return "";
    }
    pq.pop();
    maxPair->inQueue = false;
    
    assert(maxPair->head->list == maxPair);
    assert(maxPair->tail->list == maxPair);
    
    int symbolLen = maxPair->pairPhrase.size();
    ThreadedCell* node = maxPair->head;
//    
//    ThreadedCell* node2 = maxPair->head;
//    while(node2) {
//        if (node2->list != maxPair) {
//            int k = 0;
//        }
//        node2 = node2->nextPair;
//    }
    
    while(node) {
        
        ThreadedCell tempCell = * node;
        int originalSymbolLen = node->symbolLen;
        node->symbolLen = symbolLen;
        
        int i = node->i;
        int j = node->j;
        
        threadedArray[i][j + symbolLen - 1].symbolHead = node;
        
        removePairCell(node);
        
        if (j > 0) {
            ThreadedCell* leftSymbolCell = threadedArray[i][j-1].symbolHead;
            removePairCell(leftSymbolCell);
            PairList* list = appendPairCell(strings, leftSymbolCell, leftSymbolCell->symbolLen, symbolLen);
            if(!list->inQueue && list->count > 1) {
                list->qhandle = pq.push(list);
                list->inQueue = true;
            }
        }
        if (j + symbolLen < threadedArray[i].size()) {
            ThreadedCell* secondSymbolCell = &threadedArray[i][j + originalSymbolLen];
            removePairCell(secondSymbolCell);
            PairList* list = appendPairCell(strings, node, symbolLen, threadedArray[i][j + symbolLen].symbolLen);
            if(!list->inQueue && list->count > 1) {
                list->qhandle = pq.push(list);
                list->inQueue = true;
            }
        }
        node = tempCell.nextPair;
        while (node) {
            if (node->i != i || node->j >= j + symbolLen) {
                break;
            }
            node = node->nextPair;
        }
    }
    return maxPair->pairPhrase;
}

Trie* RePair::getPatternTrie(const vector<string>& strings) {
    auto patterns = this->getPhrases(strings);
//    unordered_set<string> patternSet(patterns.begin(), patterns.end());
//    for (const string& pattern : patternSet) {
//        for (int i = 1; i < pattern.size(); i++) {
//            patternSet.insert(pattern.substr(i, pattern.size() - i));
//        }
//    }
//    vector<string> augPatterns(patternSet.begin(), patternSet.end());
    Trie* trie = new Trie();
//    trie->BuildTrie(augPatterns);
    trie->BuildTrie(patterns);
    return trie;
}