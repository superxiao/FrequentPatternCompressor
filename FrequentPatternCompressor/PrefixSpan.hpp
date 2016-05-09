//
//  PrefixSpan.hpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 10/17/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#ifndef PrefixSpan_hpp
#define PrefixSpan_hpp

#include <stdio.h>
#include <vector>
#include <string>
#include "Trie.hpp"

using namespace std;

class PrefixSpan {
    
public:
    static Trie* GetFrequentPatterns(const vector<const string*>& strings, int minSupport, bool prune);
    
private:
    inline static void PutCharPosition(Trie* tree, size_t stringIdx,
                                size_t charIdxInString, const string& string);
    static void DepthFirstSearchForFrequentPatternsNoPrune(Trie* tree, int prefixLen,
                                                           const vector<Position>& prefixPositions, const vector<const string*>& strings, int minSupport);
    static void DepthFirstSearchForFrequentPatternsShallow(Trie* tree, int prefixLen,
                                                        const vector<Position>& prefixPositions, const vector<const string*>& strings, int minSupport);
    static void DepthFirstSearchForFrequentPatternsDeep(Trie* tree, int prefixLen,
                                                    const vector<Position>& prefixPositions, const vector<const string*>& strings, int minSupport);
};

#endif /* PrefixSpan_hpp */
