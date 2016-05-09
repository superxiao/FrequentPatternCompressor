//
//  TestPrefixSpan.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 10/20/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#include <stdio.h>

#include "catch.hpp"
#include "PrefixSpan.hpp"

TEST_CASE( "PrefixSpan should be able to find just the frequent"
          " patterns in input, and nothing more", "[PrefixSpan]" ) {
    PrefixSpan prefixSpan;
    vector<string> strings;
    strings.push_back("1234");
    strings.push_back("1235");
    strings.push_back("2345");
    strings.push_back("4235");
    strings.push_back("023465");
    vector<const string*> string_ptrs;
    for(auto& str : strings) {
        string_ptrs.push_back(&str);
    }
    Trie* trie = prefixSpan.GetFrequentPatterns(string_ptrs, 2, false);
    REQUIRE(trie->Contains("123"));
    REQUIRE(trie->Contains("234"));
    REQUIRE(trie->Contains("235"));
    REQUIRE(trie->Contains("35"));
    REQUIRE(trie->Contains("23"));
    REQUIRE(!trie->Contains("345"));
    REQUIRE(!trie->Contains("1234"));
}
