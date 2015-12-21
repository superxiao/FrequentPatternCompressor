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
    int patternNum = 0;
    int patternLenSum = 0;

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
    inline string GetPrefix() {
        return currNode->prefix;
    }
    inline void AddChildNode(uint8_t c, vector<Position>* positions) {
        size++;
        Node* newChild = new Node();
        currNode->children[c] = newChild;
        currNode->frequentChildren.push_back(newChild);
        newChild->prefix = currNode->prefix + (char)c;
        newChild->partialLen = 0;
        newChild->indices.resize(1);
        newChild->patternPositions = *positions;
        newChild->depth = currNode->prefix.length() - 1;
    }
    
    inline void SetPatternLenSum(int patternLenSum) {
        this->patternLenSum = patternLenSum;
    }
    
    inline double GetPatternLenSum() {
        return this->patternLenSum;
    }
    
    inline void SetPatternNum(int patternNum) {
        this->patternNum = patternNum;
    }
    
    inline double GetPatternNum() {
        return this->patternNum;
    }
    
    int MySize();
    void BuildTrie(const vector<string>& strings);
};

#endif /* Trie_hpp */
