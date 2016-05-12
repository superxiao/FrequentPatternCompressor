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
#include "ZlibCompressor.hpp"

using namespace std;
using namespace std::chrono;

static const int LINES_PER_BLOCK = 100000;
static const bool PRINT_STATS = true;
static const int REPEAT = 1;
static const int BLOCK_SIZE = 4 * 1024 * 1024;
static const unsigned long OVERALL_SIZE = 5L * 1024 * 1024 * 1024;

struct cstat {
    unsigned long original_size;
    unsigned long compressed_size;
    double compression_ratio;
    double compression_time;
    double decompression_time;
    double pattern_num;
    double sample_size;
};

enum UseCompressor{use_snappy, use_gzip};

void append_stat(string file, cstat s) {
    fstream stream(file, fstream::app);
    stream << s.original_size << " " << s.compressed_size << " "
    << s.compression_ratio << " " << s.compression_time << " "
    << s.decompression_time << " " << s.pattern_num << " " << s.sample_size << endl;
    stream.close();
}

inline string compress_snappy(const string& data) {
    auto compressor = SnappyCompressor();
    string compressed = compressor.Compress(data);
    return move(compressed);
}

inline string decomrpess_snappy(const string& compressed) {
    string decompressed;
    snappy::Uncompress(compressed.c_str(), compressed.length(), &decompressed);
    return move(decompressed);
}

inline string compress_gzip(const string& data) {
    auto compressor = ZlibCompressor();
    string compressed = compressor.Compress(data);
    return move(compressed);
}

inline string decompress_gzip(const string& compressed) {
    auto compressor = ZlibCompressor();
    return move(compressor.Decompress(compressed));
}

inline string decompress_frequent(const string& compressed) {
    auto decompressor = FrequentPatternDecompressor();
    vector<uint32_t> lens;
    auto decompressed = decompressor.Decompress(compressed, lens);
    return move(decompressed);
}

inline string compress_frequent(const vector<string>& strings, int* pattern_num = NULL, int* sample_size = NULL, double sample_rate = 0.001, double rel_support = 0.05, bool prune = false) {
    auto compressor = FrequentPatternCompressor();
    string compressed = compressor.Compress(strings, sample_rate, rel_support, prune);
    if (pattern_num != NULL)
        *pattern_num = compressor.patternNum;
    if (sample_size != NULL)
        *sample_size = compressor.sampleSize;
    return move(compressed);
}

inline void print_stat(long compressed_size, long original_size,
                long compression_time, long decompression_time) {
    if (PRINT_STATS) {
        cout << fixed << setprecision(3);
        cout << "Compression ratio:\t" << 1.0 * compressed_size / original_size << endl;
        cout << "Original Size:\t\t" << original_size / 1024.0 / 1024 << " MB" << endl;
        cout << "Compressed Size:\t" << compressed_size / 1024.0 / 1024 << " MB" << endl;
        cout << "Compression Time:\t" << compression_time / 1000.0 << " ms" << endl;
        cout << "Decompression Time:\t" << decompression_time / 1000.0 << " ms" << endl;
        cout << "Compression speed:\t" << original_size / 1024.0/ 1024.0 / (compression_time / 1000.0 / 1000.0)
        << " MB/s" << endl;
        cout << endl;
    }
}

cstat compress_file(string file, UseCompressor useCompressor) {
    unsigned long size = 0;
    unsigned long compressedSize = 0;
    long duration = 0;
    long decompressDuration = 0;
    for (int j = 0; size < OVERALL_SIZE; j++) {
        ifstream s(file);
        while (!s.eof() && size < OVERALL_SIZE) {
            string lenData;
            string data;
            int i = 0;
            for( string line; getline( s, line );)
            {
                if (line == "") {
                    continue;
                }
                auto len = line.length();
                char* lenBytes = reinterpret_cast<char*>(&len);
                data.insert(data.end(), lenBytes, lenBytes + 2);
                data.append(line);
                i++;
                if (data.size() > BLOCK_SIZE) {
                    break;
                }
            }
            if (data.empty()) {
                continue;
            }
            string compressed;
            auto t1 = high_resolution_clock::now();
            if (useCompressor == use_snappy) {
                compressed = compress_snappy(data);
            } else {
                compressed = compress_gzip(data);
            }
            auto t2 = high_resolution_clock::now();
            duration += duration_cast<microseconds>( t2 - t1 ).count();
            compressedSize += compressed.length();
            size += data.size();
            string decompressed;
            t1 = high_resolution_clock::now();
            if (useCompressor == use_snappy) {
                decompressed = decomrpess_snappy(compressed);
            } else {
                decompressed = decompress_gzip(compressed);
            }
            t2 = high_resolution_clock::now();
            decompressDuration += duration_cast<microseconds>( t2 - t1 ).count();
        }
        s.close();
    }
    print_stat(compressedSize, size, duration, decompressDuration);
    cstat s = {
        size, compressedSize,
        compressedSize * 1.0 / size,
        duration * 1.0 / 1000,
        decompressDuration * 1.0 / 1000, 0, 0};
    return s;
}

