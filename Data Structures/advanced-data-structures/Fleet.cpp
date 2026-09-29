//
// Created by hadis on 06/08/2024.
//

#include "Fleet.h"

Fleet::Fleet(int Id):Id(Id),numOfShips(1),size(0),AddTo(0),isAvilable(true)
{
}

int Fleet::getId()
{
    return Id;
}

void Fleet::setId(int Id)
{
    this->Id=Id;
}

int Fleet::getNumOfShips()
{
    return numOfShips;
}

void Fleet::setNumOfShips(int Num)
{
    this->numOfShips=Num;
}

std::shared_ptr<Fleet> Fleet::getParent()
{
    return this->parent.lock();
}

void Fleet::setParent(std::shared_ptr<Fleet> root)
{
    this->parent = root;
}

int Fleet::getSize() {
    return size;
}

void Fleet::setSize(int Size) {
    this->size=Size;
}

int Fleet::getAddTo()
{
    return AddTo;
}

void Fleet::setAddTo(int Add)
{
    this->AddTo = Add;
}

void Fleet::setAvilability() {
    this->isAvilable = false;
}

bool Fleet::getAvilability() {
    return isAvilable;
}
