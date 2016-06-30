
//
//  FrequentPatternCompressor.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 10/19/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#include "FrequentPatternCompressor.hpp"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cstring>
#include "PrefixSpan.hpp"
#include <iostream>
#include "Utils.hpp"
#include <cassert>
#include <unordered_set>
#include "RePair.hpp"
#include <cmath>

using namespace std::chrono;
using namespace std;

char out[100 * 1024 * 1024];
int indexEnd = 0;
uint32_t indices[100*1024*1024]; // Dynamic or static no difference
int outEnd = 0;

vector<Node*> leftNodes; // No string should be longer than 255
vector<Node*> rightNodes;
vector<Node*> rightMaxNodes;
vector<vector<Node*>> arraysOfNodes;

void CompareTrie(Node* trie1, Node* trie2) {
    for(int i = 0; i < 256; i++) {
        auto c1 = trie1->children[i];
        auto c2 = trie2->children[i];
        if (!c1 && !c2) {
            continue;
        }
        if (c1 && c2) {
            CompareTrie(c1, c2);
        }
        else {
            if (c1) {
                cout << "Frequent pattern in trie 1 but not trie 2:\n" << c1->prefix<<endl;
                cout << "Frequent pattern in trie 2 but not trie 1:\n" << c2->prefix<<endl;
            }
        }
    }
}

string FrequentPatternCompressor::Compress(const vector<string>& strings, double sample_rate, int support, bool prune, bool lookahead) {
    if (lookahead) {
        leftNodes.resize(1000);
        rightNodes.resize(1000);
        rightMaxNodes.resize(1000);
    }
    indexEnd = 0;
    int sample_size = ceil(strings.size() * sample_rate);
    sample_size = min(sample_size, (int)strings.size());
    this->sampleSize = sample_size;
    //sample_size = min(sample_size, 100);
    vector<const string*> sample(sample_size);
    srand((unsigned)time(NULL));

    for (size_t i = 0; i < sample_size; i++) {
        sample[i] = &strings[rand() % strings.size()];
    }
    
//    support = max(4.0, ceil(0.03 * sample.size()));
    
    Trie* trie = PrefixSpan::GetFrequentPatterns(sample, support, prune);
    
//    RePair repair;
//    Trie* trie = repair.getPatternTrie(sample);
    
    num_trie_nodes = trie->size;
    
    patterns.reserve(strings.size() + 256);
    patterns.push_back("0");
    
    // Using 32 bit here will cause later copying to be slower. But that
    // should be optimized away with load/store.
    uint64_t uncompressed_size = 0;

    if (!prune) {
        for (const string& s : strings) {
            if (lookahead) {
                ForwardCoverWithLookAhead(s, trie);
            } else {
                ForwardCoverShallow(s, trie);
//                ForwardCoverWithGappedPhrases(s, trie);
            }
            uncompressed_size += s.length();
        }
    }
    else {
        for (const string& s : strings) {
            ForwardCoverDeep(s, trie);
            uncompressed_size += s.length();
        }
    }
    
    outEnd = 0;
    memcpy(out, &uncompressed_size, sizeof(uncompressed_size));
    outEnd += sizeof(uncompressed_size);
    
    uint32_t numStrings = (uint32_t)strings.size();
    memcpy(out + outEnd, &numStrings, sizeof(numStrings));
    outEnd += sizeof(numStrings);
    
    unordered_set<string> s;
    
    for(auto &pattern : patterns) {
        s.insert(pattern);
        uint16_t length = pattern.length();
        memcpy(out + outEnd, &length, 2);
        outEnd += 2;
        memcpy(out + outEnd, pattern.c_str(), pattern.length());
        outEnd += pattern.length();
    }

    out[outEnd++] = 0;
    out[outEnd++] = 0;
    
    vector<uint32_t> lens;
    
    size_t compressedSize = 2 * strings.size();

    for(auto& str : strings) {
        lens.push_back((uint32_t)str.size());
    }
    
    auto lenSizeField = reinterpret_cast<uint32_t*>(out + outEnd);
    
    outEnd += 4;
    
    if (outEnd % 16 != 0){
        outEnd += 16 - outEnd % 16;
    }
    
    encodeArray(lens.data(),
                strings.size(),
                reinterpret_cast<uint32_t*>(out + outEnd),
                compressedSize);
    
    *lenSizeField = (uint32_t)compressedSize;
    
    outEnd += compressedSize * 4;
    
    compressedSize = 2 * indexEnd;
    
    encodeArray(reinterpret_cast<uint32_t*>(indices),
                indexEnd,
                reinterpret_cast<uint32_t*>(out + outEnd),
                compressedSize);
    
    string result(out, outEnd + compressedSize * 4);
    delete trie;
    return result;
}

void FrequentPatternCompressor::ForwardCoverShallow(const string& string, Trie* trie){
    Node*& currNode = trie->currNode;
    Node* root = trie->root;
    currNode = root;
    for(uint8_t c : string) {
        Node* child = currNode->children[c];
        if (child) {
            currNode = child;
        } else {
            UseCurrentPattern(currNode);
            currNode = root->children[c];
        }
    }
    UseCurrentPattern(currNode);
}

