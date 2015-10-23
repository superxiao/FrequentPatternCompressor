//
//  SnappyCompressor.hpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 10/21/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#ifndef SnappyCompressor_hpp
#define SnappyCompressor_hpp

#include <stdio.h>
#include <string>
#include <vector>

using namespace std;

class SnappyCompressor {
public:
    string Compress(const string& strings);
};

#endif /* SnappyCompressor_hpp */
