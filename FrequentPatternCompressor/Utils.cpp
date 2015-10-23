//
//  Utils.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 10/20/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#include "Utils.hpp"
#include <random>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <cstring>

vector<string> random_dates(int n) {
    vector<string> strings;
    random_device rd;
    mt19937 mt(rd());
    uniform_int_distribution<> dist(INT32_MIN, INT32_MAX);
    stringstream stream;
    tm tm;
    while(strings.size() < n) {
        stream.str("");
        time_t t = time(NULL) + dist(mt);
        tm = *std::localtime(&t);
        stream << put_time(&tm, "%c %Z");
        strings.push_back(stream.str());
    }
    return strings;
}
