//
//  Node.hpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 10/16/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#ifndef Node_hpp
#define Node_hpp

#include <stdio.h>
#include <string>
#include <vector>

using namespace std;

struct Node {
    Node* children[256];
    Node* parent;
    int index = -1;
    int usage = 0;
    int code;
    int codeLen;
    string string;
    vector<int> patternPositions;
};

#endif /* Node_hpp */
