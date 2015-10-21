//
//  TestTrie.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 10/20/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#include <stdio.h>

#include "catch.hpp"
#include "Trie.hpp"
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <random>
#include "Utils.hpp"

TEST_CASE( "Trie should support adding strings, and testing the presence"
          " of those strings.", "[Trie]" ) {
    int batchSize = 1000;
    vector<string> strings = random_dates(10);
    for(string& s : strings) {
        //cout << s << endl;
    }
    Trie* trie = new Trie();
    trie->BuildTrie(strings);
    SECTION( "test BuildTrie" ) {
        for (string& string : strings) {
            REQUIRE(trie->Contains(string));
        }
        REQUIRE_FALSE(trie->Contains("123456789"));
    }
    SECTION("test traversal methods") {
        for (string& string : strings) {
            trie->GoToRoot();
            for(char c : string) {
                REQUIRE(trie->GoToChild(c));
            }
        }
        
        string s = "123456789";
        bool contains = true;
        trie->GoToRoot();
        for(char c : s) {
            if (!trie->GoToChild(c)) {
                contains = false;
                break;
            }
        }
        REQUIRE_FALSE(contains);
    }
    delete trie;
}
