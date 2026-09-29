//
// Created by hadis on 06/08/2024.
//

#ifndef WET2_FLEET_H
#define WET2_FLEET_H

#include "memory"
#include "Pirate.h"
class Pirate;
class Fleet
{
private:
    int Id;
    int numOfShips;
    int size;
    int AddTo;
    std::weak_ptr<Fleet> parent;
    bool isAvilable;
public:
    Fleet(int Id);
    ~Fleet()=default;
    bool getAvilability();
    void setAvilability();
    int getId();
    void setId(int Id);
    int getSize();
    void setSize(int Size);
    int getAddTo();
    void setAddTo(int Add);
    int getNumOfShips();
    void setNumOfShips(int Num);
    std::shared_ptr<Fleet> getParent();
    void setParent(std::shared_ptr<Fleet> root);
};


#endif //WET2_FLEET_H
