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
#include "FrequentPatternDecompressor.hpp"
#include "snappy/snappy.h"
#include "ZlibCompressor.hpp"

using namespace std;
using namespace std::chrono;

static const bool PRINT_STATS = true;
static const int BLOCK_SIZE = 4 * 1024 * 1024;
static const unsigned long OVERALL_SIZE = 1L * 1024 * 1024 * 1024;

// TODO use current path for input and output files
static const string INDIR = "/Users/xiaojianwang/Documents/workspace/benchmarks/gen/";
static const string OUTDIR = INDIR + "out/";

struct comp_stat {
    unsigned long original_size;
    unsigned long compressed_size;
    double compression_ratio;
    double compression_time; // nanosec
    double decompression_time; // nanosec
    double num_trie_nodes;
    double sample_size;
};

enum UseCompressor{use_snappy, use_gzip};

void log_stat(string log_file, comp_stat s) {
    fstream stream(log_file, fstream::app);
    stream << s.original_size << " " << s.compressed_size << " "
    << s.compression_ratio << " " << s.compression_time / 1000.0 << " "
    << s.decompression_time / 1000.0 << " " << s.num_trie_nodes << " "
    << s.sample_size << endl;
    stream.close();
}

inline string compress_snappy(const string& data) {
    string compressed;
    snappy::Compress(data.c_str(), data.length(), &compressed);
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


inline string compress_frequent(const vector<string>& strings, double sample_rate = 0.005, int support = 5, bool prune = false, bool lookahead = false, int* num_trie_nodes = NULL, int* sample_size = NULL) {
    auto compressor = FrequentPatternCompressor();
    string compressed = compressor.Compress(strings, sample_rate, support, prune, lookahead);
    if (num_trie_nodes != NULL)
        *num_trie_nodes = compressor.num_trie_nodes;
    if (sample_size != NULL)
        *num_trie_nodes = compressor.sampleSize;
    return move(compressed);
}

inline string decompress_frequent(const string& compressed) {
    auto decompressor = FrequentPatternDecompressor();
    vector<uint32_t> lens;
    auto decompressed = decompressor.Decompress(compressed, lens);
    return move(decompressed);
}

inline void print_stat(comp_stat s) {
    auto bytes_to_mb = [](double bytes) { return bytes / 1024.0 / 1024.0; };
    auto nanosec_to_millisec = [](double nanosec) {return nanosec / 1.0e6;};
    auto nanosec_to_sec = [](double nanosec) {return nanosec / 1.0e9;};
    if (PRINT_STATS) {
        cout << fixed << setprecision(3);
        cout << "Compression ratio:\t" << 1.0 * s.compressed_size / s.original_size << endl;
        cout << "Original Size:\t\t" << bytes_to_mb(s.original_size) << " MB" << endl;
        cout << "Compressed Size:\t" << bytes_to_mb(s.compressed_size) << " MB" << endl;
        cout << "Compression Time:\t" << nanosec_to_millisec(s.compression_time) << " ms" << endl;
        cout << "Decompression Time:\t" << nanosec_to_millisec(s.decompression_time) << " ms" << endl;
        cout << "Compression speed:\t" << bytes_to_mb(s.original_size) / nanosec_to_sec(s.compression_time)
        << " MB/s" << endl;
        cout << endl;
    }
}

comp_stat compress_file_baseline(string file, UseCompressor useCompressor, string logFile, bool log = true) {
    unsigned long inputSize = 0;
    unsigned long compressedSize = 0;
    unsigned long compDuration = 0;
    unsigned long decompDuration = 0;
    while (inputSize < OVERALL_SIZE) {
        ifstream s(file);
        while (!s.eof() && inputSize < OVERALL_SIZE) {
            string data;
            for( string line; getline( s, line );)
            {
                if (line == "") {
                    continue;
                }
                auto len = line.length();
                char* lenBytes = reinterpret_cast<char*>(&len);
                data.insert(data.end(), lenBytes, lenBytes + 2);
                data.append(line);
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
            compDuration += duration_cast<microseconds>( t2 - t1 ).count();
            long blockDuration = duration_cast<microseconds>( t2 - t1 ).count();
            compressedSize += compressed.length();
            inputSize += data.size();
            
            string decompressed;
            t1 = high_resolution_clock::now();
            if (useCompressor == use_snappy) {
                decompressed = decomrpess_snappy(compressed);
            } else {
                decompressed = decompress_gzip(compressed);
            }
            t2 = high_resolution_clock::now();
            decompDuration += duration_cast<microseconds>( t2 - t1 ).count();
            long blockDecompDuration = duration_cast<microseconds>( t2 - t1 ).count();
            
            comp_stat block_stat = {
                data.size(), compressed.length(),
                compressed.length() * 1.0 / data.size(),
                blockDuration * 1.0,
                blockDecompDuration * 1.0};
            if (log) {
                log_stat(logFile, block_stat);
            }
        }
        s.close();
    }
    comp_stat s = {
        inputSize, compressedSize,
        compressedSize * 1.0 / inputSize,
        compDuration * 1.0,
        decompDuration * 1.0};
    print_stat(s);
    return s;
}

comp_stat compress_file_frequent(string file, string outfile, bool prune, double sample_rate = 0.05, int support = 5, bool lookahead = false, unsigned long overall_size = OVERALL_SIZE, bool log = true) {
    
    std::ifstream in(file, std::ifstream::ate | std::ifstream::binary);
    long fileSize = in.tellg();
    in.close();
    
    vector<string> strings;
    unsigned long compDuration = 0;
    unsigned long decompressDuration = 0;
    unsigned long inputSize = 0;
    unsigned long compressedSize = 0;
    double totalNumTrieNodes = 0;
    double totalSampleSize = 0;
    int blockNum = 0;
    while (inputSize < overall_size) {
        ifstream s(file);
        unsigned long itrSize = 0;
        while(!s.eof() && inputSize < overall_size) {
            unsigned long blockSize = 0;
            strings.clear();
            for (string line; getline(s, line);) {
                if (line == "") {
                    continue;
                }
                strings.push_back(line);
                inputSize += line.length() + 2;
                blockSize += line.length() + 2;
                itrSize += line.length() + 1;
                if (blockSize >= BLOCK_SIZE && itrSize < fileSize - 2*1024*1024) {
                    break;
                }
            }
            if (strings.empty()) {
                continue;
            }
            auto t1 = high_resolution_clock::now();
            int num_trie_nodes = 0, sample_size = 0;
            auto compressed = compress_frequent(strings, sample_rate, support, prune, lookahead, &num_trie_nodes, &sample_size);
            auto t2 = high_resolution_clock::now();
            totalNumTrieNodes += num_trie_nodes;
            totalSampleSize += sample_size;
            blockNum++;
            compressedSize += compressed.length();
            compDuration += duration_cast<microseconds>( t2 - t1 ).count();
            long block_duration = duration_cast<microseconds>( t2 - t1 ).count();
            
            t1 = high_resolution_clock::now();
            auto decompressed = decompress_frequent(compressed);
            t2 = high_resolution_clock::now();
            decompressDuration += duration_cast<microseconds>( t2 - t1 ).count();
            long block_decomp_duration = duration_cast<microseconds>( t2 - t1 ).count();
            
            comp_stat block_stat = {
                blockSize, compressed.length(),
                compressed.length() * 1.0 / blockSize,
                block_duration * 1.0,
                block_decomp_duration * 1.0,
                num_trie_nodes * 1.0,
                sample_size * 1.0
            };
            if(log) {
                log_stat(outfile, block_stat);
            }
        }
        s.close();
    }
    comp_stat s = {
        inputSize, compressedSize,
        compressedSize * 1.0 / inputSize,
        compDuration * 1.0,
        decompressDuration * 1.0,
        totalNumTrieNodes / blockNum,
        totalSampleSize / blockNum
    };
    print_stat(s);
    return s;
}

void compress_file_frequent_varying(string file, string outfolder, int repeat = 1, bool lookahead = false){
    
    vector<double> sample_rates = {0.001, 0.003, 0.005, 0.01, 0.02, 0.03, 0.05};
    vector<int> supports = {3, 5, 10, 15, 20, 30, 40, 50};
    unsigned long total_size = OVERALL_SIZE;
    if (lookahead) {
        total_size = 1L * 1024 * 1024 * 1024;
    }
    for (int i = 0; i < repeat; i++) {
        for(double sample_rate : sample_rates) {
            cout << "Benchmarking for " << file << ". Sample rate " << sample_rate << endl;
            ostringstream ss;
            ss << outfolder << "sample/" << file << "_sample_" << sample_rate << ".txt";
            compress_file_frequent(INDIR + file + ".txt", ss.str(), false, sample_rate, 5, lookahead, total_size);
            if (!lookahead) {
                ss.str("");
                ss << outfolder << "sample/" << file << "_sample_prune_" << sample_rate << ".txt";
                compress_file_frequent(INDIR + file + ".txt", ss.str(), true, sample_rate, 5, lookahead);
            }
        };

        for(int support : supports) {
            cout << "Benchmarking for " << file << ". Support count " << support << endl;
            ostringstream ss;
            ss << outfolder << "support/" << file << "_support_" << support << ".txt";
            compress_file_frequent(INDIR + file + ".txt", ss.str(), false, 0.005, support, lookahead, total_size);
            if(!lookahead) {
                ss.str("");
                ss << outfolder << "support/" << file << "_support_prune_" << support << ".txt";
                compress_file_frequent(INDIR + file + ".txt", ss.str(), true, 0.005, support, lookahead);
            }
        };
    }
}

int main(int argc, const char * argv[]) {
    vector<string> infiles = {
//        "gen-text",
//        "gen-phone_number",
//        "gen-address",
//        "gen-name",
//        "gen-iso8601",
//        "gen-uri",
//        "gen-email",
//        "gen-user_agent",
//        "gen-credit_card_number",
//        "gen-credit_card_full",
//        "gen-sha1",
        "gen-fine_foods",
//        "gen-wiki-links",
    };
    
    for(int j = 0; j < 10; j++) {
        cout << "iteration " << j << ":" << endl;
        for (string& file : infiles) {
            cout << "Benchmarking " << file << endl;
            compress_file_frequent(INDIR + file + ".txt", OUTDIR + "frequent_" + file + ".txt", true, 0.05, 5, false);
            compress_file_baseline(INDIR + file + ".txt", use_snappy, OUTDIR + "snappy_" + file + ".txt");
            compress_file_frequent(INDIR + file + ".txt", OUTDIR + "frequent_prune_" + file + ".txt", true);
//            auto g_stat = compress_file_baseline(indir + file + ".txt", use_gzip, outdir + "gzip_" + file + ".txt");
            
//            compress_file_frequent_varying(file, outdir+"varying_repair/", 1);
//            compress_file_frequent_varying(file, outdir+"varying/", 1);
//            compress_file_frequent_varying(file, outdir+"varying_lookahead/", 1, true);
            
        }
    }
}
