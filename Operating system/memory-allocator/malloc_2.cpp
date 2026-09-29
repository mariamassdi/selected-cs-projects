#include <string.h>
#include <unistd.h>
#define MAX_BLOCK_SIZE 100000000
class List
{
private:
    class MallocMetadata
    {
    public:
        size_t size;
        bool is_free;
        MallocMetadata *prev;
        MallocMetadata *next;
    };
    MallocMetadata *head;
    MallocMetadata *tail;
    size_t num_free_blocks;      // when free or when we take an allocated place that was freed
    size_t num_free_bytes;       // when free or when we take an allocated place that was freed
    size_t num_allocated_blocks; // when we call sbrk
    size_t num_allocated_bytes;  // when we call sbrk
    size_t num_meta_data_bytes;  // when we call sbrk
    const size_t size_meta_data = sizeof(MallocMetadata);

public:
    List();
    void *add(size_t size);
    void remove(void *p);
    size_t get_num_free_blocks();
    size_t get_num_free_bytes();
    size_t get_num_allocated_blocks();
    size_t get_num_allocated_bytes();
    size_t get_num_meta_data_bytes();
    size_t get_size_meta_data();
    size_t get_block_size(void *p);
};

List::List() : head(nullptr), tail(nullptr), num_free_blocks(0), num_free_bytes(0), num_allocated_blocks(0),
               num_allocated_bytes(0), num_meta_data_bytes(0)
{
}
void *List::add(size_t size)
{
    MallocMetadata *temp = head;
    /// need to check a place in the list

    while (temp)
    {
        if (temp->is_free && temp->size >= size)
        {
            num_free_blocks--;
            temp->is_free = false;
            num_free_bytes -= temp->size;
            return temp;
        }
        temp = temp->next;
    }
    void *alloc = sbrk(size + sizeof(List::MallocMetadata));

    if (alloc == (void *)-1)
    {
        return nullptr;
    }
    num_allocated_blocks++;
    num_allocated_bytes += size;
    num_meta_data_bytes += size_meta_data;
    MallocMetadata *newMetaData = (MallocMetadata *)alloc;
    newMetaData->size = size;
    newMetaData->next = nullptr;
    newMetaData->prev = tail;
    newMetaData->is_free = false;

    if (head == nullptr)
    {
        head = tail = newMetaData;
        newMetaData->next = nullptr;
        newMetaData->prev = nullptr;
    }
    else
    {
        tail->next = newMetaData;
        tail = tail->next;
    }
    return alloc;
}

void List::remove(void *p)
{
    MallocMetadata *pMetaData = (MallocMetadata *)((char *)p - size_meta_data);
    if (pMetaData->is_free == true)
    {
        return;
    }
    pMetaData->is_free = true;
    num_free_blocks++;
    num_free_bytes += pMetaData->size;
}

size_t List::get_num_free_blocks()
{
    return num_free_blocks;
}

size_t List::get_num_free_bytes()
{
    return num_free_bytes;
}

size_t List::get_num_allocated_blocks()
{
    return num_allocated_blocks;
}

size_t List::get_num_allocated_bytes()
{
    return num_allocated_bytes;
}

size_t List::get_num_meta_data_bytes()
{
    return num_meta_data_bytes;
}

size_t List::get_size_meta_data()
{
    return size_meta_data;
}

size_t List::get_block_size(void *p)
{
    MallocMetadata *pMetaData = (MallocMetadata *)((char *)p - size_meta_data);
    return pMetaData->size;
}
/////////////////////////////////////////////////the implemntation of the functions /////////////////////////////////////



List list;

void *smalloc(size_t size)
{
    if (size <= 0 || size > MAX_BLOCK_SIZE)
    {
        return nullptr;
    }
    void *res = list.add(size);

    if(res==NULL)
    return NULL;
    else {
        return (char *)res + list.get_size_meta_data();
    }
}

void *scalloc(size_t num, size_t size)
{
    void *res = smalloc(num * size);

    if (res == NULL)
    {
        return NULL;
    }
    memset(res, 0, num * size);
    return res;
}

void sfree(void *ptr)
{
    if (ptr != NULL)
        list.remove(ptr);
}

void *srealloc(void *oldp, size_t size)
{
    if (size <= 0 || size > MAX_BLOCK_SIZE)
    {
        return NULL;
    }

    if (oldp == NULL)
    {
        return smalloc(size);
    }

    if (list.get_block_size(oldp) >= size)
    {
        return oldp;
    }
    void *res = smalloc(size);

    if (res == NULL)
    {
        return NULL;
    }

    memmove(res, oldp, size);

    sfree(oldp);
    return res;
}

size_t _num_free_blocks()
{
    return list.get_num_free_blocks();
}

size_t _num_free_bytes()
{
    return list.get_num_free_bytes();
}

size_t _num_allocated_blocks()
{
    return list.get_num_allocated_blocks();
}

size_t _num_allocated_bytes()
{
    return list.get_num_allocated_bytes();
}

size_t _num_meta_data_bytes()
{
    return list.get_num_meta_data_bytes();
}

size_t _size_meta_data()
{
    return list.get_size_meta_data();
}
