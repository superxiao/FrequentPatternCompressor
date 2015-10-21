//
//  Trie.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 10/16/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#include "Trie.hpp"

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
        char c = str[i];
        if (currNode->children[c] == NULL) {
            return false;
        } else {
            currNode = currNode->children[c];
        }
    }
    return true;
}

bool Trie::GoToParent(){
    if(currNode->parent != NULL) {
        currNode = currNode->parent;
        return true;
    }
    return false;
}

bool Trie::GoToSibling(char sibling){
    if(currNode->parent != NULL && currNode->parent->children[sibling] != NULL) {
        currNode = currNode->parent->children[sibling];
        return true;
    }
    return false;
}

void Trie::SetIndex(int index){
    currNode->index = index;
    indexedNodes[index] = currNode;
}

void Trie::SetUsageByIndex(int index, int usage){
    indexedNodes[index]->usage = usage;
}

int Trie::GetUsageByIndex(int index){
    return indexedNodes[index]->usage;
}

int Trie::GetCodeByIndex(int index){
    return indexedNodes[index]->code;
}

int Trie::GetCodeLenByIndex(int index){
    return indexedNodes[index]->codeLen;
}

void Trie::ClearUsage(){
    for (int i = 0; i < indexedNodes.size(); i++) {
        if (indexedNodes[i] == NULL)
            break;
        indexedNodes[i]->usage = 0;
    }
}

int Trie::GetTotalEncodedLengthInBits(){
    return totalEncodedLengthIntBits;
}

string Trie::GetString(){
    return currNode->string;
}

void Trie::AddChildNode(char c){
    size++;
    Node* newChild = new Node();
    currNode->children[c] = newChild;
    currNode->frequentChildren.push_back(newChild);
    newChild->string = currNode->string + c;
    newChild->parent = currNode;
}

int Trie::MySize(){
    return size;
}

void Trie::Remove(char c){
    size--;
    delete currNode->children[c];
    currNode->children[c] = NULL;
}

bool Trie::HasChild(char c){
    return currNode->children[c] != NULL;
}

void Trie::AddPatternPositionToChild(char c, const Position& position){
    Node* child = currNode->children[c];
    child->patternPositions.push_back(position);
}

void Trie::PruneInfrequentChildren(int minSupport){
    for(auto itr = currNode->frequentChildren.begin(); itr != currNode->frequentChildren.end();) {
        Node* node = *itr;
        if (node->patternPositions.size() < minSupport) {
            this->Remove(node->string.back());
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
    for (const string& string : strings) {
        currNode = root;
        for (char c : string) {
            if (!currNode->children[c]) {
                currNode->children[c] = new Node();
            }
            currNode = currNode->children[c];
        }
    }
}
    