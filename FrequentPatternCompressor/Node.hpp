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
#include <list>

using namespace std;

struct Position {
    uint32_t stringIndex;
    uint32_t positionInString;
};

struct Node {
    
    ~Node();
    list<Node*> frequentChildren;
    int index = 0;
    string str;
    vector<int32_t> indices;
    vector<Position> patternPositions;
    uint8_t depth;
    Node* children[256] {NULL}; // Optimization using pointer?
};

#endif /* Node_hpp */
