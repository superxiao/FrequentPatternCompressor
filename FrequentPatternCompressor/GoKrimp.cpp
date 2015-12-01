//
//  GoKrimp.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 11/13/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#include "GoKrimp.hpp"
#include "PatternGrowthCommons.hpp"
#include <algorithm>
#include <climits>
#include <unordered_map>

GoKrimp::GoKrimp() {
    
}

vector<string> buildMarkArrays(const vector<string>& strings) {
    vector<string> arrays;
    for (auto& str : strings) {
        arrays.emplace_back(str.length(), 0);
    }
    return arrays;
}

vector<vector<Position>> countChars(const vector<string>& strings, vector<string>& markArrays) {
    
    vector<vector<Position>> gkprojected;
    gkprojected.resize(256);
    for (auto& vec : gkprojected) {
        vec.clear();
    }
    
    for (uint32_t i = 0; i < strings.size(); i++) {
        for (uint32_t j = 0; j < strings[i].size(); j++) {
            if (markArrays[i][j] == 1) {
                continue;
            }
            uint8_t c = strings[i][j];
            gkprojected[c].push_back(Position {i, j});
        }
    }
    return gkprojected;
}

vector<vector<Position>> countProjected(const vector<string>& strings, const vector<Position>& positions,
                                 int prefixLen, vector<string> markArrays) {
    
    vector<vector<Position>> gkprojected;
    gkprojected.resize(256);
    for (auto& vec : gkprojected) {
        vec.clear();
    }
    for (auto& position : positions) {
        auto& str = strings[position.stringIndex];
        if (str.length() <= position.positionInString + prefixLen) {
            continue;
        }
        if (markArrays[position.stringIndex][position.positionInString + prefixLen] == 1) {
            continue;
        }
        uint8_t c = str[position.positionInString + prefixLen];
        gkprojected[c].push_back(position);
    }
    return gkprojected;
}

void mark(vector<Position>& positions, int len, vector<string>& markArrays) {
    for (auto position : positions) {
        for (int i = position.positionInString; i < position.positionInString + len; i++) {
            markArrays[position.stringIndex][i] = 1;
        }
    }
}

string GetNextCompressingPattern(const vector<string>& strings, vector<string>& markArrays, int& freq) {
    vector<vector<Position>> prevPostions;
    prevPostions.resize(256);
    int prevMaxChildChar = -1;
    auto currPositions = countChars(strings, markArrays);
    int prefixLen = 1;
    uint8_t maxChildChar = -1;
    int maxChildCharPositions = 0;
    string pattern;
    int compressionBenefit = INT_MIN;
    while (true) {
        prevMaxChildChar = maxChildChar;
        maxChildChar = -1;
        maxChildCharPositions = 0;
        for (int i = 0; i < 256; i++) {
            if (currPositions[i].size() > maxChildCharPositions) {
                maxChildCharPositions = currPositions[i].size();
                maxChildChar = i;
            }
        }
        double currCompressionBenefit = maxChildCharPositions * (prefixLen - 1) - prefixLen - 2;
        if (maxChildCharPositions < 2) {
            break;
        }
        if (currCompressionBenefit > compressionBenefit) {
            compressionBenefit = currCompressionBenefit;
        }
        else {
            break;
        }
        prevPostions = move(currPositions);
        pattern += (char)maxChildChar;
        currPositions = countProjected(strings, prevPostions[maxChildChar], prefixLen, markArrays);
        prefixLen++;
    }
    if (pattern == "") {
        return pattern;
    }
    mark(prevPostions[prevMaxChildChar], prefixLen - 1, markArrays);
    freq = prevPostions[prevMaxChildChar].size();
    return pattern;
}

void AddSingletons(const vector<string>& strings, const vector<string>& markArrays, vector<string>& patterns, vector<int>& freqs) {
    unordered_map<char, int> singletonFreqs;
    for (int i = 0; i < markArrays.size(); i++) {
        for (int j = 0; j < markArrays[i].size(); j++) {
            if (markArrays[i][j] == 0) {
                auto got = singletonFreqs.find(strings[i][j]);
                if (got == singletonFreqs.end()) {
                    singletonFreqs[strings[i][j]] = 1;
                }
                else {
                    singletonFreqs[strings[i][j]]++;
                }
            }
        }
    }
    for (auto it = singletonFreqs.begin(); it != singletonFreqs.end(); it++) {
        string c = string(1, it->first);
        patterns.push_back(c);
        freqs.push_back(it->second);
    }
}

vector<string> GoKrimp::GetCompressingPatterns(const vector<string>& strings, int k, vector<int>& freqs) {
    auto markArrays = buildMarkArrays(strings);
    vector<string> patterns;
    for(int i = 0; i < k; i++) {
        int freq;
        string pattern = GetNextCompressingPattern(strings, markArrays, freq);
        if (pattern == "") {
            break;
        }
        patterns.push_back(pattern);
        freqs.push_back(freq);
    }
    AddSingletons(strings, markArrays, patterns, freqs);
    return patterns;
}

Trie* GoKrimp::GetCompressingPatternsTrie(const vector<string> &strings, int k, vector<int>& freqs){
    Trie* trie = new Trie();
    trie->BuildTrie(GetCompressingPatterns(strings, k, freqs));
    return trie;
}

double GoKrimp::Compress(const vector<string> &strings) {
    vector<int> freqs;
    auto patterns = GetCompressingPatterns(strings, 200, freqs);

    double compressedSize = 0;
   
    int bitsPerIndex = sizeof(unsigned) * 8 - __clz((unsigned)patterns.size());

    for (int i = 0; i < patterns.size(); i++) {
        compressedSize += freqs[i] * bitsPerIndex / 8.0;
        compressedSize += patterns[i].size() + 2;
    }
    return compressedSize;
}



