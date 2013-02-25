#ifndef _MEMORY_MASTER_H_
#define _MEMORY_MASTER_H_

#include <iostream>
#include <cstdlib>
#include <cassert>

#include "MemoryPool.h"
// #define USE_GENERAL_MEMORY_POOL

namespace mm {

struct MemoryList {
#ifdef USE_GENERAL_MEMORY_POOL
    static mm::GeneralMemoryPool& Gmp() {
        static mm::GeneralMemoryPool __instance;
        return __instance;
    }
#endif

    std::size_t size;
    const char* fileName;
    int lineNo;
    bool usePool;
    MemoryList* next;

    static MemoryList* memory;
};

static inline void* memAlloc(std::size_t size, const char* name, int line) {
    void* allocPtr = NULL;
#ifdef USE_GENERAL_MEMORY_POOL
    allocPtr = MemoryList::Gmp().poolAlloc(size + sizeof(MemoryList));
    if (allocPtr) {
        MemoryList* current = static_cast<MemoryList*>(allocPtr);
        current->size = size;
        current->fileName = name;
        current->lineNo = line;
        current->usePool = true;
        return static_cast<void*>(static_cast<char*>(allocPtr) + sizeof(MemoryList));
    }
#endif

    allocPtr = std::malloc(size + sizeof(MemoryList));
    assert(allocPtr);
    MemoryList* current = static_cast<MemoryList*>(allocPtr);
    current->next = NULL;

    if (!MemoryList::memory) {
        MemoryList::memory = current;
    }
    else {
        MemoryList* last = MemoryList::memory;
        while (last->next) {
            last = last->next;
        }
        last->next = current;
    }
    current->size = size;
    current->fileName = name;
    current->lineNo = line;
    current->usePool = false;
    return static_cast<void*>(static_cast<char*>(allocPtr) + sizeof(MemoryList));
}

static inline void memFree(void* freePtr) {
    MemoryList* current =
        reinterpret_cast<MemoryList*>(static_cast<char*>(freePtr) - sizeof(MemoryList));

#ifdef USE_GENERAL_MEMORY_POOL
    if (current->usePool) {
        MemoryList::Gmp().poolFree(current, current->size + sizeof(MemoryList));
        return;
    }
#endif

    if (MemoryList::memory == current) {
        MemoryList::memory = current->next;
    }
    else {
        for (MemoryList* p = MemoryList::memory; p; p = p->next) {
            if (p->next == current) {
                p->next = current->next;
            }
        }
    }
    std::free(current);
}

static inline void leakReport() {
    for (MemoryList* ptr = MemoryList::memory; ptr; ptr = ptr->next) {
        std::cout << "FileName : "<< ptr->fileName
                  << ", LineNo(" << ptr->lineNo << ")"
                  << ", Size(" << ptr->size << ")"
                  << std::endl;
    }
}

MemoryList *MemoryList::memory = NULL;

} /* end of namespace MemoryMaster */

inline void* operator new(std::size_t size) {
    return mm::memAlloc(size, "Unknown", 0);
}

inline void* operator new(std::size_t size, const char* name, int line) {
    return mm::memAlloc(size, name, line);
}

inline void* operator new[](std::size_t size, const char* name, int line) {
    return ::operator new(size, name, line);
}

inline void operator delete(void* deletePtr) {
    mm::memFree(deletePtr);
}

inline void operator delete[](void* deletePtr) {
    ::operator delete(deletePtr);
}

#endif



