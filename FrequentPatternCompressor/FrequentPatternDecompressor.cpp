//
//  FrequentPatternDecompressor.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 10/25/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#include "FrequentPatternDecompressor.hpp"
#include <iostream>
#include "Utils.hpp"

uint32_t integers[100 * 1024 * 1024];


string FrequentPatternDecompressor::Decompress(const string& compressed, vector<uint32_t>& outLens) {
    
    const uint8_t* begin = reinterpret_cast<const uint8_t*>(compressed.c_str());
    const uint8_t* data = begin;
    
    uint64_t uncompressed_size = *reinterpret_cast<const uint64_t*>(data);
    data += sizeof(uncompressed_size);
    
    uint32_t numStrings = *reinterpret_cast<const uint32_t*>(data);
    data += sizeof(numStrings);
    vector<const uint8_t*> dict;
    vector<int> lens;
    if (outLens.size() < numStrings) {
        outLens.resize(numStrings);
    }
    while (true) {
        int patternLen = (int)*reinterpret_cast<const uint16_t*>(data);
        data += 2;
        if (patternLen == 0) {
            break;
        }
        dict.push_back(data);
        lens.push_back(patternLen);
        data += patternLen;
    }
    
    uint32_t lenSize = *reinterpret_cast<const uint32_t*>(data);
    
    data += 4;
    
    if ((data - begin) % 16 != 0){
        data += 16 - (data - begin) % 16;
    }
    
    size_t recoveredsize = uncompressed_size;
    
    decodeArray(reinterpret_cast<const uint32_t*>(data),
                lenSize, reinterpret_cast<uint32_t*>(outLens.data()), recoveredsize);

    string result;
    result.resize(uncompressed_size);
    
    data += lenSize * 4;
    
    recoveredsize = uncompressed_size;
    
    decodeArray(reinterpret_cast<const uint32_t*>(data),
                      (begin + compressed.size() - data) / 4, integers, recoveredsize);
    char* op = &*result.begin();
    auto end = integers + recoveredsize;
    for(auto p = integers; p != end; p++) {
        const uint8_t* pattern_ip = dict[*p];
        int len = (int)lens[*p];
        auto curr_op = op;
        while (true) {
            UNALIGNED_STORE64(curr_op, UNALIGNED_LOAD64(pattern_ip));
            //memcpy(op, &*dict[buffer & mask].begin(), 8);
            len -= 8;
            if (__builtin_expect(len <= 0, 1)) {
                break;
            }
            pattern_ip += 8;
            curr_op += 8;
            
        }
        op += lens[*p];
    }
    
    result.resize(op - &*result.begin());

    return move(result);
}

vector<string> FrequentPatternDecompressor::Decompress(const string& compressed) {
    vector<uint32_t> lens;
    string decompressed = Decompress(compressed, lens);
    vector<string> result;
    auto curr = decompressed.begin();
    for (auto len : lens) {
        result.emplace_back(curr, curr + len);
        curr += len;
    }
    return move(result);
}