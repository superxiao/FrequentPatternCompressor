//
//  TestFrequentPatternCompressor.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 10/20/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#include <stdio.h>
#include <iostream>
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
    
    string indir = "/Users/xiaojianwang/Documents/workspace/benchmarks/gen/";
    vector<string> infiles = {
        //        "gen-iso8601",
        //        "gen-uri",
        //        "gen-email",
        //        "gen-user_agent",
        //        "gen-credit_card_number",
        //        "gen-credit_card_full",
        //        "gen-sha1",
        "gen-text",
        //        "gen-phone_number",
        //        "gen-address",
        //        "gen-name",
        
        
    };
    for (string file : infiles) {
        ifstream s(indir + file + ".txt");
        long uncompressedSize = 0;
        for (string line; strings.size() < 100000 && getline(s, line);) {
            strings.push_back(line);
            uncompressedSize += 2 + line.size();
        }
        compressed = compressor.Compress(strings);
        cout << "Compression ratio: " << compressed.size() * 1.0 / uncompressedSize << endl;
        decompressed = decompressor.Decompress(compressed);
        REQUIRE(strings.size() == decompressed.size());
        for(int i = 0; i < strings.size(); i++) {
            REQUIRE(strings[i] == decompressed[i]);
        }
        strings.clear();
    }
}
