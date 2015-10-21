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
#include <fstream>
#include "FrequentPatternCompressor.hpp"
#include "Utils.hpp"
#include <chrono>

using namespace std;
using namespace std::chrono;

int main(int argc, const char * argv[]) {
    int iterations = 1000;
    //vector<string> strings = random_dates(100000);
    vector<string> strings;
    ifstream myfile("/Users/xiaojianwang/Desktop/dates.txt");
    for( std::string line; getline( myfile, line ); )
    {
        strings.push_back(line);
    }
    myfile.close();
    
    long size = 0;
    for (string& s : strings) {
        size += s.length();
    }
    size *= iterations;
    long compressedSize = 0;
    
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    
    for(int i = 0; i < iterations; i++) {
        auto compressor = FrequentPatternCompressor();
        string compressed = compressor.Compress(strings);
        compressedSize += compressed.size();
    }
    
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    
    auto duration = duration_cast<microseconds>( t2 - t1 ).count();
    
    cout << "Compression ratio: " << 1.0 * compressedSize / size << endl;
    cout << "Size: " << size / 1024 / 1024 << " MB" << endl;
    cout << "Used " << duration / 1000.0 << " ms" << endl;
}
