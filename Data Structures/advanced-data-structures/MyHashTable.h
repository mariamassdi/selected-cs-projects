#ifndef MYHASHTABLE_H
#define MYHASHTABLE_H

#include "AVLTree.h"
#include <memory>
template <class T, class M>
class HashTable
{
private:
    Tree<T, M> *array;
    int size;
    int counter;
    int counterTaken;
    void inorder(typename Tree<T,M>::Node *root,Tree<T,M> *newArray,int newsize);
public:
    explicit HashTable();
    void push(M id,T object);
    void pull(M id);
    T& find(M id);
    bool exist(M id);
    int capacity();
    bool isEmpty();
    void eraseAll();
    ~HashTable();
    int getcounter();
};

template<class T, class M>
int HashTable<T, M>::getcounter() {
    return counter;
}


template <class T, class M>
HashTable<T, M>::HashTable():size(100),counter(0),counterTaken(0)
{
    array = new Tree<T, M>[size];
}
template <class T, class M>
void HashTable<T, M>::push(M id,T object)
{
    int hash = id % size;
    if(array[hash].find(array[hash].root,id) != nullptr){
        return;
    }
    if(array[hash].root == nullptr){
        counterTaken++;
    }
    array[hash].root = array[hash].insert(array[hash].root,id,object);
    counter++;
    if(counterTaken == size){
        Tree<T,M> *newArray = new Tree<T,M> [size*2];
        counterTaken=0;
        for(int i=0;i<size;i++){
            inorder(array[i].root,newArray,size*2);
        }
        size *=2;
        delete[] array;
        array = newArray;
    }
}
template <class T, class M>
HashTable<T, M>::~HashTable()
{
    delete[] array;
}

template<class T,class M>
void HashTable<T,M>::inorder(typename Tree<T,M>::Node *root,Tree<T,M> *newArray,int newsize){
    if (root != nullptr){
        inorder(root->getLeft(),newArray,newsize);
        int newhash= root->getKey() % newsize;
        if(newArray[newhash].root == nullptr){
            counterTaken++;
        }
        newArray[newhash].root = newArray[newhash].insert(newArray[newhash].root,root->getKey(),root->getT());
        inorder(root->getRight(),newArray,newsize);
    }
}
template<class T, class M>
bool HashTable<T, M>::isEmpty() {
    return counter == 0;
}

template <class T, class M>
void HashTable<T, M>::eraseAll()
{
    for(int i=0;i<size;i++){
        array[i].Clear();
    }
    counter = 0;
    counterTaken=0;
    size=100;
    delete[] array;
    array = new Tree<T,M> [size];
}

template<class T, class M>
int HashTable<T, M>::capacity() {
    return size;
}

template<class T, class M>
T& HashTable<T, M>::find(M id) {
    if(counter == 0){
        throw exception();
    }
    int hash = id % size;
    T* find = array[hash].find(array[hash].root,id);
    if(find == nullptr){
        throw exception();
    }
    return *find;
}

template <class T, class M>
bool HashTable<T, M>::exist(M id)
{
    int hash = id % size;
    T *find = array[hash].find(array[hash].root,id);
    bool returnvalue = find != nullptr;
    return returnvalue;
}

template<class T, class M>
void HashTable<T, M>::pull(M id) {
    int hash = id % size;
    array[hash].root = array[hash].Delete(array[hash].root,id);
    if(array[hash].root == nullptr){
        counterTaken--;
    }
    counter--;

    if(counterTaken == size/4){
        Tree<T,M> *newArray = new Tree<T,M> [size/2];
        counterTaken=0;
        for(int i=0;i<size;i++)
        {
            inorder(array[i].root,newArray,size/2);
        }
        size/=2;
        delete[] array;
        array = newArray;
    }
}
#endif // MYHASHTABLE_H