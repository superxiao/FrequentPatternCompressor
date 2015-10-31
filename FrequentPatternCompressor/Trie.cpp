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

void Trie::AddChildNode(uint8_t c, vector<Position>&& positions){
    size++;
    Node* newChild = new Node();
    currNode->children[c] = newChild;
    currNode->frequentChildren.push_back(newChild);
    newChild->str = currNode->str + (char)c;
    newChild->patternPositions = move(positions);
}

int Trie::MySize(){
    return size;
}

void Trie::Remove(uint8_t c){
    size--;
    delete currNode->children[c];
    currNode->children[c] = NULL;
}

bool Trie::HasChild(uint8_t c){
    return currNode->children[c] != NULL;
}

void Trie::AddPatternPositionToChild(uint8_t c, const Position& position){
    Node* child = currNode->children[c];
    child->patternPositions.push_back(position);
}

void Trie::PruneInfrequentChildren(int minSupport){
    for(auto itr = currNode->frequentChildren.begin(); itr != currNode->frequentChildren.end();) {
        Node* node = *itr;
        if (node->patternPositions.size() < minSupport) {
            this->Remove(node->str.back());
            itr = currNode->frequentChildren.erase(itr);
            continue;
        }
        itr++;
    }
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
    