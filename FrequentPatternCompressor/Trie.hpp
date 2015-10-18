//
//  Trie.hpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 10/16/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#ifndef Trie_hpp
#define Trie_hpp

#include <stdio.h>
#include <vector>
#include "Node.hpp"

struct Position {
    size_t stringIndex;
    size_t positionInString;
};

class Trie {
private:
    Node* root;
    Node* currNode;
    vector<Node*> indexedNodes;
    int totalEncodedLengthIntBits;
    int size;
public:
    Trie();
    bool Contains(string);
    bool GoToChild(char);
    bool GoToParent();
    bool GoToSibling(char);
    void GoToRoot();
    void IncrementUsage();
    int GetUsage();
    void SetIndex(int);
    int GetIndex();
    void SetUsageByIndex(int index, int usage);
    int GetUsageByIndex(int index);
    int GetCodeByIndex(int index);
    int GetCodeLenByIndex(int index);
    void ClearUsage();
    int GetTotalEncodedLengthInBits();
    string GetString();
    void AddChildNode(char c);
    int MySize();
    void Remove(char);
    bool HasChild(char);
    void AddPatternPositionToChild(char, Position);
    void PruneInfrequentChildren();
};

#endif /* Trie_hpp */
