//
// Created by hadis on 06/08/2024.
//

#ifndef WET2_PIRATE_H
#define WET2_PIRATE_H

#include "memory"
#include "Fleet.h"
class Fleet;
class Pirate {
private:
    int Id;
    int salary;
    int fleetId;
    int rank;
public:
    Pirate(int pirateId,int fleetId,int size);
    ~Pirate()=default;
    int getId();
    void setId(int Id);
    void setSalary(int salary);
    int getSalary();
    int getRank();
    void setRank(int Rank);
    int getFlleetId();
};


#endif //WET2_PIRATE_H
