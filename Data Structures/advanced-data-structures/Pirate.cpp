//

#include "Pirate.h"

//
// Created by hadis on 06/08/2024.
Pirate::Pirate(int pirateId,int fleetId,int size):Id(pirateId),salary(0),fleetId(fleetId),rank(size+1)
{
}

int Pirate::getId() {
    return Id;
}

void Pirate::setId(int Id) {
    this->Id=Id;
}

void Pirate::setSalary(int salary) {
    this->salary=salary;
}

int Pirate::getSalary() {
    return salary;
}

int Pirate::getRank()
{
    return this->rank;
}

void Pirate::setRank(int Rank)
{
    this->rank=rank;
}

int Pirate::getFlleetId()
{
    return this->fleetId;
}
