//
//  PrefixSpan.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 10/17/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#include "PrefixSpan.hpp"
#include "PatternGrowthCommons.hpp"

vector<Position> projected[256];
vector<uint8_t> ls;

void PrefixSpan::DepthFirstSearchForFrequentPatternsShallow(Trie* tree, int prefixLen,
                                                     const vector<Position>& prefixPositions, const vector<string>& strings, int minSupport) {
    for(size_t i = 0; i < prefixPositions.size(); i++)
    {
        Position prefixPos = prefixPositions[i];
        const string& str = strings[prefixPos.stringIndex];
        size_t nextCharPos = prefixPos.positionInString + prefixLen;
        if (nextCharPos < str.length()) {
            uint8_t c = str[nextCharPos]; // TODO better way?
            projected[c].push_back(prefixPos);
            if (projected[c].size() == 1) {
                ls.push_back(c);
            }
        }
    }
    for(auto poss : ls) {
        auto& vec = projected[poss];
        if (vec.size() >= minSupport) {
            tree->AddChildNode(poss, &vec);
        }
        vec.clear();
    }
    ls.clear();
    auto& children = tree->currNode->frequentChildren;
    Node* currNode = tree->currNode;
    for (auto itr = children.begin(); itr != children.end(); itr++) {
        tree->currNode = *itr;
        if (prefixLen < 2) {
            DepthFirstSearchForFrequentPatternsShallow(tree, prefixLen + 1,
                                            tree->currNode->patternPositions, strings, minSupport);
        }
        else {
            DepthFirstSearchForFrequentPatternsDeep(tree, prefixLen + 1, tree->currNode->patternPositions, strings, minSupport);
        }
        tree->currNode = currNode;
    }
}

void PrefixSpan::DepthFirstSearchForFrequentPatternsDeep(Trie* tree, int prefixLen,
                                                     const vector<Position>& prefixPositions, const vector<string>& strings, int minSupport) {
    for(size_t i = 0; i < prefixPositions.size(); i++)
    {
        Position prefixPos = prefixPositions[i];
        const string& str = strings[prefixPos.stringIndex];
        size_t nextCharPos = prefixPos.positionInString + prefixLen;
        if (nextCharPos < str.length()) {
            uint8_t c = str[nextCharPos]; // TODO better way?
            projected[c].push_back(prefixPos);
            if (projected[c].size() == 1) {
                ls.push_back(c);
            }
        }
    }

    int childrenNum = 0;
    auto poss = ls.begin();
    uint8_t max = 0;
    int maxSize = 0;
    while (poss != ls.end()){
        auto& vec = projected[*poss];
        if (vec.size() > maxSize) {
            maxSize = vec.size();
            max = *poss;
        }
        poss++;
        
    }
    if (ls.size() == 0 || projected[max].size() < minSupport) {
        for(auto poss : ls) {
            auto& vec = projected[poss];
            vec.clear();
        }
        ls.clear();
        return;
    }
    
    tree->currNode->partial += max;
    tree->currNode->partialLen++;
    tree->currNode->indices.resize(tree->currNode->indices.size() + 1);
    tree->currNode->patternPositions = projected[max];

    for(auto poss : ls) {
        auto& vec = projected[poss];
        vec.clear();
    }
    ls.clear();

    DepthFirstSearchForFrequentPatternsDeep(tree, prefixLen + 1,
                                        tree->currNode->patternPositions, strings, minSupport);
    return;
}

Trie* PrefixSpan::GetFrequentPatterns(const vector<string>& strings, int minSupport) {
    Trie* tree = BuildTreeWithCharFrequencies(strings, minSupport);
    // bug
    Node** children = tree->currNode->children;
    for (int i = 0; i < 256; i++) {
        if (children[i]->patternPositions.size() < minSupport) {
            continue;
        }
        Node* currNode = tree->currNode;
        tree->currNode = children[i];
        DepthFirstSearchForFrequentPatternsShallow(tree, 1, tree->currNode->patternPositions,
                                            strings, minSupport);
        tree->currNode = currNode;
    }
    return tree;
}
