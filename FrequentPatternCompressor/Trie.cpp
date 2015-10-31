//
//  Trie.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 10/16/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#include "Trie.hpp"
#include <cassert>
#include <iostream>

Trie::Trie(){
    this->root = new Node();
    this->currNode = this->root;
}

Trie::~Trie(){
    delete root;
    root = NULL;
    currNode = NULL;
}

bool Trie::Contains(string str){
    Node* currNode = root;
    for (int i = 0; i < str.size(); i++) {
        uint8_t c = str[i];
        if (currNode->children[c] == NULL) {
            return false;
        } else {
            currNode = currNode->children[c];
        }
    }
    return true;
}

int Trie::GetTotalEncodedLengthInBits(){
    return totalEncodedLengthIntBits;
}


int Trie::MySize(){
    return size;
}



void Trie::BuildTrie(const vector<string> &strings) {
    Node* currNode;
    for (auto& child : root->children) {
        child = new Node();
    }
    for (const string& str : strings) {
        currNode = root;
        for (char c : str) {
            if (!currNode->children[c]) {
                currNode->children[c] = new Node();
            }
            currNode = currNode->children[c];
        }
    }
}
    