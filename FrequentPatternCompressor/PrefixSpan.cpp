//
//  PrefixSpan.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 10/17/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#include "PrefixSpan.hpp"


Trie* PrefixSpan::BuildTreeWithCharFrequencies(vector<string>& strings, int minSupport) {
    Trie* tree = new Trie();
    for (int stringIdx = 0; stringIdx < strings.size(); stringIdx++) {
        string string = strings[stringIdx];
        for (int charIdx = 0; charIdx < string.length(); charIdx++) {
            PutCharPosition(tree, stringIdx, charIdx, string);
        }
    }
    tree->PruneInfrequentChildren(minSupport);
    return tree;
}

void PrefixSpan::PutCharPosition(Trie* tree, int stringIdx,
                                 int charIdxInString, string string) {
    PutPositionsOfExpanded(tree, 0, Position{static_cast<size_t>(stringIdx), static_cast<size_t>(charIdxInString)},
                           string);
}

bool PrefixSpan::PutPositionsOfExpanded(Trie* tree, int prefixLen,
                                        Position prefixPos, string string) {
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
                                                     vector<Position>& prefixPositions, vector<string>& strings, int minSupport) {
    for(size_t i = 0; i < prefixPositions.size(); i++)
    {
        Position prefixPos = prefixPositions[i];
        string string = strings[prefixPos.stringIndex];
        PutPositionsOfExpanded(tree, prefixLen, prefixPos, string);
    }
    tree->PruneInfrequentChildren(minSupport);
    Node** children = tree->currNode->children; // TODO should be list?
    Node* currNode = tree->currNode;
    for (size_t i = 0; i < 256; i++) {
        Node* node = children[i];
        tree->currNode = node;
        DepthFirstSearchForFrequentPatterns(tree, prefixLen + 1,
                                            tree->currNode->patternPositions, strings, minSupport);
        tree->currNode = currNode;
    }
}

Trie* PrefixSpan::GetFrequentPatterns(vector<string>& strings, int minSupport) {
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
