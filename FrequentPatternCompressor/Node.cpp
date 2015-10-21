//
//  Node.cpp
//  FrequentPatternCompressor
//
//  Created by Xiaojian Wang on 10/16/15.
//  Copyright © 2015 Xiaojian Wang. All rights reserved.
//

#include "Node.hpp"

Node::~Node() {
    for(Node* node : children) { // Delete from frequent list?
        delete node;
    }
}