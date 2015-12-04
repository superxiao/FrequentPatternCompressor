//
//  TestLibART.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 12/3/15.
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
#include "libart/art.h"

using namespace std::chrono;
using namespace std;

TEST_CASE( "Some test with libart"
          , "[LIBART]" ) {
    vector<string> strings;
    strings.push_back("1234");
    strings.push_back("1235");
    strings.push_back("2345");
    strings.push_back("4235");
    strings.push_back("023465");
    
    art_tree t;
    int res = art_tree_init(&t);
    
    int len;
    uintptr_t line = 1;
    
    for (auto& str : strings){
        len = str.length();
        art_insert(&t, (unsigned char*)&str[0], len, (void*)line);
        line++;
    }
    int i1;
    // Search for each line
    for (auto& str : strings) {
        len = str.length();
        
        int match_len = art_match_len(&t, (unsigned char*)&str[0], len, &i1);
        REQUIRE(match_len == len);
    }
    
    string str = "167";
    int match_len = art_match_len(&t, (unsigned char*)&str[0], str.length(), &i1);
    REQUIRE(match_len == 1);
    
    str = "2367";
    match_len = art_match_len(&t, (unsigned char*)&str[0], str.length(), &i1);
    REQUIRE(match_len == 2);
    
    str = "023469";
    match_len = art_match_len(&t, (unsigned char*)&str[0], str.length(), &i1);
    REQUIRE(match_len == 5);
    
    res = art_tree_destroy(&t);
}



