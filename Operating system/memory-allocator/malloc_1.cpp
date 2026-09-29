#include <unistd.h>
#define MAX 100000000
void* smalloc(size_t size){
    if(size == 0 || size > MAX){
        return NULL;
    }
    void* return_value = sbrk(size);
    if(return_value == (void*)-1){
        return NULL;
    }
    return return_value;
}
