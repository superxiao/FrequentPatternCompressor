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

char out[10 * 1024 * 1024];
int indixEnd = 0;
int indices[10*1024*1024]; // Dynamic or static no difference
int outEnd = 0;

string FrequentPatternCompressor::Compress(const vector<string>& strings, int sample_size, int support) {
    indixEnd = 0;
    sample_size = min(sample_size, (int)strings.size());
    vector<string> sample(sample_size);
    srand((unsigned)time(NULL));
    //int size = 0;
    //for (const string& s : strings) {
    //    size += s.length();
    //}
    for (size_t i = 0; i < sample_size; i++) {
        sample[i] = strings[rand() % strings.size()]; // TODO optimization: use string pointers
        //sample[i] = strings[i];
    }
    
    Trie* trie = PrefixSpan::GetFrequentPatterns(sample, support);
    
    patterns.reserve(strings.size() + 256);
    
    // Using 32 bit here will cause later copying to be slower. But that
    // should be optimized away with load/store.
    uint64_t uncompressed_size = 0;
    
    for (const string& s : strings) {
        ForwardCover(s, trie);
        uncompressed_size += s.length();
    }
    //if (size > out.capacity()) {
    //    out.reserve(size);
    //}
    
    outEnd = 0;
    memcpy(out, &uncompressed_size, sizeof(uncompressed_size));
    outEnd += sizeof(uncompressed_size);
    
    for(auto &pattern : patterns) {
        uint16_t length = pattern.length();
        memcpy(out + outEnd, reinterpret_cast<char*>(&length), 2);
        outEnd += 2;
        memcpy(out + outEnd, pattern.c_str(), pattern.length());
        outEnd += pattern.length();
    }
    //indices.reserve(charNum);
    out[outEnd++] = 0;
    out[outEnd++] = 0;
    
    AppendPackedLengths(strings);
    int bitsPerIndex = sizeof(unsigned) * 8 - __builtin_clz((unsigned)patterns.size());
    AppendPackedIntegers(indices, indixEnd, bitsPerIndex);
    string result(out, outEnd);
    delete trie;
    return result;
    
}

void FrequentPatternCompressor::AppendPackedLengths(const vector<string>& strings) {
    vector<int> lengths;
    lengths.reserve(strings.size());
    int minLen = (int)strings[0].size();
    int maxLen = minLen;
    for (const string& s : strings) {
        lengths.push_back((int)s.size());
        minLen = min(minLen, (int)s.size());
        maxLen = max(maxLen, (int)s.size());
    }
    for (int& i : lengths) {
        i -= minLen;
    }
    int size = (int)strings.size();
    memcpy(out + outEnd, reinterpret_cast<char*>(&size), 4);
    outEnd += 4;
    memcpy(out + outEnd, reinterpret_cast<char*>(&minLen), 2);
    outEnd += 2;
    int bitsPerLen = 0;
    if (maxLen > minLen) {
        int leading0s = __builtin_clz((unsigned)(maxLen - minLen));
        bitsPerLen = sizeof(unsigned) * 8 - leading0s; // TODO clz doesn't support 0?
    }
    memcpy(out + outEnd, reinterpret_cast<char*>(&bitsPerLen), 2);
    outEnd += 2;
    AppendPackedIntegers(&lengths[0], (int)lengths.size(), bitsPerLen);
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
            UseCurrentPattern(trie);
            currNode = root->children[c];
        }
    }
    UseCurrentPattern(trie);
}

void FrequentPatternCompressor::UseCurrentPattern(Trie* trie) {
    if (trie->GetIndex() == -1) {
        trie->SetIndex((int)patterns.size());
        patterns.push_back(trie->GetString());
    }
    //trie->IncrementUsage();
    indices[indixEnd++] =trie->GetIndex();
    
}

void FrequentPatternCompressor::AppendPackedIntegers(int* integers, int size, int bitsPerInt){
    // resize first using outSize to optmize?
    // int outSize = (indices.size() * bitsPerIndex + 7) / 8;
    if (bitsPerInt == 0) {
        return;
    }
    uint64_t acc = 0;
    int bits = 0;
    int i = 0;
    while (i < size) {
        if (bits) {
            memcpy(out + outEnd, &acc, 4);
            outEnd += 4;
            bits -= 32;
            acc >>= 32;
        }
        while (bits < 32 && i < size) {
            acc |= (uint64_t)integers[i++] << bits;
            bits += bitsPerInt;
        }
    }
    
    for (; bits > 0; bits -= 8) {
        out[outEnd++] = (char)acc;
        acc >>= 8;
    }
}

