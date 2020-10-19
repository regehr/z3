/*++
Copyright (c) 2011 Microsoft Corporation

Module Name:

    scoped_timer.cpp

Abstract:

    <abstract>

Author:

    Leonardo de Moura (leonardo) 2011-04-26.

Revision History:

    Nuno Lopes (nlopes) 2019-02-04  - use C++11 goodies

--*/

#include "util/scoped_timer.h"
#include "util/util.h"
#include <chrono>
#include <climits>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

struct pool_entry {
  std::thread m_thread;
  unsigned ms;
  event_handler * eh;
  std::timed_mutex m_mutex;
  std::unique_lock<std::mutex> block;
  std::condition_variable cv;
};

static std::vector<pool_entry *> free_threads;
static std::mutex m;
static int thread_cnt = 0;

struct scoped_timer::imp {
private:
    pool_entry * pe;

    static void thread_func(pool_entry *pe) {
        while (1) {
            auto end = std::chrono::steady_clock::now() + std::chrono::milliseconds(pe->ms);

            while (!pe->m_mutex.try_lock_until(end)) {
                if (std::chrono::steady_clock::now() >= end) {
                    pe->eh->operator()(TIMEOUT_EH_CALLER);
                    return;
                }
            }

            pe->m_mutex.unlock();
            pe->cv.wait(pe->block);
        }
    }

public:
    imp(unsigned ms, event_handler * eh) {
        m.lock();
        if (free_threads.empty()) {
          pe = new pool_entry { std::thread(), ms, eh };
          if (1) {
            thread_cnt++;
            std::cerr << "created a new thread, there are " << thread_cnt << " now\n";
          }
          m.unlock();
          pe->m_mutex.lock();
          pe->m_thread = std::thread(thread_func, pe);
        } else {
          pe = free_threads.back();
          free_threads.pop_back();
          m.unlock();
          pe->ms = ms;
          pe->eh = eh;
          pe->m_mutex.lock();
          pe->cv.notify_one();
        }
    }

    ~imp() {
      pe->m_mutex.unlock();
      m.lock();
      free_threads.push_back(pe);
      m.unlock();
    }
};

scoped_timer::scoped_timer(unsigned ms, event_handler * eh) {
    if (ms != UINT_MAX && ms != 0)
        m_imp = alloc(imp, ms, eh);
    else
        m_imp = nullptr;
}
    
scoped_timer::~scoped_timer() {
    dealloc(m_imp);
}