void FrequentPatternCompressor::ForwardCoverWithLookAhead(const string& string, Trie* trie){
    Node* root = trie->root;
    int i = 0;
    Node* leftNode = root;
    Node* rightNode = root;
    int rightMaxBegin = 0;
    int rightMaxLen = 0;
    int rightBegin = 0;
    while(i != string.size()) {
        int leftBegin = i;
        int leftLen = 0;
        if (rightMaxBegin) {
            leftBegin = rightMaxBegin;
            leftLen = rightMaxLen;
            leftNode = rightMaxNodes[rightMaxLen - 1];
            swap(leftNodes, rightMaxNodes);
        }
        else {
            while(i != string.size()) {
                uint8_t c = string[i];
                Node* child = leftNode->children[c];
                if (child) {
                    leftNode = child;
                    leftNodes[leftLen] = leftNode;
                } else {
                    break;
                }
                i++;
                leftLen++;
            }
            rightBegin = leftBegin + 1;
        }
        
        int leftEnd = leftBegin + leftLen;
        rightMaxBegin = 0;
        rightMaxLen = leftLen;
        int rightLen = 0;
        for (; rightBegin != leftEnd; rightBegin++) {
            rightNode = root;
            int j = rightBegin;
            rightLen = 0;
            while(j != string.size()) {
                uint8_t c = string[j];
                Node* child = rightNode->children[c];
                if (child) {
                    rightNode = child;
                    rightNodes[rightLen] = rightNode;
                } else {
                    if(rightLen > rightMaxLen) {
                        i = j;
                        rightMaxLen = rightLen;
                        rightMaxBegin = rightBegin;
                        std::swap(rightNodes, rightMaxNodes);
                    }
                    break;
                }
                j++;
                rightLen++;
            }
        }
        if(rightMaxBegin) {
            rightBegin = leftEnd;
        }
        
        if (rightMaxBegin == 0) {
            UseCurrentPattern(leftNode);
            leftNode = root;
        } else {
            UseCurrentPattern(leftNodes[rightMaxBegin - leftBegin - 1]);
            leftNode = root;
        }
    }
    int k = 0;
}

void FrequentPatternCompressor::ForwardCoverWithGappedPhrases(const string& string, Trie* trie){
    Node*& currNode = trie->currNode;
    Node* root = trie->root;
    currNode = root;
    Node* candidateNode = NULL;
    int i = 0;
    int j = 0;
    while(i < string.size()) {
        uint8_t c = string[i];
        Node* child = currNode->children[c];
        if (child) {
            if (child->isCandidate) {
                candidateNode = child;
                j = i;
            }
            currNode = child;
        } else {
            UseCurrentPattern(candidateNode);
            j++;
            i = j;
            c = string[i];
            currNode = root->children[c];
            candidateNode = currNode;
        }
        i++;
    }
    UseCurrentPattern(currNode);
}

void FrequentPatternCompressor::ForwardCoverDeep(const string& string, Trie* trie){
    if (!string.size()) {
        return;
    }
    Node*& currNode = trie->currNode;
    Node* root = trie->root;
    currNode = root;
    const uint8_t* c = (const uint8_t*)&string[0];
    auto end = c + string.size();
    while(c != end) {
        
        auto node = trie->array[*(uint16_t*)c];
        
        if (!node) {
            
            currNode = currNode->children[*c];
            if (++c == end) {
                break;
            }
            UseCurrentPattern(currNode);
            currNode = root;
            continue;
        }
        
        currNode = node;
        if ((c+=2) == end) {
            break;
        }
        
//        child = currNode->children[*c];
//        if (!child) {
//            UseCurrentPattern(currNode);
//            currNode = root;
//            continue;
//        }
//        currNode = child;
//        if (++c == end) {
//            break;
//        }
        
//        child = currNode->children[*c];
//        if (!child) {
//            UseCurrentPattern(currNode);
//            currNode = root;
//            continue;
//        }
//        currNode = child;
//        if (++c == end) {
//            break;
//        }
        
//        child = currNode->children[*c];
//        if (!child) {
//            UseCurrentPattern(currNode);
//            currNode = root;
//            continue;
//        }
//        currNode = child;
//        c++;
        
        Node* child = currNode->children[*c];
        if (child) {
            currNode = child;
            c++;
            auto ic = c;
            for (auto pc : currNode->partial) {
                if (c == end) {
                    auto d = c - ic;
                    if (d == 0) {
                        UseCurrentPattern(currNode);
                    } else
                    {
                        UseCurrentPattern(currNode, d);
                    }
                    return;
                }
                if (pc != *c) {
                    break;
                }
                c++;
            }
            auto d = c - ic;
            if (d == 0) {
                UseCurrentPattern(currNode);
            } else
            {
                UseCurrentPattern(currNode, d);
            }
            currNode = root;
        }
        else {
            UseCurrentPattern(currNode);
            currNode = root;
        }
    }
    if (currNode != root) {
        UseCurrentPattern(currNode);
    }
}

void FrequentPatternCompressor::UseCurrentPattern(Node* node) {
    auto& index = node->index;
    if (__builtin_expect(!index, 0)) {
        index = (int)patterns.size();
        patterns.push_back(node->prefix);
    }
    //trie->IncrementUsage();
    indices[indexEnd++] =index;
    
}

void FrequentPatternCompressor::UseCurrentPattern(Node* node, int i) {
    auto& index = node->indices[i];
    if (!index) {
        index = (int)patterns.size();
        patterns.push_back(node->prefix + string(&node->partial[0], i));
    }
    //trie->IncrementUsage();
    indices[indexEnd++] =index;
    
}