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
    static Trie* GetFrequentPatterns(vector<string>& strings, int minSupport);
    
private:
    static Trie* BuildTreeWithCharFrequencies(vector<string>& strings, int minSupport);
    static void PutCharPosition(Trie* tree, int stringIdx,
                                int charIdxInString, string string);
    static bool PutPositionsOfExpanded(Trie* tree, int prefixLen,
                                       Position prefixPos, string string);
    static void DepthFirstSearchForFrequentPatterns(Trie* tree, int prefixLen,
                                                    vector<Position>& prefixPositions, vector<string>& strings, int minSupport);
};

#endif /* PrefixSpan_hpp */
