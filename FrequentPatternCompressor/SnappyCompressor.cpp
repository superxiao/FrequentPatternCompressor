//
//  SnappyCompressor.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 10/21/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#include "SnappyCompressor.hpp"
#include "snappy/snappy.h"

string SnappyCompressor::Compress(const string& data) {
    string out;
    snappy::Compress(data.c_str(), data.length(), &out);
    return move(out);
}