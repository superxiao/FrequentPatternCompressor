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
    for (size_t stringIdx = 0; stringIdx < strings.size(); stringIdx++) {
        const string& string = strings[stringIdx];
        for (size_t charIdx = 0; charIdx < string.length(); charIdx++) {
            PutCharPosition(tree, stringIdx, charIdx, string);
        }
    }
    tree->PruneInfrequentChildren(minSupport);
    return tree;
}

void PrefixSpan::PutCharPosition(Trie* tree, size_t stringIdx,
                                 size_t charIdxInString, const string& string) {
    PutPositionsOfExpanded(tree, 0, Position{stringIdx, charIdxInString},
                           string);
}

bool PrefixSpan::PutPositionsOfExpanded(Trie* tree, int prefixLen,
                                        Position prefixPos, const string& string) {
    size_t nextCharPos = prefixPos.positionInString + prefixLen;
    if (nextCharPos < string.length()) {
        char c = string[nextCharPos];
        if (!tree->HasChild(c)) {
            tree->AddChildNode(c);
        }
        tree->AddPatternPositionToChild(c, prefixPos);
        return true;
    }
    return false;
}

void PrefixSpan::DepthFirstSearchForFrequentPatterns(Trie* tree, int prefixLen,
                                                     const vector<Position>& prefixPositions, const vector<string>& strings, int minSupport) {
    for(size_t i = 0; i < prefixPositions.size(); i++)
    {
        Position prefixPos = prefixPositions[i];
        const string& string = strings[prefixPos.stringIndex];
        PutPositionsOfExpanded(tree, prefixLen, prefixPos, string);
    }
    tree->PruneInfrequentChildren(minSupport);
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
    tree->indexedNodes = vector<Node*>();
    tree->indexedNodes.resize(10000);
    Node** children = tree->currNode->children;
    for (int i = 0; i < 256; i++) {
        if (children[i] == NULL) {
            children[i] = new Node();
            children[i]->string = (char) i;
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
