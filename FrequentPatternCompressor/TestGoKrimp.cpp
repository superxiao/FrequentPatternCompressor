//
//  TestGoKrimp.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 11/23/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#include "catch.hpp"
#include <string>
#include <unordered_set>
#include <iostream>
#include "GoKrimp.hpp"
#include "PrefixSpan.hpp"
#include "Utils.hpp"
#include "FrequentPatternCompressor.hpp"

using namespace std::chrono;
using namespace std;

TEST_CASE( "GoKrimp should find the most compressing patterns."
          , "[GoKrimp]" ) {
    GoKrimp goKrimp;
    vector<string> strings;
    strings.push_back("1234");
    strings.push_back("1235");
    strings.push_back("2345");
    strings.push_back("4235");
    strings.push_back("023465");
    vector<int> freqs;
    auto patterns = goKrimp.GetCompressingPatterns(strings, 5, freqs);
    sort(patterns.begin(), patterns.end());
    REQUIRE((patterns == vector<string> {"1", "23", "234", "5"}));
}

//TEST_CASE( "GoKrimp test 2"
//          , "[GoKrimp]" ) {
//    GoKrimp goKrimp;
//    PrefixSpan prefixSpan;
//    auto dates = random_dates(100);
//    long duration1 = 0;
//    long duration2 = 0;
//    for (int i = 0; i < 100; i++) {
//        auto t1 = high_resolution_clock::now();
//        vector<int> freqs;
//        auto goPatterns = goKrimp.GetCompressingPatternsTrie(dates, 20, freqs);
//        auto t2 = high_resolution_clock::now();
//        auto psPatterns = prefixSpan.GetFrequentPatterns(dates, 5);
//        auto t3 = high_resolution_clock::now();
//        duration1 = duration_cast<microseconds>(t2 - t1).count();
//        duration2 = duration_cast<microseconds>(t3 - t2).count();
//    }
//    cout << "Time for GoKrimp:" << duration1 / 1000.0 << endl;
//    cout << "Time for PrefixSpan:" << duration2 / 1000.0 << endl;
//    //sort(patterns.begin(), patterns.end());
//    //REQUIRE((patterns == vector<string> {"1", "23", "234", "5"}));
//}

TEST_CASE( "GoKrimp compression test"
          , "[GoKrimp]" ) {
    GoKrimp goKrimp;
    FrequentPatternCompressor compressor;
    auto dates = random_dates(10000);
    long duration1 = 0;
    long duration2 = 0;
    
    long len = 0;
    for (auto& date : dates) {
        len += date.length();
    }
    
    double compressedLen = 0;
    double frequentCompressedLen = 0;
    for (int i = 0; i < 10; i++) {
        auto t1 = high_resolution_clock::now();
        double compressed = GoKrimp::Compress(dates);
        auto t2 = high_resolution_clock::now();
        double frequentCompressed = compressor.Compress(dates, 100, 5).size();
        auto t3 = high_resolution_clock::now();
        duration1 += duration_cast<microseconds>(t2 - t1).count();
        duration2 += duration_cast<microseconds>(t3 - t2).count();
        compressedLen += compressed;
        frequentCompressedLen += frequentCompressed;
    }
    len *= 10;
    
    cout << "Time for GoKrimp:" << duration1 / 1000.0 << endl;
    cout << "Time for PrefixSpan:" << duration2 / 1000.0 << endl;
    
    cout << "Compression ratio for GoKrimp:" << compressedLen / len << endl;
    cout << "Compression ratio for Frequent:" << frequentCompressedLen / len << endl;
    //sort(patterns.begin(), patterns.end());
    //REQUIRE((patterns == vector<string> {"1", "23", "234", "5"}));
}