cstat compress_file_frequent(string file, bool prune, int sample_size = 200, int support = 5) {
    std::ifstream in(file, std::ifstream::ate | std::ifstream::binary);
    long file_size = in.tellg();
    in.close();
    
    vector<string> strings;
    long duration = 0;
    long decompressDuration = 0;
    unsigned long size = 0;
    unsigned long compressedSize = 0;
    double totalPatternNum = 0;
    double totalSampleSize = 0;
    int blockNum = 0;
    for (int j = 0; size < OVERALL_SIZE; j++) {
        ifstream s(file);
        unsigned long itrSize = 0;
        while(!s.eof() && size < OVERALL_SIZE) {
            int i = 0;
            unsigned long blockSize = 0;
            strings.clear();
            for (string line; getline(s, line);) {
                if (line == "") {
                    continue;
                }
                strings.push_back(line);
                size += line.length() + 2;
                blockSize += line.length() + 2;
                itrSize += line.length() + 1;
                i++;
                if (blockSize >= BLOCK_SIZE && itrSize < file_size - 2*1024*1024) {
                    break;
                }
            }
            if (strings.empty()) {
                continue;
            }
            auto t1 = high_resolution_clock::now();
            int pattern_num = 0, sample_size = 0;
            auto compressed = compress_frequent(strings, &pattern_num, &sample_size, 0.005, 0.03, prune);
            auto t2 = high_resolution_clock::now();
            totalPatternNum += pattern_num;
            totalSampleSize += sample_size;
            blockNum++;
            compressedSize += compressed.length();
            duration += duration_cast<microseconds>( t2 - t1 ).count();
            
            t1 = high_resolution_clock::now();
            auto decompressed = decompress_frequent(compressed);
            t2 = high_resolution_clock::now();
            decompressDuration += duration_cast<microseconds>( t2 - t1 ).count();
        }
        s.close();
    }
    print_stat(compressedSize, size, duration, decompressDuration);
    cstat s = {
        size, compressedSize,
        compressedSize * 1.0 / size,
        duration * 1.0 / 1000,
        decompressDuration * 1.0 / 1000,
        totalPatternNum / blockNum,
        totalSampleSize / blockNum
    };
    return s;
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
    print_stat(compressedSize, size, duration, decompressDuration);
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
    print_stat(compressedSize, size, duration, decompressDuration);
}

const string indir = "/Users/xiaojianwang/Documents/workspace/benchmarks/gen/";
const string outdir = indir + "out/";

void compress_file_frequent_varying(string file, int repeat = 1){
    
    vector<int> sample_sizes = {150, 200, 300, 500, 700, 1000};
    vector<int> supports = {2, 10, 15, 20, 30, 40, 50};
    
    for(int sample_size : sample_sizes) {
        cout << "Benchmarking for " << file << ". Sample size " << sample_size << endl;
        for (int i = 0; i < repeat; i++) {
            ostringstream ss;
            ss << outdir << "sample_support_count/" << file << "_sample_" << sample_size << ".txt";
            auto f_stat = compress_file_frequent(indir + file + ".txt", sample_size, sample_size / 20, true);
            append_stat(ss.str(), f_stat);
            ss.str("");
            ss << outdir << "sample/" << file << "_sample_" << sample_size << ".txt";
            f_stat = compress_file_frequent(indir + file + ".txt", sample_size, 5, true);
            append_stat(ss.str(), f_stat);
        };
    }
    
    for(int support : supports) {
        cout << "Benchmarking for " << file << ". Support count " << support << endl;
        for (int i = 0; i < repeat; i++) {
            ostringstream ss;
            ss << outdir << "support_count/" << file << "_sc_" << support << ".txt";
            auto f_stat = compress_file_frequent(indir + file + ".txt", 100, support, true); // Try 200?
            append_stat(ss.str(), f_stat);
        };
    }
}

int main(int argc, const char * argv[]) {
    vector<string> infiles = {
        "gen-text",
        "gen-phone_number",
        "gen-address",
        "gen-name",
        "gen-iso8601",
        "gen-uri",
        "gen-email",
        "gen-user_agent",
        "gen-credit_card_number",
        "gen-credit_card_full",
        "gen-sha1",
//        "reddit",
//        "reddit_pure_comment",
//        "NASA_access_log_Jul95",
//        "wiki.1190153705",
//        "clarknet_access_log_Aug28",
        "gen-fine_foods",
        "gen-wiki-links",
//        "ol_dump_authors_2016-04-30",
//        "2013.csv"
    };
    
    for(int j = 0; j < 10; j++) {
        cout << "iteration " << j << ":" << endl;
        for (string& file : infiles) {
            cout << "Benchmarking " << file << endl;
            auto f_stat = compress_file_frequent(indir + file + ".txt", false);
            append_stat(outdir + "frequent_" + file + ".txt", f_stat);
//            auto s_stat = compress_file(indir + file + ".txt", use_snappy);
//            append_stat(outdir + "snappy_" + file + ".txt", s_stat);
            auto fp_stat = compress_file_frequent(indir + file + ".txt", true);
            append_stat(outdir + "frequent_prune_" + file + ".txt", fp_stat);
//            auto g_stat = compress_file(indir + file + ".txt", use_gzip);
//            append_stat(outdir + "gzip_" + file + ".txt", g_stat);
//            auto s_stat = compress_file_snappy("/Users/xiaojianwang/Documents/workspace/benchmarks/xaa");
//            append_stat("/Users/xiaojianwang/Documents/workspace/benchmarks/RC_2015-01_snappy.txt", s_stat);
//            auto f_stat = compress_file_frequent("/Users/xiaojianwang/Documents/workspace/benchmarks/xaa");
//            append_stat("/Users/xiaojianwang/Documents/workspace/benchmarks/RC_2015-01_frequent.txt", f_stat);
//            compress_file_frequent_varying(file, 1);
        }
    }
}
