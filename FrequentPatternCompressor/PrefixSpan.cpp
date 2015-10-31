//
//  PrefixSpan.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 10/17/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#include "PrefixSpan.hpp"


Trie* PrefixSpan::BuildTreeWithCharFrequencies(const vector<string>& strings, int minSupport) {
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

vector<Position> projected[256];
vector<uint8_t> ls;

void PrefixSpan::DepthFirstSearchForFrequentPatterns(Trie* tree, int prefixLen,
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
        DepthFirstSearchForFrequentPatterns(tree, prefixLen + 1,
                                            tree->currNode->patternPositions, strings, minSupport);
        tree->currNode = currNode;
    }
}

Trie* PrefixSpan::GetFrequentPatterns(const vector<string>& strings, int minSupport) {
    Trie* tree = BuildTreeWithCharFrequencies(strings, minSupport);
    // bug
    Node** children = tree->currNode->children;
    for (int i = 0; i < 256; i++) {
        if (children[i] == NULL) {
            children[i] = new Node();
            children[i]->str = (char) i;
            continue;
        }
        Node* currNode = tree->currNode;
        tree->currNode = children[i];
        DepthFirstSearchForFrequentPatterns(tree, 1, tree->currNode->patternPositions,
                                            strings, minSupport);
        tree->currNode = currNode;
    }
    return tree;
}
