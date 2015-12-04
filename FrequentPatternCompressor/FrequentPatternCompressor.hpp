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
#include "art.h"

class FrequentPatternCompressor {
private:
    int maxIndex = 0;
    vector<string> patterns;
    
    // Does inining make a difference?
    inline void ForwardCover(const string& string, Trie* trie);
    inline void ForwardCoverWithART(const string& string, art_tree* trie);
    inline void UseCurrentPattern(Node* node);
    
    void AppendPackedLengths(const vector<string>& strings);
    void AppendPackedIntegers(int* integers, int size, int bitsPerInt);

public:
    int charNum;
    long forwardCoverTime;
    string Compress(const vector<string>& strings, int sample_size = 100, int support = 5, bool use_gokrimp = false);
    string CompressWithART(const vector<string>& strings, int sample_size = 100, int support = 5, bool use_gokrimp = false);
};
#endif /* FrequentPatternCompressor_hpp */
