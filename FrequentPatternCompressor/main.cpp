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
#include "SnappyCompressor.hpp"
#include "FrequentPatternDecompressor.hpp"
#include "snappy/snappy.h"

using namespace std;
using namespace std::chrono;

const int LINES_PER_BLOCK = 100000;

const int REPEAT = 50;

string compress_snappy(const string& data) {
    auto compressor = SnappyCompressor();
    string compressed = compressor.Compress(data);
    return move(compressed);
}

string decomrpess_snappy(const string& compressed) {
    string decompressed;
    snappy::Uncompress(compressed.c_str(), compressed.length(), &decompressed);
    return move(decompressed);
}

string decompress_frequent(const string& compressed) {
    auto decompressor = FrequentPatternDecompressor();
    vector<int> lens;
    auto decompressed = decompressor.Decompress(compressed, lens);
    return move(decompressed);
}

string compress_frequent(const vector<string>& strings) {
    auto compressor = FrequentPatternCompressor();
    string compressed = compressor.Compress(strings);
    return move(compressed);
}

void print_stat(long compressedSize, long size, long duration) {
    cout << "Compression ratio: " << 1.0 * compressedSize / size << endl;
    cout << "Original Size: " << size / 1024 / 1024 << " MB" << endl;
    cout << "Compressed Size: " << compressedSize / 1024 / 1024 << "MB" << endl;
    cout << "Used " << duration / 1000.0 << " ms" << endl;
}

void compress_file_snappy(string file) {
    long size = 0;
    long compressedSize = 0;
    long duration = 0;
    for (int j = 0; j < REPEAT; j++) {
        ifstream s(file);
        while (!s.eof()) {
            string data;
            int i = 0;
            for( string line; getline( s, line );)
            {
                auto len = line.length();
                char* lenBytes = reinterpret_cast<char*>(&len);
                data.insert(data.end(), lenBytes, lenBytes + 2);
                data.append(line);
                i++;
                if (i == LINES_PER_BLOCK) {
                    break;
                }
            }
            auto t1 = high_resolution_clock::now();
            string compressed = compress_snappy(data);
            auto t2 = high_resolution_clock::now();
            duration += duration_cast<microseconds>( t2 - t1 ).count();
            compressedSize += compressed.length();
            size += data.size();
        }
        s.close();
    }
    print_stat(compressedSize, size, duration);
}

void compress_file_frequent(string file) {
    vector<string> strings;
    long duration = 0;
    long decompressDuration = 0;
    long size = 0;
    long compressedSize = 0;
    for (int j = 0; j < REPEAT; j++) {
        ifstream s(file);
        while(!s.eof()) {
            int i = 0;
            strings.clear();
            for (string line; getline(s, line);) {
                strings.push_back(line);
                size += line.length() + 2;
                i++;
                if (i == LINES_PER_BLOCK) {
                    break;
                }
            }
            auto t1 = high_resolution_clock::now();
            auto compressed = compress_frequent(strings);
            auto t2 = high_resolution_clock::now();
            compressedSize += compressed.length();
            duration += duration_cast<microseconds>( t2 - t1 ).count();
            
            t1 = high_resolution_clock::now();
            auto decompressed = decompress_frequent(compressed);
            t2 = high_resolution_clock::now();
            decompressDuration += duration_cast<microseconds>( t2 - t1 ).count();
        }
        s.close();
    }
    print_stat(compressedSize, size, duration);
    cout << "Decompress time: " << decompressDuration / 1000.0 << " ms" << endl;
}

void compress_small_file_snappy(string file) {
    string data;
    ifstream s(file);
    long size = 0;
    for( string line; getline( s, line );)
    {
        auto len = line.length();
        char* lenBytes = reinterpret_cast<char*>(&len);
        data.insert(data.end(), lenBytes, lenBytes + 2);
        data.append(line);
    }
    s.close();
    size = data.size();
    size *= REPEAT;
    long compressedSize = 0;
    long duration = 0;
    long decompressDuration = 0;
    for (int j = 0; j < REPEAT; j++) {
        auto t1 = high_resolution_clock::now();
        auto compressed = compress_snappy(data);
        auto t2 = high_resolution_clock::now();
        compressedSize += compressed.length();
        duration += duration_cast<microseconds>( t2 - t1 ).count();
        
        t1 = high_resolution_clock::now();
        auto decompressed = decomrpess_snappy(compressed);
        t2 = high_resolution_clock::now();
        decompressDuration += duration_cast<microseconds>( t2 - t1 ).count();
    }
    print_stat(compressedSize, size, duration);
    cout << "Decompress time: " << decompressDuration / 1000.0 << " ms" << endl;
}

void compress_small_file_frequent(string file) {
    vector<string> strings;
    ifstream s(file);
    long size = 0;
    for(string line; getline(s, line);) {
        strings.push_back(line);
        size += line.length() + 2;
    }
    s.close();
    size *= REPEAT;
    long compressedSize = 0;
    long duration = 0;
    long decompressDuration = 0;
    for (int j = 0; j < REPEAT; j++) {
        auto t1 = high_resolution_clock::now();
        auto compressed = compress_frequent(strings);
        auto t2 = high_resolution_clock::now();
        compressedSize += compressed.length();
        duration += duration_cast<microseconds>( t2 - t1 ).count();
        
        t1 = high_resolution_clock::now();
        auto decompressed = decompress_frequent(compressed);
        t2 = high_resolution_clock::now();
        decompressDuration += duration_cast<microseconds>( t2 - t1 ).count();
    }
    print_stat(compressedSize, size, duration);
    cout << "Decompress time: " << decompressDuration / 1000.0 << " ms" << endl;
}

int main(int argc, const char * argv[]) {
    string file = "/Users/xiaojianwang/Desktop/dates.txt";
    //ifstream myfile("/Users/xiaojianwang/Desktop/uri.txt");
    //ifstream myfile("/Users/xiaojianwang/Desktop/alice29r.txt");
    //string file = "/Users/xiaojianwang/Documents/workspace/benchmarks/customer_address.txt";
    //string file = "/Users/xiaojianwang/Documents/workspace/benchmarks/part_type.txt";
    //string file = "/Users/xiaojianwang/Desktop/dates.txt";
    //compress_file_frequent(file);
    //compress_small_file_frequent(file);
    compress_small_file_snappy(file);
    //compress_file_snappy(file);
}
