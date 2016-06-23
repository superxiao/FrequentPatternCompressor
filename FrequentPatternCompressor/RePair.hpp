//
//  RePair.hpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 4/15/16.
//  Copyright © 2016 Xiaojian Wang. All rights reserved.
//

#ifndef RePair_hpp
#define RePair_hpp

#include <stdio.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <boost/heap/fibonacci_heap.hpp>
#include "Trie.hpp"

using namespace std;
using namespace boost::heap;

struct ThreadedCell;
struct PairList;

struct compare_pair_node
{
    inline bool operator()(PairList* const& n1, PairList* const& n2) const;
};

struct SymbolPair {
    uint16_t firstSymbol;
    uint16_t secondSymbol;
};

struct PairList {
    int count = 0;
    ThreadedCell* head = NULL;
    ThreadedCell* tail = NULL;
    fibonacci_heap<PairList*, compare<compare_pair_node>>::handle_type qhandle;
    //uint32_t pairPhrase; // First 16 bit is first symbol, second is next
    string phrase = "";
    bool inQueue = false;
    SymbolPair pair;
};

struct ThreadedCell {
    int i = 0;
    int j = 0;
    ThreadedCell* nextPair = NULL;
    ThreadedCell* prevPair = NULL;
    PairList* list = NULL;
    ThreadedCell* symbolHead = NULL;
    int symbolLen = 1;
    uint32_t symbol;
};

class RePair {
public:
    vector<string> getPhrases(const vector<const string*>& strings);
    Trie* getPatternTrie(const vector<const string*>& strings);
    
private:
    inline void initRePair(const vector<const string*>& strings);
    inline string getNextPair(const vector<const string*>& strings);
    inline PairList* appendPairCell(const vector<const string*>& strings, ThreadedCell* cell, uint16_t firstSymbol, uint16_t secondSymbol);
    inline void removePairCell(ThreadedCell* cell);
};

#endif /* RePair_hpp */
