#ifndef __mm__memory_master__
#define __mm__memory_master__

#include <iostream>
#include <cstdlib>
#include <cassert>

#include "MemoryPool.h"
// #define USE_GENERAL_MEMORY_POOL

namespace mm {

struct MemoryList {
    std::size_t size;
    const char* fileName;
    int lineNo;
    bool usePool;
    MemoryList* next;
};

class MemoryMaster {
public:
    static MemoryMaster& Instance() {
        static MemoryMaster __instance;
        return __instance;
    }

    MemoryMaster() : memory_list_(NULL), general_pool_() {}

    void *Alloc(std::size_t size, const char* name, int line) {
        void* allocPtr = NULL;
#ifdef MM_USE_GENERAL_MEMORY_POOL
        allocPtr = general_pool_.poolAlloc(size + sizeof(MemoryList));
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

        if (!memory_list_) {
            memory_list_ = current;
        } else {
            MemoryList *last = memory_list_;
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

    void Free(void *freePtr) {
        MemoryList* current =
            reinterpret_cast<MemoryList*>(static_cast<char*>(freePtr) - sizeof(MemoryList));

#ifdef MM_USE_GENERAL_MEMORY_POOL
        if (current->usePool) {
            general_pool_.poolFree(current, current->size + sizeof(MemoryList));
            return;
        }
#endif

        if (memory_list_ == current) {
            memory_list_ = current->next;
        } else {
            for (MemoryList *p = memory_list_; p; p = p->next) {
                if (p->next == current) {
                    p->next = current->next;
                }
            }
        }
        std::free(current);
    }

    void leakReport() {
        for (MemoryList* ptr = memory_list_; ptr; ptr = ptr->next) {
            std::cout << "FileName : "<< ptr->fileName
                      << ", LineNo(" << ptr->lineNo << ")"
                      << ", Size(" << ptr->size << ")"
                      << std::endl;
        }
    }

private:
    MemoryList *memory_list_;
    GeneralMemoryPool general_pool_;
};
    
} /* end of namespace MemoryMaster */

#ifdef MM_OVERRIDE_OPERATOR
inline void* operator new(std::size_t size) {
    return mm::MemoryMaster::Instance().Alloc(size, "Unknown", 0);
}

inline void* operator new(std::size_t size, const char* name, int line) {
    return mm::MemoryMaster::Instance().Alloc(size, name, line);
}

inline void* operator new[](std::size_t size, const char* name, int line) {
    return ::operator new(size, name, line);
}

inline void operator delete(void* deletePtr) {
    mm::MemoryMaster::Instance().Free(deletePtr);
}

inline void operator delete[](void* deletePtr) {
    ::operator delete(deletePtr);
}
#endif

#endif  /* defined(__mm__memory_master__) */
