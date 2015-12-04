//
//  PrefixSpanWithART.hpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 12/3/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#ifndef PrefixSpanWithART_hpp
#define PrefixSpanWithART_hpp

#include <stdio.h>
#include <vector>
#include <string>
#include "Trie.hpp"
#include "art.h"

using namespace std;

class PrefixSpanWithART {
    
public:
    static art_tree* GetFrequentPatterns(const vector<string>& strings, int minSupport);
    
private:
    static void DepthFirstSearchForFrequentPatterns(Trie* tree, art_tree* t, int prefixLen,
                                                    const vector<Position>& prefixPositions, const vector<string>& strings, int minSupport);
};

#endif /* PrefixSpanWithART_hpp */
