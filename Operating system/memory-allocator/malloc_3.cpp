#include <string.h>
#include <unistd.h>
#include <math.h>
#include <cstdint>
#include <sys/mman.h>
#include <iostream>
#define ORDER 10
#define MAX_BLOCK_SIZE 128 * 1024
#define MAX_SIZE 100000000

class List
{
private:
    class MallocMetadata
    {
    public:
        size_t size;
        size_t block_size;
        bool is_free;
        MallocMetadata *prev;
        MallocMetadata *next;
    };
    class small_list
    {
    public:
        MallocMetadata *head;
        MallocMetadata *tail;
        size_t size;
    };

    small_list allocated[ORDER + 1];
    small_list freed[ORDER + 1];

    size_t diff;
    bool first;
    size_t num_free_bytes;
    size_t num_allocated_blocks_by_mmap;
    size_t num_allocated_bytes;
    const size_t size_meta_data = sizeof(MallocMetadata);
    void insert_by_location(small_list &lst, MallocMetadata *block);
    int delete_from_list(small_list &list, MallocMetadata *p);

public:
    List();
    void merge(int order);
    bool checkmerge(void *p, size_t block_size, size_t wanted_size);
    void *merge_blocks_if_needed(void *oldp, size_t size);
    void create_blocks();
    void *add(size_t size);
    void remove(void *p);
    bool split(int order);

    size_t get_num_free_blocks();
    size_t get_num_free_bytes();
    size_t get_num_allocated_blocks();
    size_t get_num_allocated_bytes();
    size_t get_num_meta_data_bytes();
    size_t get_size_meta_data();
    size_t get_block_size(void *p);
    size_t get_size(void *p);
    void set_size(void *p, size_t size);
};

List::List() : allocated(), freed(), diff(0), first(true), num_free_bytes(0), num_allocated_blocks_by_mmap(0),
               num_allocated_bytes(0)
{
}

