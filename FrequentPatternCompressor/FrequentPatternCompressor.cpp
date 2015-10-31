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

using namespace std::chrono;

char out[10 * 1024 * 1024];
int indexEnd = 0;
int32_t indices[10*1024*1024]; // Dynamic or static no difference
int outEnd = 0;

string FrequentPatternCompressor::Compress(const vector<string>& strings, int sample_size, int support) {
    indexEnd = 0;
    sample_size = min(sample_size, (int)strings.size());
    vector<string> sample(sample_size);
    srand((unsigned)time(NULL));

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
    
    outEnd = 0;
    memcpy(out, &uncompressed_size, sizeof(uncompressed_size));
    outEnd += sizeof(uncompressed_size);
    
    uint32_t numStrings = (uint32_t)strings.size();
    memcpy(out + outEnd, &numStrings, sizeof(uint32_t));
    outEnd += sizeof(uint32_t);
    
    for(auto &pattern : patterns) {
        uint16_t length = pattern.length();
        memcpy(out + outEnd, reinterpret_cast<char*>(&length), 2);
        outEnd += 2;
        memcpy(out + outEnd, pattern.c_str(), pattern.length());
        outEnd += pattern.length();
    }

    out[outEnd++] = 0;
    out[outEnd++] = 0;
    
    uint32_t* lens = new uint32_t[strings.size()];
    
    size_t compressedSize = 2 * strings.size();
    
    int i = 0;
    for(auto& str : strings) {
        lens[i] = (uint32_t)str.size();
    }
    
    auto lenSizeField = reinterpret_cast<uint32_t*>(out + outEnd);
    
    outEnd += 4;
    
    if (outEnd % 16 != 0){
        outEnd += 16 - outEnd % 16;
    }
    
    encodeArray(lens,
                strings.size(),
                reinterpret_cast<uint32_t*>(out + outEnd),
                compressedSize);
    
    delete[] lens;
    
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
    indices[indexEnd++] =trie->GetIndex();
    
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

