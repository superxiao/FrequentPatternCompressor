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
    size_t stringIndex;
    size_t positionInString;
};

struct Node {
    Node* children[256] {NULL}; // Optimization using pointer?
    list<Node*> frequentChildren;
    Node* parent;
    int index = -1;
    int usage = 0;
    int code;
    int codeLen;
    string string;
    vector<Position> patternPositions;
};

#endif /* Node_hpp */
