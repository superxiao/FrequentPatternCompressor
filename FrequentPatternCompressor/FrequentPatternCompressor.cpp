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
#include "PrefixSpan.hpp"

vector<int> FrequentPatternCompressor::indices;
char* FrequentPatternCompressor::out = new char[10 * 1024 * 1024];
int FrequentPatternCompressor::outEnd = 0;

string FrequentPatternCompressor::Compress(const vector<string>& strings) {
    indices.clear();
    indices.reserve(strings.size());
    int sample_size = 100;
    vector<string> sample(sample_size);
    srand((unsigned)time(NULL));
    //int size = 0;
    //for (const string& s : strings) {
    //    size += s.length();
    //}
    for (size_t i = 0; i < sample_size; i++) {
        //sample[i] = strings[rand() % strings.size()]; // TODO optimization: use string pointers
        sample[i] = strings[i];
    }
    
    Trie* trie = PrefixSpan::GetFrequentPatterns(sample, 5);
    
    patterns.reserve(strings.size() + 256);
    
    for (const string& s : strings) {
        ForwardCover(s, trie);
    }
    //if (size > out.capacity()) {
    //    out.reserve(size);
    //}
    outEnd = 0;
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
    
    int bitsPerIndex = sizeof(unsigned) * 8 - __clz((unsigned)patterns.size());
    memcpy(out + outEnd, reinterpret_cast<char*>(&bitsPerIndex), 2);
    outEnd += 2;
    AppendPackedIntegers(this->indices, bitsPerIndex);
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
    int leading0s = __clz((unsigned)(maxLen - minLen));
    int bitsPerLen = sizeof(unsigned) * 8 - leading0s; // TODO clz doesn't support 0?
    memcpy(out + outEnd, reinterpret_cast<char*>(&bitsPerLen), 2);
    outEnd += 2;
    AppendPackedIntegers(lengths, bitsPerLen);
}

void FrequentPatternCompressor::ForwardCover(const string& string, Trie* trie){
    trie->GoToRoot();
    Node*& currNode = trie->currNode;
    Node* root = trie->root;
    for(char c : string) {
        Node* child = currNode->children[c];
        if(!child) {
            UseCurrentPattern(trie);
            currNode = root->children[c];
        }
        else {
            currNode = child;
        }
    }
    UseCurrentPattern(trie);
}

void FrequentPatternCompressor::AppendPackedIntegers(const vector<int>& integers, int bitsPerInt){
    // resize first using outSize to optmize?
    // int outSize = (indices.size() * bitsPerIndex + 7) / 8;
    unsigned long acc = 0;
    int bits = 0;
    int i = 0;
    while (i < integers.size()) {
        if (bits) {
            *reinterpret_cast<uint32_t*>(out + outEnd) = (uint32_t)acc;
            outEnd += 4;
            bits -= 32;
            acc >>= 32;
        }
        while (bits < 32 && i < integers.size()) {
            acc |= integers[i++] << bits;
            bits += bitsPerInt;
        }
    }
    
    for (; bits > 0; bits -= 8) {
        out[outEnd++] = (char)acc;
        acc >>= 8;
    }
}

