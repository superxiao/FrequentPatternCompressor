//
//  PatternGrowthCommons.hpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 11/13/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#ifndef PatternGrowthCommons_hpp
#define PatternGrowthCommons_hpp

#include <stdio.h>
#include "Trie.hpp"

inline Trie* BuildTreeWithCharFrequencies(const vector<string>& strings, int minSupport){
    Trie* tree = new Trie();
    vector<vector<Position>> projected(256);
    for (size_t stringIdx = 0; stringIdx < strings.size(); stringIdx++) {
        const string& string = strings[stringIdx];
        for (size_t charIdx = 0; charIdx < string.length(); charIdx++) {
            uint8_t c = *reinterpret_cast<const uint8_t*>(&strings[stringIdx][charIdx]);
            projected[c].push_back(move(Position{(uint32_t)stringIdx, (uint32_t)charIdx}));
        }
    }
    for(int c = 0; c < 256; c++) {
        if (projected[c].size() >= minSupport) {
            tree->AddChildNode(c, &projected[c]);
        }
    }
    return tree;
}

#endif /* PatternGrowthCommons_hpp */
