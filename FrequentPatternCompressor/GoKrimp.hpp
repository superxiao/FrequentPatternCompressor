//
//  GoKrimp.hpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 11/13/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#ifndef GoKrimp_hpp
#define GoKrimp_hpp

#include <stdio.h>
#include "Trie.hpp"

class GoKrimp {
public:
    GoKrimp();
    static vector<string> GetCompressingPatterns(const vector<string>& strings, int k, vector<int>& freqs);
    static Trie* GetCompressingPatternsTrie(const vector<string>& strings, int k, vector<int>& freqs);
    static double Compress(const vector<string>& strings, int patternNum = 200);
private:
    
};

#endif /* GoKrimp_hpp */
