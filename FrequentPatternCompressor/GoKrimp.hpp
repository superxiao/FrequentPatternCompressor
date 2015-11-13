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
    Trie* GetCompressingPatterns(const vector<string>& strings, int minSupport);
private:
    
};

#endif /* GoKrimp_hpp */
