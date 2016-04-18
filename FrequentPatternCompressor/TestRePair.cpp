//
//  TestRePair.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 4/16/16.
//  Copyright © 2016 Xiaojian Wang. All rights reserved.
//

#include <stdio.h>

#include "catch.hpp"
#include "RePair.hpp"
#include "Utils.hpp"

TEST_CASE( "RePair should give a list of compressing patterns"
          " from a list of input strings", "[RePair]" ) {
    RePair repair;
    auto dates = random_dates(100);
    repair.getPhrases(dates);
    vector<string> strings;
    strings.push_back("1234");
    strings.push_back("1235");
    strings.push_back("2345");
    strings.push_back("4235");
    strings.push_back("023465");
    RePair repair2;
    repair2.getPhrases(strings);
}
