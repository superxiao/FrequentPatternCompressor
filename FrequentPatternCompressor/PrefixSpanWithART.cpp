//
//  PrefixSpanWithART.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 12/3/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#include "PrefixSpanWithART.hpp"
#include "PatternGrowthCommons.hpp"

vector<Position> projected2[256];
vector<uint8_t> ls2;

void PrefixSpanWithART::DepthFirstSearchForFrequentPatterns(Trie* tree, art_tree* t, int prefixLen,
                                                     const vector<Position>& prefixPositions, const vector<string>& strings, int minSupport) {
    if(prefixLen > 18) {
        art_insert(t, (unsigned char*)&tree->currNode->str[0], tree->currNode->str.length(), NULL);
        return;
    }
    for(size_t i = 0; i < prefixPositions.size(); i++)
    {
        Position prefixPos = prefixPositions[i];
        const string& str = strings[prefixPos.stringIndex];
        size_t nextCharPos = prefixPos.positionInString + prefixLen;
        if (nextCharPos < str.length()) {
            uint8_t c = str[nextCharPos]; // TODO better way?
            projected2[c].push_back(prefixPos);
            if (projected2[c].size() == 1) {
                ls2.push_back(c);
            }
        }
    }
    for(auto poss : ls2) {
        auto& vec = projected2[poss];
        if (vec.size() >= minSupport) {
            tree->AddChildNode(poss, &vec);
        }
        vec.clear();
    }
    ls2.clear();
    auto& children = tree->currNode->frequentChildren;
    
    if (children.size() == 0) {
        art_insert(t, (unsigned char*)&tree->currNode->str[0], tree->currNode->str.length(), NULL);
    }
    
    Node* currNode = tree->currNode;
    for (auto itr = children.begin(); itr != children.end(); itr++) {
        tree->currNode = *itr;
        DepthFirstSearchForFrequentPatterns(tree, t, prefixLen + 1,
                                            tree->currNode->patternPositions, strings, minSupport);
        tree->currNode = currNode;
    }
}

void PrefixSpanWithART::GetFrequentPatterns(const vector<string>& strings, int minSupport, art_tree* t) {
    Trie* tree = BuildTreeWithCharFrequencies(strings, minSupport);
 
    for (int c = 0; c < 256; c++) {
        art_insert(t, (unsigned char*)&c, 1, NULL);
    }
    
    // bug
    Node** children = tree->currNode->children;
    for (int i = 0; i < 256; i++) {
        if (children[i]->patternPositions.size() < minSupport) {
            continue;
        }
        Node* currNode = tree->currNode;
        tree->currNode = children[i];
        DepthFirstSearchForFrequentPatterns(tree, t, 1, tree->currNode->patternPositions,
                                            strings, minSupport);
        tree->currNode = currNode;
    }
    
    delete tree;
}
