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

class Trie {
private:

public:
    
    Node* root;
    Node* currNode;
    vector<Node*> indexedNodes;
    int totalEncodedLengthIntBits;
    int size;

    Trie();
    ~Trie();
    
    bool Contains(string);
    inline bool GoToChild(char c) {
        return currNode->children[c]?currNode = currNode->children[c], true: false;
    }
    
    inline void GoToChildOfRoot(char c) {
        currNode = root->children[c];
    }
    bool GoToParent();
    bool GoToSibling(char);
    inline void GoToRoot() {
        currNode = root;
    }
    inline void IncrementUsage() {
        currNode->usage++;
    }
    inline int GetUsage() {
        return currNode->usage;
    }
    void SetIndex(int);
    inline int GetIndex() {
        return currNode->index;
    }
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
    void AddPatternPositionToChild(char, const Position&);
    void PruneInfrequentChildren(int minSupport);
    void BuildTrie(const vector<string>& strings);
};

#endif /* Trie_hpp */
