//
//  Utils.hpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 10/20/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#ifndef Utils_hpp
#define Utils_hpp

#include <stdio.h>
#include <vector>
#include <cstring>
#include <string>

using namespace std;

vector<string> random_dates(int n);

void encodeArray(const uint32_t *in, const size_t length, uint32_t *out,
                 size_t &nvalue);

const uint32_t * decodeArray(const uint32_t *in, const size_t length,
                             uint32_t *out, size_t & nvalue);


#endif /* Utils_hpp */