void *List::add(size_t size)
{
    int order = ceil(std::log2(ceil(double(size + size_meta_data) / double(128))));
    if (order > ORDER)
    {
        void *ptr = mmap(nullptr, size + size_meta_data, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
        if (ptr == (void *)-1)
        {
            return nullptr;
        }
        MallocMetadata *res = (MallocMetadata *)ptr;
        res->block_size = size + size_meta_data;
        res->size = size;
        res->next = nullptr;
        res->prev = nullptr;
        res->is_free = false;

        num_allocated_blocks_by_mmap++;
        num_allocated_bytes += res->size;
        return res;
    }
    else
    {
        if (freed[order].size == 0)
        {
            if (order == ORDER)
            {
                return nullptr;
            }
            split(order + 1);
        }
        if (freed[order].size == 0)
        {
            return nullptr;
        }
        MallocMetadata *Toalloc = freed[order].head;

        Toalloc->is_free = false;
        Toalloc->size = size;
        // remove from freed[order]
        delete_from_list(freed[order], Toalloc);
        num_free_bytes -= (Toalloc->block_size - size_meta_data);
        // finished remove from freed[order]

        // add to allocated[order]
        insert_by_location(allocated[order], Toalloc);
        // finished add to allocated[order]
        return Toalloc;
    }
    return nullptr;
}

int List::delete_from_list(List::small_list &list, MallocMetadata *pMetaData)
{
    int flag = 0;
    if (list.size == 1)
    {
        if (list.head == pMetaData)
        {
            list.size--;
            list.head->next = nullptr;
            list.head->prev = nullptr;
            list.head = nullptr;
            list.tail = nullptr;
            flag = 1;
        }
    }
    else
    {
        if (pMetaData == list.head)
        {
            list.head = pMetaData->next;
            list.head->prev = nullptr;
            pMetaData->next = nullptr;
            pMetaData->prev = nullptr;
            list.size--;
            return 1;
        }
        else if (pMetaData == list.tail)
        {
            list.tail = pMetaData->prev;
            list.tail->next = nullptr;
            pMetaData->next = nullptr;
            pMetaData->prev = nullptr;
            list.size--;
            return 1;
        }
        List::MallocMetadata *temp = list.head;
        while ((temp != nullptr))
        {
            if (temp == pMetaData)

            {
                if (temp->next != nullptr)
                {
                    temp->next->prev = temp->prev;
                }
                else
                {
                    list.tail = temp->prev;
                }
                if (temp->prev != nullptr)
                {
                    temp->prev->next = temp->next;
                }
                else
                {
                    list.head = temp->next;
                }
                temp->next = nullptr;
                temp->prev = nullptr;
                list.size--;
                flag = 1;
            }
            temp = temp->next;
        }
    }
    return flag;
}

void List::remove(void *p)
{
    MallocMetadata *pMetaData = (MallocMetadata *)((char *)p - size_meta_data);
    if (pMetaData->is_free == true || pMetaData == nullptr)
    {
        return;
    }
    pMetaData->is_free = true;
    int order = ceil(std::log2(ceil((double)(pMetaData->block_size) / 128.0)));
    if (order > ORDER)
    {
        size_t size = pMetaData->size;
        munmap(pMetaData, pMetaData->block_size);
        num_allocated_blocks_by_mmap--;

        num_allocated_bytes -= size;
    }
    else
    {
        int flag = delete_from_list(allocated[order], pMetaData);
        if (flag)
        {
            pMetaData->next = nullptr;
            pMetaData->prev = nullptr;

            insert_by_location(freed[order], pMetaData);
            num_free_bytes += pMetaData->block_size - size_meta_data;
            merge(order);
        }
    }
}

void List::merge(int order)
{
    for (int i = order; i < 10; i++)
    {
        int flag = 0;
        MallocMetadata *temp = freed[i].head;

        while (temp != nullptr && temp->next != nullptr && flag == 0)
        {
            if (((reinterpret_cast<size_t>(temp) + diff) xor (temp->block_size)) == reinterpret_cast<size_t>(temp->next) + diff)
            {
                MallocMetadata *buddy = temp;
                buddy->block_size = temp->block_size * 2;
                delete_from_list(freed[i], temp->next);
                delete_from_list(freed[i], temp);

                insert_by_location(freed[i + 1], buddy);

                num_allocated_bytes += size_meta_data;
                num_free_bytes += size_meta_data;
                flag = 1;
            }
            temp = temp->next;
        }
    }
}
bool List::checkmerge(void *p, size_t block_size, size_t wanted_size)
{
    if (block_size >= wanted_size)
    {
        return true;
    }
    if (block_size == MAX_BLOCK_SIZE)
    {
        return false;
    }
    MallocMetadata *ptr = (MallocMetadata *)(p);
    MallocMetadata *buddy = (MallocMetadata *)(((reinterpret_cast<size_t>(p) + diff) xor (block_size)) - diff);
    if (buddy->is_free == true && buddy->block_size == block_size)
    {
        uintptr_t min = reinterpret_cast<uintptr_t>((char *)ptr < (char *)buddy ? ptr : buddy);
        return checkmerge((void *)(min), block_size * 2, wanted_size);
    }
    return false;
}

void *List::merge_blocks_if_needed(void *oldp, size_t size)
{
    MallocMetadata *original = (MallocMetadata *)(oldp);
    MallocMetadata *ptr = original;
    size_t old_size = ptr->size;
    int order = ceil(std::log2(ceil(double(ptr->block_size) / double(128))));
    delete_from_list(allocated[order], ptr);
    while (true)
    {
        MallocMetadata *buddy = (MallocMetadata *)(((reinterpret_cast<size_t>(ptr) + diff) xor (ptr->block_size)) - diff);
        if (buddy->block_size != ptr->block_size || buddy->is_free == false || ptr->block_size >= (size + size_meta_data))
        {
            break;
        }
        int order2 = ceil(std::log2(ceil(double(buddy->block_size) / double(128))));
        delete_from_list(freed[order2], buddy);
        size_t merged_size = ptr->block_size * 2;
        num_allocated_bytes += size_meta_data;
        num_free_bytes -= (buddy->block_size - size_meta_data);

        buddy->is_free = false;
        ptr = (MallocMetadata *)(reinterpret_cast<uintptr_t>(
            (char *)ptr < (char *)buddy ? ptr : buddy));

        ptr->block_size = merged_size;
        ptr->is_free = false;
 }
    if (ptr != original)
    {
        memmove((char *)ptr + size_meta_data,
                (char *)original + size_meta_data,
                old_size);
    }
    ptr->size = size;
    int order3 = ceil(std::log2(ceil(double(ptr->size + size_meta_data) / double(128))));
    ptr->block_size = 128 * pow(2, order3);
    insert_by_location(allocated[order3], ptr);
    return (void *)((char *)ptr + size_meta_data);
}

size_t List::get_num_free_blocks()
{
    size_t sum = 0;
    for (int i = 0; i < 11; i++)
    {
        sum += freed[i].size;
    }
    return sum;
}

size_t List::get_num_free_bytes()
{
    return num_free_bytes;
}

size_t List::get_num_allocated_blocks()
{
    size_t sum = 0;
    for (int i = 0; i < 11; i++)
    {
        sum += (freed[i].size + allocated[i].size);
    }
    return sum + num_allocated_blocks_by_mmap;
}

size_t List::get_num_allocated_bytes()
{
    return num_allocated_bytes;
}

size_t List::get_num_meta_data_bytes()
{
    return get_num_allocated_blocks() * size_meta_data;
}

size_t List::get_size_meta_data()
{
    return size_meta_data;
}

size_t List::get_block_size(void *p)
{
    MallocMetadata *pMetaData = (MallocMetadata *)((char *)p - size_meta_data);
    return pMetaData->block_size;
}
size_t List::get_size(void *p)
{
    MallocMetadata *pMetaData =(MallocMetadata *)((char *)p - size_meta_data);

    return pMetaData->size;
}

void List::set_size(void *p, size_t size)
{
    MallocMetadata *pMetaData =(MallocMetadata *)((char *)p - size_meta_data);

    pMetaData->size = size;
}
void List::insert_by_location(small_list &lst, MallocMetadata *block)
{
    if (lst.size == 0)
    {
        lst.head = lst.tail = block;
        block->next = nullptr;
        block->prev = nullptr;
    }
    else if (reinterpret_cast<uintptr_t>(lst.tail) <= reinterpret_cast<uintptr_t>(block))
    {
        block->next = nullptr;  
        block->prev = lst.tail;
        lst.tail->next = block;
        lst.tail = block;
    }
    else if (reinterpret_cast<uintptr_t>(lst.head) > reinterpret_cast<uintptr_t>(block))
    {
        block->prev = nullptr;
        block->next = lst.head;
        lst.head->prev = block;
        lst.head = block;
    }
    else
    {
        MallocMetadata *temp = lst.head;
        while (temp != nullptr)
        {
            if (reinterpret_cast<uintptr_t>(temp) > reinterpret_cast<uintptr_t>(block))
            {
                block->next = temp;
                block->prev = temp->prev;
                if (temp->prev != nullptr)
                {
                    temp->prev->next = block;
                }
                temp->prev = block;
                break;
            }
            temp = temp->next;
        }
    }
    lst.size++;
}

void List::create_blocks()
{
    if (!first)
    {
        return;
    }
    first = false;
    size_t Program_break = (size_t)(sbrk(0));
    size_t diff = (size_t)(pow(2, ceil(std::log2(Program_break))) - Program_break);
    this->diff = diff;
    char *allocation = (char *)sbrk(32 * (MAX_BLOCK_SIZE));
    for (int i = 0; i <= ORDER; i++)
    {
        freed[i] = {nullptr, nullptr, 0};
        allocated[i] = {nullptr, nullptr, 0};
    }
    for (int i = 0; i < 32; i++)
    {
        MallocMetadata *data = (MallocMetadata *)(allocation + (MAX_BLOCK_SIZE)*i);
        data->is_free = true;
        data->size = MAX_BLOCK_SIZE - size_meta_data;
        data->block_size = MAX_BLOCK_SIZE;
        insert_by_location(freed[ORDER], data);
    }
    num_free_bytes = 32 * (MAX_BLOCK_SIZE)-32 * size_meta_data;
    num_allocated_bytes = num_free_bytes;
}

bool List::split(int order)
{
    if (order == ORDER + 1)
    {
        return false;
    }
    if (freed[order].size == 0)
    {
        if (split(order + 1) == false)
        {
            return false;
        }
    }
    MallocMetadata *temp = freed[order].head;
    MallocMetadata *buddy1, *buddy2;
    buddy1 = temp;
    buddy1 = temp;
    buddy1->block_size = (temp->block_size) / 2;
    buddy1->is_free = true;

    buddy2 = (MallocMetadata *)((char *)temp + (buddy1->block_size));
    buddy2->is_free = true;
    buddy2->block_size = buddy1->block_size;

    delete_from_list(freed[order], temp);
    insert_by_location(freed[order - 1], buddy1);
    insert_by_location(freed[order - 1], buddy2);

    num_allocated_bytes -= size_meta_data;
    num_free_bytes -= size_meta_data;

    return true;
}

/////////////////////////////////////////////////the implemntation of the functions /////////////////////////////////////

List list;

void *smalloc(size_t size)
{
    list.create_blocks();
    if (size <= 0 || size > MAX_SIZE)
    {
        return nullptr;
    }
    void *res = list.add(size);

    if (res == NULL)
        return NULL;
    else
    {
        return (char *)res + list.get_size_meta_data();
    }
}

void *scalloc(size_t num, size_t size)
{
     if (num == 0 || size == 0 || num > MAX_SIZE / size)
    {
        return NULL;
    }

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
    if (size <= 0 || size > MAX_SIZE)
    {
        return NULL;
    }

    if (oldp == NULL)
    {
        return smalloc(size);
    }

    size_t old_size = list.get_size(oldp);

    if (list.get_block_size(oldp) > MAX_BLOCK_SIZE)
    {
        if (old_size == size)
        {
            return oldp;
        }

        void *res = smalloc(size);
        if (res == nullptr)
        {
            return nullptr;
        }

        memmove(res, oldp,old_size < size ? old_size : size);

        sfree(oldp);
        return res;
    }

    if (list.get_block_size(oldp)- list.get_size_meta_data() >= size)
    {
        list.set_size(oldp, size);
        return oldp;
    }

    if (list.checkmerge((void *)((char *)oldp- list.get_size_meta_data()),list.get_block_size(oldp),size + list.get_size_meta_data()))
    {
        return list.merge_blocks_if_needed((void *)((char *)oldp -list.get_size_meta_data()),size);
    }

    void *res = smalloc(size);
    if (res == nullptr)
    {
        return nullptr;
    }

    memmove(res, oldp, old_size);
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
