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
    static vector<int> indices;
    int maxIndex = 0;
    vector<string> patterns;
    
    // Does inining make a difference?
    inline void ForwardCover(const string& string, Trie* trie);
    inline void UseCurrentPattern(Trie* trie);
    void AppendPackedLengths(const vector<string> strings, string& out);
    void AppendPackedIntegers(const vector<int> integers, string& out, int bitsPerInt);

public:
    int charNum;
    long forwardCoverTime;
    string Compress(vector<string> strings);
};
#endif /* FrequentPatternCompressor_hpp */
