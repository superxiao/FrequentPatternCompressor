//
//  FrequentPatternCompressor.hpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 10/19/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#ifndef FrequentPatternCompressor_hpp
#define FrequentPatternCompressor_hpp

#include <stdio.h>
#include "PrefixSpan.hpp"

class FrequentPatternCompressor {
private:
    int maxIndex = 0;
    vector<string> patterns;
    
    // Does inining make a difference?
    inline void ForwardCover(const string& string, Trie* trie);
    
    inline void UseCurrentPattern(Trie* trie);
    
    void AppendPackedLengths(const vector<string>& strings);
    void AppendPackedIntegers(int* integers, int size, int bitsPerInt);

public:
    int charNum;
    long forwardCoverTime;
    string Compress(const vector<string>& strings);
};
#endif /* FrequentPatternCompressor_hpp */
