//
// Created by hadis on 06/08/2024.
//

#ifndef WET2_UNIONFIND_H
#define WET2_UNIONFIND_H

#include "MyHashTable.h"
#include "Pirate.h"
#include "Fleet.h"
using namespace std;
class Pirate;
class Fleet;
class UnionFind{
public:
    UnionFind()=default;
    void unite(shared_ptr<Fleet> root1,shared_ptr<Fleet> root2);
    shared_ptr<Fleet> find(shared_ptr<Fleet> pirate);
    ~UnionFind()=default;
};
#endif //WET2_UNIONFIND_H
