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
    bool operator()(PairList* const& n1, PairList* const& n2) const;
};

struct PairList {
    int count = 0;
    ThreadedCell* head = NULL;
    ThreadedCell* tail = NULL;
    fibonacci_heap<PairList*, compare<compare_pair_node>>::handle_type qhandle;
    string pairPhrase = "";
    bool inQueue = false;
};

struct ThreadedCell {
    int i = 0;
    int j = 0;
    ThreadedCell* nextPair = NULL;
    ThreadedCell* prevPair = NULL;
    PairList* list = NULL;
    ThreadedCell* symbolHead = NULL;
    int symbolLen = 1;
};

class RePair {
public:
    vector<string> getPhrases(const vector<string>& strings);
    Trie* getPatternTrie(const vector<string>& strings);
    
private:
    void initRePair(const vector<string>& strings);
    string getNextPair(const vector<string>& strings);
    fibonacci_heap<PairList*, compare<compare_pair_node>> pq;
    vector<vector<ThreadedCell>> threadedArray;
    unordered_map<string, PairList> pairTable;
    PairList* appendPairCell(const vector<string>& strings, ThreadedCell* cell, int firstSymbolLen, int secondSymbolLen);
    void removePairCell(ThreadedCell* cell);
};

#endif /* RePair_hpp */
