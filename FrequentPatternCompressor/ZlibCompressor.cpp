//
//  ZlibCompressor.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 5/8/16.
//  Copyright © 2016 Xiaojian Wang. All rights reserved.
//

#include "ZlibCompressor.hpp"

#include <sstream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

string ZlibCompressor::Compress(const string& data) {
    namespace bio = boost::iostreams;
    
    std::stringstream compressed;
    std::stringstream origin(data);
    
    bio::filtering_streambuf<bio::input> out;
    out.push(bio::gzip_compressor(bio::gzip_params(bio::gzip::best_compression)));
    out.push(origin);
    bio::copy(out, compressed);
    
    return compressed.str();
}

string ZlibCompressor::Decompress(const string& data) {
    namespace bio = boost::iostreams;

    std::stringstream compressed(data);
    std::stringstream decompressed;

    bio::filtering_streambuf<bio::input> out;
    out.push(bio::gzip_decompressor());
    out.push(compressed);
    bio::copy(out, decompressed);

    return decompressed.str();
}