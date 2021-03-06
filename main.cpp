#include "MemoryMaster.h"
#include <boost/progress.hpp>

// #define new new(__FILE__, __LINE__)

class Base {
  int a, b;
  float c, d;
};

class Test : public Base {
public:
  Test() {}
  ~Test() {}
};

class TestAppendMemoryPool :
  public Base,
  public mm::Poolable<Test, 10000>
{
public:
  TestAppendMemoryPool() {}
  ~TestAppendMemoryPool() {}
};

int main() {
  {
    boost::progress_timer t;
    Test* ptr;
    for (int j = 0; j < 10000; ++j) {
      for (int i = 0; i < 10000; ++i) {
        ptr = new Test;
        delete ptr;
      }
    }
  }
  {
    boost::progress_timer t;
    TestAppendMemoryPool::Init();
    TestAppendMemoryPool* ptr;
    for (int j = 0; j < 10000; ++j) {
      for (int i = 0; i < 10000; ++i) {
          ptr = new TestAppendMemoryPool;
        delete ptr;
      }
    }
  }
  {
    boost::progress_timer t;
    mm::PoolableFactory<Test, 10000> mp;
    Test* ptr;
    for (int j = 0; j < 10000; ++j) {
      for (int i = 0; i < 10000; ++i) {
        ptr = mp.New();
        mp.Delete(ptr);
      }
    }
  }
}

