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
    int totalEncodedLengthIntBits;
    int size = 0;

    Trie();
    ~Trie();
    
    bool Contains(string);
    inline bool GoToChild(uint8_t c) {
        return currNode->children[c]?currNode = currNode->children[c], true: false;
    }
    
    inline void GoToChildOfRoot(uint8_t c) {
        currNode = root->children[c];
    }
    bool GoToParent();
    bool GoToSibling(uint8_t);
    inline void GoToRoot() {
        currNode = root;
    }
    inline void SetIndex(int index) {
        currNode->index = index;
    }
    inline int GetIndex() {
        return currNode->index;
    }
    int GetTotalEncodedLengthInBits();
    inline string GetString() {
        return currNode->str;
    }
    void AddChildNode(uint8_t c);
    int MySize();
    void Remove(uint8_t);
    bool HasChild(uint8_t);
    void AddPatternPositionToChild(uint8_t, const Position&);
    void PruneInfrequentChildren(int minSupport);
    void BuildTrie(const vector<string>& strings);
};

#endif /* Trie_hpp */
