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
#include "GoKrimp.hpp"

using namespace std::chrono;

char out[10 * 1024 * 1024];
int indexEnd = 0;
int32_t indices[50*1024*1024]; // Dynamic or static no difference
int outEnd = 0;

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
                cout << "Frequent pattern in trie 1 but not trie 2:\n" << c1->str<<endl;
                cout << "Frequent pattern in trie 2 but not trie 1:\n" << c2->str<<endl;
            }
        }
    }
}

string FrequentPatternCompressor::Compress(const vector<string>& strings, int sample_size, int support,
                                           bool use_gokrimp) {
    indexEnd = 0;
    sample_size = min(sample_size, (int)strings.size());
    vector<string> sample(sample_size);
    srand((unsigned)time(NULL));

    for (size_t i = 0; i < sample_size; i++) {
        sample[i] = strings[rand() % strings.size()]; // TODO optimization: use string pointers
        //sample[i] = strings[i];
    }
    Trie* trie = nullptr;
    if(use_gokrimp) {
        vector<int> freqs;
        trie = GoKrimp::GetCompressingPatternsTrie(sample, 200, freqs);
    } else {
        trie = PrefixSpan::GetFrequentPatterns(sample, support);
    }
    patterns.clear();
    patterns.reserve(strings.size() + 256);
    
    // Using 32 bit here will cause later copying to be slower. But that
    // should be optimized away with load/store.
    uint64_t uncompressed_size = 0;
    
    for (const string& s : strings) {
        ForwardCover(s, trie);
        uncompressed_size += s.length();
    }
    
    outEnd = 0;
    memcpy(out, &uncompressed_size, sizeof(uncompressed_size));
    outEnd += sizeof(uncompressed_size);
    
    uint32_t numStrings = (uint32_t)strings.size();
    memcpy(out + outEnd, &numStrings, sizeof(numStrings));
    outEnd += sizeof(numStrings);
    
    for(auto &pattern : patterns) {
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

void FrequentPatternCompressor::ForwardCover(const string& string, Trie* trie){
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

void FrequentPatternCompressor::UseCurrentPattern(Node* node) {
    auto& index = node->index;
    if (index == -1) {
        index = (int)patterns.size();
        patterns.push_back(node->str);
    }
    //trie->IncrementUsage();
    indices[indexEnd++] =index;
    
}