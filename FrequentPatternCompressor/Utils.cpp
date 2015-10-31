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

#include "headers/codecfactory.h"

using namespace FastPForLib;

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

void encodeArray(const uint32_t *in, const size_t length, uint32_t *out,
                 size_t &nvalue) {
    string codecName = "simdbinarypacking";
    FastPForLib::IntegerCODEC & codec = * FastPForLib::CODECFactory::getFromName(codecName);
    codec.encodeArray(in, length, out, nvalue);
}

const uint32_t * decodeArray(const uint32_t *in, const size_t length,
                             uint32_t *out, size_t & nvalue) {
    string codecName = "simdbinarypacking";
    FastPForLib::IntegerCODEC & codec = * FastPForLib::CODECFactory::getFromName(codecName);
    return codec.decodeArray(in, length, out, nvalue);
}

