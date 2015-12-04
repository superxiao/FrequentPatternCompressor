//
//  TestFrequentPatternCompressor.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 10/20/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#include <stdio.h>

//#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "Utils.hpp"
#include "FrequentPatternCompressor.hpp"
#include "FrequentPatternDecompressor.hpp"

TEST_CASE( "FrequentPatternCompressor should compress data in a way that "
          "FrequentPatternDecompressor can losslessly reconstruct the original data",
          "[FrequentPatternCompressor]" ) {
    FrequentPatternCompressor compressor;
    auto dates = random_dates(10000);
    auto compressed = compressor.Compress(dates);
    FrequentPatternDecompressor decompressor;
    auto decompressed = decompressor.Decompress(compressed);
    REQUIRE(dates.size() == decompressed.size());
    for(int i = 0; i < dates.size(); i++) {
        REQUIRE(dates[i] == decompressed[i]);
    }

    
    vector<string> strings;
   
    ifstream s("/Users/xiaojianwang/Desktop/alice29r.txt");
    for (string line; getline(s, line);) {
        strings.push_back(line);
    }
    compressed = compressor.Compress(strings);
    decompressed = decompressor.Decompress(compressed);
    REQUIRE(strings.size() == decompressed.size());
    for(int i = 0; i < strings.size(); i++) {
        REQUIRE(strings[i] == decompressed[i]);
    }
}
