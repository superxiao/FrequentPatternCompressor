//
//  main.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 10/16/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#include <iostream>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>
#include "FrequentPatternCompressor.hpp"
#include "Utils.hpp"

using namespace std;

int main(int argc, const char * argv[]) {
    int iterations = 1000;
    vector<string> strings = random_dates(100000);
    long size = 0;
    for (string& s : strings) {
        size += s.length();
    }
    size *= iterations;
    long compressedSize = 0;
    for(int i = 0; i < iterations; i++) {
        auto compressor = FrequentPatternCompressor();
        string compressed = compressor.Compress(strings);
        compressedSize += compressed.size();
    }
    cout << "Compression ratio: " << 1.0 * compressedSize / size << endl;
    cout << "Size: " << size / 1024 / 1024 << " MB" << endl;
}
