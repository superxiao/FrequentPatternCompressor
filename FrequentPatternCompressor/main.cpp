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

struct comp_stat {
    unsigned long original_size;
    unsigned long compressed_size;
    double compression_ratio;
    double compression_time;
    double decompression_time;
    double num_trie_nodes;
    double sample_size;
};

enum UseCompressor{use_snappy, use_gzip};

void log_stat(string log_file, comp_stat s) {
    fstream stream(log_file, fstream::app);
    stream << s.original_size << " " << s.compressed_size << " "
    << s.compression_ratio << " " << s.compression_time << " "
    << s.decompression_time << " " << s.num_trie_nodes << " "
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
    auto bytes_to_mb = [](long bytes) { return bytes / 1024.0 / 1024.0; };
    auto microsec_to_millisec = [](long millisec) {return millisec / 1000.0;};
    auto microsec_to_sec = [](long millisec) {return millisec / 1.0e6;};
    if (PRINT_STATS) {
        cout << fixed << setprecision(3);
        cout << "Compression ratio:\t" << 1.0 * s.compressed_size / s.original_size << endl;
        cout << "Original Size:\t\t" << bytes_to_mb(s.original_size) << " MB" << endl;
        cout << "Compressed Size:\t" << bytes_to_mb(s.compressed_size) << " MB" << endl;
        cout << "Compression Time:\t" << microsec_to_millisec(s.compression_time) << " ms" << endl;
        cout << "Decompression Time:\t" << microsec_to_millisec(s.decompression_time) << " ms" << endl;
        cout << "Compression speed:\t" << bytes_to_mb(s.original_size) / microsec_to_sec(s.compression_time)
        << " MB/s" << endl;
        cout << endl;
    }
}

comp_stat compress_file(string file, UseCompressor useCompressor, string outfile) {
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
            long block_duration = duration_cast<microseconds>( t2 - t1 ).count();
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
            long block_decomp_duration = duration_cast<microseconds>( t2 - t1 ).count();
            comp_stat block_stat = {
                data.size(), compressed.length(),
                compressed.length() * 1.0 / data.size(),
                block_duration * 1.0 / 1000,
                block_decomp_duration * 1.0 / 1000,
                0,
                0
            };
//            log_stat(outfile, block_stat);
        }
        s.close();
    }
    comp_stat s = {
        size, compressedSize,
        compressedSize * 1.0 / size,
        duration * 1.0 / 1000,
        decompressDuration * 1.0 / 1000, 0, 0};
    print_stat(s);
    return s;
}

const string indir = "/Users/xiaojianwang/Documents/workspace/benchmarks/gen/";
const string outdir = indir + "out/";

comp_stat compress_file_frequent(string file, string outfile, bool prune, double sample_rate = 0.05, int support = 5, bool lookahead = false, unsigned long overall_size = OVERALL_SIZE) {
    
    std::ifstream in(file, std::ifstream::ate | std::ifstream::binary);
    long file_size = in.tellg();
    in.close();
    
    vector<string> strings;
    long duration = 0;
    long decompressDuration = 0;
    unsigned long size = 0;
    unsigned long compressedSize = 0;
    double totalNumTrieNodes = 0;
    double totalSampleSize = 0;
    int blockNum = 0;
    for (int j = 0; size < overall_size; j++) {
        ifstream s(file);
        unsigned long itrSize = 0;
        while(!s.eof() && size < overall_size) {
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
            int num_trie_nodes = 0, sample_size = 0;
            auto compressed = compress_frequent(strings, sample_rate, support, prune, lookahead, &num_trie_nodes, &sample_size);
            auto t2 = high_resolution_clock::now();
            totalNumTrieNodes += num_trie_nodes;
            totalSampleSize += sample_size;
            blockNum++;
            compressedSize += compressed.length();
            duration += duration_cast<microseconds>( t2 - t1 ).count();
            long block_duration = duration_cast<microseconds>( t2 - t1 ).count();
            
            t1 = high_resolution_clock::now();
            auto decompressed = decompress_frequent(compressed);
            t2 = high_resolution_clock::now();
            decompressDuration += duration_cast<microseconds>( t2 - t1 ).count();
            long block_decomp_duration = duration_cast<microseconds>( t2 - t1 ).count();
            
            comp_stat block_stat = {
                blockSize, compressed.length(),
                compressed.length() * 1.0 / blockSize,
                block_duration * 1.0 / 1000,
                block_decomp_duration * 1.0 / 1000,
                num_trie_nodes * 1.0,
                sample_size * 1.0
            };
//            log_stat(outfile, block_stat);
        }
        s.close();
    }
    comp_stat s = {
        size, compressedSize,
        compressedSize * 1.0 / size,
        duration * 1.0 / 1000,
        decompressDuration * 1.0 / 1000,
        totalNumTrieNodes / blockNum,
        totalSampleSize / blockNum
    };
    print_stat(s);
    return s;
}

