//
//  FrequentPatternDecompressor.hpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 10/25/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#ifndef FrequentPatternDecompressor_hpp
#define FrequentPatternDecompressor_hpp

#include <stdio.h>
#include <vector>
#include <string>

using namespace std;

class FrequentPatternDecompressor {
public:
    vector<string> Decompress(const string& compressed);
    string Decompress(const string& compressed, vector<int>& outLens);
    
private:
    vector<int> UnpackIntegers(const char*& data);
};

#endif /* FrequentPatternDecompressor_hpp */
