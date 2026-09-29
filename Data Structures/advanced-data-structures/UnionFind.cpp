//
// Created by hadis on 06/08/2024.
//
#include "UnionFind.h"

shared_ptr<Fleet> UnionFind::find(shared_ptr<Fleet> fleet) {
    std::shared_ptr<Fleet> temp = fleet;
    int sumR = 0;
    while (temp->getParent()->getId() != temp->getId()) {
        sumR += temp->getAddTo();
        temp = temp->getParent();
    }
    std::shared_ptr<Fleet> root = temp;
    temp = fleet;
    int toSubtract = 0;
    while (temp->getParent()->getId() != temp->getId())
    {
        int tempR=temp->getAddTo();
        temp->setAddTo(temp->getAddTo()+sumR - toSubtract - tempR);
        toSubtract += tempR;
        std::shared_ptr<Fleet> helper = temp->getParent();

        temp->setParent(root);
        temp = helper;
    }
    return root;
}



void UnionFind::unite(shared_ptr<Fleet> root1,shared_ptr<Fleet> root2){
    if(root1->getId() != root2->getId()){
        if(root1->getSize() >= root2->getSize()){
            root2->setAddTo(root2->getAddTo() +root1->getSize());
            root1->setSize(root1->getSize() + root2->getSize());
            root1->setNumOfShips(root1->getNumOfShips()+root2->getNumOfShips());
            root2->setParent(root1);
            root2->setAvilability();

        }
        else{
            root1->setAddTo(root1->getAddTo() +root2->getSize());
            root2->setSize(root2->getSize() + root1->getSize());
            root2->setNumOfShips(root2->getNumOfShips()+root1->getNumOfShips());
            root1->setParent(root2);
            root1->setAvilability();
        }
    }
}