void compress_file_frequent_varying(string file, string outfolder, int repeat = 1, bool lookahead = false){
    
    vector<double> sample_rates = {0.001, 0.003, 0.005, 0.01, 0.02, 0.03, 0.05};
//    vector<int> supports = {3, 5, 10, 15, 20, 30, 40, 50};
    vector<int> supports = {3};
    unsigned long total_size = OVERALL_SIZE;
    if (lookahead) {
        total_size = 1L * 1024 * 1024 * 1024;
    }
    for (int i = 0; i < repeat; i++) {
        for(double sample_rate : sample_rates) {
            cout << "Benchmarking for " << file << ". Sample rate " << sample_rate << endl;
            ostringstream ss;
//            ss << outdir << "sample_support_count/" << file << "_sample_" << sample_size << ".txt";
//            auto f_stat = compress_file_frequent(indir + file + ".txt", sample_size, sample_size / 20, true);
//            log_stat(ss.str(), f_stat);
            ss.str("");

            ss << outfolder << "sample/" << file << "_sample_" << sample_rate << ".txt";
            auto f_stat = compress_file_frequent(indir + file + ".txt", ss.str(), false, sample_rate, 5, lookahead, total_size);
//            log_stat(ss.str(), f_stat);
//            if (!lookahead) {
//                ss.str("");
//                ss << outfolder << "sample/" << file << "_sample_prune_" << sample_rate << ".txt";
//                auto fp_stat = compress_file_frequent(indir + file + ".txt", ss.str(), true, sample_rate, 5, lookahead);
////                log_stat(ss.str(), fp_stat);
//            }
        };

        for(int support : supports) {
            cout << "Benchmarking for " << file << ". Support count " << support << endl;
            ostringstream ss;
            ss << outfolder << "support/" << file << "_support_" << support << ".txt";
            auto f_stat = compress_file_frequent(indir + file + ".txt", ss.str(), false, 0.005, support, lookahead, total_size);
//            log_stat(ss.str(), f_stat);
//            if(!lookahead) {
//                ss.str("");
//                ss << outfolder << "support/" << file << "_support_prune_" << support << ".txt";
//                auto fp_stat = compress_file_frequent(indir + file + ".txt", ss.str(), true, 0.005, support, lookahead);
////                log_stat(ss.str(), fp_stat);
//            }
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
            auto f_stat = compress_file_frequent(indir + file + ".txt", outdir + "frequent_" + file + ".txt", true, 0.05, 5, false);
//            log_stat(outdir + "frequent_" + file + ".txt", f_stat);
//            auto s_stat = compress_file(indir + file + ".txt", use_snappy, outdir + "snappy_" + file + ".txt");
//            log_stat(outdir + "snappy_" + file + ".txt", s_stat);
//            auto fp_stat = compress_file_frequent(indir + file + ".txt", outdir + "frequent_prune_" + file + ".txt", true);
//            log_stat(outdir + "frequent_prune_" + file + ".txt", fp_stat);
//            auto g_stat = compress_file(indir + file + ".txt", use_gzip, outdir + "gzip_" + file + ".txt");
//            log_stat(outdir + "gzip_" + file + ".txt", g_stat);
//            auto s_stat = compress_file_snappy("/Users/xiaojianwang/Documents/workspace/benchmarks/xaa");
//            log_stat("/Users/xiaojianwang/Documents/workspace/benchmarks/RC_2015-01_snappy.txt", s_stat);
//            auto f_stat = compress_file_frequent("/Users/xiaojianwang/Documents/workspace/benchmarks/xaa");
//            log_stat("/Users/xiaojianwang/Documents/workspace/benchmarks/RC_2015-01_frequent.txt", f_stat);
            
//            compress_file_frequent_varying(file, outdir+"varying_repair/", 1);
//            compress_file_frequent_varying(file, outdir+"varying/", 1);
//            compress_file_frequent_varying(file, outdir+"varying_lookahead/", 1, true);
//            compress_file_frequent_varying(file, outdir+"varying_lookahead/", 1, true);

            
        }
    }
}
