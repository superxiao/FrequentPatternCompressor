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

string FrequentPatternCompressor::Compress(vector<string> strings) {
    indices.clear();
    int sample_size = 100;
    vector<string> sample(sample_size);
    srand((unsigned)time(NULL));
    for (size_t i; i < strings.size(); i++) {
        sample[i] = strings[rand() % strings.size()]; // TODO optimization: use string pointers
    }
    
    Trie* trie = PrefixSpan::GetFrequentPatterns(sample, 5);
    
    patterns.reserve(strings.size() + 256);
    
    for (auto itr = strings.begin(); itr != strings.end(); itr++) {
        charNum += strings.size();
        ForwardCover(*itr, trie);
    }
    
    string out;
    for(auto &pattern : patterns) {
        uint16_t length = pattern.length();
        out.append(reinterpret_cast<char*>(&length), 2);
        out.append(pattern);
    }
    out.append(2, 0);
    
    AppendPackedLengths(strings, out);
    
    int bitsPerIndex = sizeof(unsigned) - __clz((unsigned)patterns.size());
    out.append(reinterpret_cast<char*>(&bitsPerIndex), 2);
    AppendPackedIntegers(this->indices, out, bitsPerIndex);
    
    return out;
    
}

void FrequentPatternCompressor::AppendPackedLengths(const vector<string> strings, string& out) {
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
    out.append(reinterpret_cast<char*>(&size), 4);
    out.append(reinterpret_cast<char*>(&minLen), 2);
    int bitsPerLen = sizeof(unsigned) - __clz((unsigned)maxLen);
    out.append(reinterpret_cast<char*>(&bitsPerLen), 2);
    AppendPackedIntegers(lengths, out, bitsPerLen);
}

void FrequentPatternCompressor::ForwardCover(const string& string, Trie* trie){
    trie->GoToRoot();
    for(char c : string) {
        if(!trie->GoToChild(c)) {
            UseCurrentPattern(trie);
            trie->GoToChildOfRoot(c);
        }
    }
    UseCurrentPattern(trie);
}

void FrequentPatternCompressor::UseCurrentPattern(Trie* trie){
    if (!trie->GetUsage()) {
        trie->SetIndex((int)patterns.size());
        patterns.push_back(trie->GetString());
    }
    trie->IncrementUsage();
    indices.push_back(trie->GetIndex());
}

void FrequentPatternCompressor::AppendPackedIntegers(const vector<int> integers, string& out, int bitsPerInt){
    // resize first using outSize to optmize?
    // int outSize = (indices.size() * bitsPerIndex + 7) / 8;
    unsigned acc = 0;
    unsigned bits = 0;
    for (int i : integers) {
        for (; bits > 7; bits -= 8) {
            out.push_back((char)acc);
            acc >>= 8;
        }
        acc |= i << bits;
        bits += bitsPerInt;
    }
    
    for (; bits > 0; bits -= 8) {
        out.push_back((char)acc);
        acc >>= 8;
    }
}

