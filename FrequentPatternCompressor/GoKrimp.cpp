//
//  GoKrimp.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 11/13/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#include "GoKrimp.hpp"
#include "PatternGrowthCommons.hpp"

GoKrimp::GoKrimp() {
    
}

Trie* GoKrimp::GetCompressingPatterns(const vector<string>& strings, int minSupport) {
    return BuildTreeWithCharFrequencies(strings, minSupport);
}