//
//  FrequentPatternDecompressor.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 10/25/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#include "FrequentPatternDecompressor.hpp"

inline void UNALIGNED_STORE64(void *p, uint64_t v) {
    memcpy(p, &v, sizeof v);
}

inline uint64_t UNALIGNED_LOAD64(const void *p) {
    uint64_t t;
    memcpy(&t, p, sizeof t);
    return t;
}

string FrequentPatternDecompressor::Decompress(const string& compressed, vector<int>& outLens) {
    const uint8_t* begin = reinterpret_cast<const uint8_t*>(compressed.c_str());
    const uint8_t* data = begin;
    
    uint64_t uncompressed_size = *reinterpret_cast<const uint64_t*>(data);
    data += sizeof(uncompressed_size);
    
    vector<string> dict;
    while (true) {
        int patternLen = (int)*reinterpret_cast<const uint16_t*>(data);
        data += 2;
        if (patternLen == 0) {
            break;
        }
        dict.emplace_back(data, data + patternLen);
        data += patternLen;
    }
    
    int intNum = (int)*reinterpret_cast<const uint32_t*>(data);
    data += 4;
    int minInt = (int)*reinterpret_cast<const uint16_t*>(data);
    data += 2;
    int bitsPerInt = (int)*reinterpret_cast<const uint16_t*>(data);
    data += 2;
    uint64_t buffer = 0;
    int bits = 0;
    unsigned long mask = ~(-1 << bitsPerInt);
    for (int i = 0; i < intNum; i++) {
        for (; bits < bitsPerInt; bits += 8) {
            buffer |= ((*data) << bits);
            data++;
        }
        outLens.push_back((int)(buffer & mask) + minInt);
        buffer >>= bitsPerInt;
        bits -= bitsPerInt;
    }
    
    int leading0s = __builtin_clz((unsigned)dict.size());
    int unsignedLen = sizeof(unsigned) * 8;
    int bitsPerIndex = unsignedLen - leading0s;
    buffer = 0;
    bits = 0;
    mask = ~(-1 << bitsPerIndex);
    string result;
    result.resize(uncompressed_size);
    char* op = &*result.begin();
    auto size = compressed.size();
    auto end = data + ((begin + size - data) / 4) * 4;
    while (data != end) {
        for (; bits < bitsPerIndex; bits+=32) {
            buffer |= ((uint64_t)(*reinterpret_cast<const uint32_t*>(data)) << bits);
            data += 4;
        }
        string* pattern = &dict[buffer & mask];
        auto ip = &*pattern->begin();
        int len = (int)pattern->length();
        auto curr_op = op;
        while (true) {
            UNALIGNED_STORE64(curr_op, UNALIGNED_LOAD64(ip));
            //memcpy(op, &*dict[buffer & mask].begin(), 8);
            len -= 8;
            if (len <= 0) {
                break;
            }
            ip += 8;
            curr_op += 8;

        }
        op += pattern->length();
        buffer >>= bitsPerIndex;
        bits -= bitsPerIndex;
        
    }
    
    buffer |= ((uint64_t)(*reinterpret_cast<const uint32_t*>(data)) << bits);
    bits += (begin + size - end) * 8;
    
    while(bits >= bitsPerIndex) {
        string* pattern = &dict[buffer & mask];
        auto ip = &*pattern->begin();
        int len = (int)pattern->length();
        auto curr_op = op;
        while (len > 0) {
            UNALIGNED_STORE64(curr_op, UNALIGNED_LOAD64(ip));
            //memcpy(op, &*dict[buffer & mask].begin(), 8);
            len -= 8;
            ip += 8;
            curr_op += 8;
        }
        op += pattern->length();
        buffer >>= bitsPerIndex;
        bits -= bitsPerIndex;
    }
    
    result.resize(op - &*result.begin());
    return move(result);
}

vector<string> FrequentPatternDecompressor::Decompress(const string& compressed) {
    vector<int> lens;
    string decompressed = Decompress(compressed, lens);
    vector<string> result;
    auto curr = decompressed.begin();
    for (auto len : lens) {
        result.emplace_back(curr, curr + len);
        curr += len;
    }
    return move(result);
}

vector<int> FrequentPatternDecompressor::UnpackIntegers(const char*& data) {
    int intNum = (int)*reinterpret_cast<const uint32_t*>(data);
    data += 4;
    int minInt = (int)*reinterpret_cast<const uint16_t*>(data);
    data += 2;
    int bitsPerInt = (int)*reinterpret_cast<const uint16_t*>(data);
    data += 2;
    vector<int> integers;
    unsigned long buffer = 0;
    int bits = 0;
    unsigned long mask = ~(-1 << bitsPerInt);
    for (int i = 0; i < intNum; i++) {
        for (; bits < bitsPerInt; bits += 8) {
            buffer &= ((*data) << bits);
            data++;
        }
        integers.push_back((int)(buffer & mask) + minInt);
        buffer >>= bitsPerInt;
        bits -= bitsPerInt;
    }
    return integers;
}
