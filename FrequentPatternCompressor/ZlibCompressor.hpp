//
//  ZlibCompressor.hpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 5/8/16.
//  Copyright © 2016 Xiaojian Wang. All rights reserved.
//

#ifndef ZlibCompressor_hpp
#define ZlibCompressor_hpp

#include <stdio.h>
#include <string>
#include <vector>

using namespace std;

class ZlibCompressor {
public:
    string Compress(const string& in);
    string Decompress(const string& out);
};

#endif /* ZlibCompressor_hpp */
