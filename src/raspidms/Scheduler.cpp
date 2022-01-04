#include "Scheduler.h"

#include <iostream>
#include <limits>

#include "Utils.h"


Scheduler::Scheduler()
    : m_funcMap(),
      m_idPQ([](id_timing_t left, id_timing_t right) { return left.first > right.first; }),
      m_threadPool(4,
                   std::bind(&Scheduler::timingCb,
                   this,
                   std::placeholders::_1,
                   std::placeholders::_2))
{

}

Scheduler::~Scheduler() {
    m_threadPool.stop();
}

long Scheduler::addFunc(SchedFunc func, double priority) {
    long id = getUniqueId();
    m_funcMap.insert({id, {func, priority, 0.}});
    m_idPQ.push({0., id});

    return id;
}

bool Scheduler::triggerFunc(long id) {
    if (m_funcMap.count(id) == 0)
        return false;

    m_threadPool.push(id, std::ref(m_funcMap[id].func));

    return true;
}

bool Scheduler::removeFunc(long id) {
    auto elemIt = m_funcMap.find(id);

    if (elemIt == m_funcMap.end())
        return false;

    m_funcMap.erase(elemIt);
    return true;
}

void Scheduler::timingCb(long id, double time) {
    // No need to mutex protect, threadPool protects the call of this callback already

    //std::cout << __FUNCTION__ << " " << id << " " << time << std::endl;

    if (m_funcMap.count(id) == 0) {
        // this func doesn't come from us
        return;
    }

    SchedFuncPack& pack = m_funcMap[id];
    pack.time_acc += time * pack.priority;

    m_idPQ.push({pack.time_acc, id});
}

void Scheduler::schedule() {
    if (m_threadPool.queue_size() > 0 || m_threadPool.n_idle() == 0)
        return;

    while (!m_idPQ.empty()) {
        id_timing_t nextIdTiming = m_idPQ.top();
        m_idPQ.pop();
        double time_acc = nextIdTiming.first;
        long id = nextIdTiming.second;

        // Tasks are inserted in the threadPool
        // by increasing time_acc order, meaning the task that has run the LESS so far
        // gets to be ran
        //
        // If the threadPool has no place left (i.e all its thread are actually running
        // or are about to run)
        // break from the loop
        //
        // Timing are updated at task completion, by the callback timingCb
        // This works most like linux CFS

        if (m_funcMap.count(id) == 0) {
            // should never happen
            abort();
        } else {
            SchedFuncPack& pack = m_funcMap[id];
            m_threadPool.push(id, std::ref(pack.func));
        }

        if (m_threadPool.size() - m_threadPool.n_idle() - m_threadPool.queue_size() <= 0)
            break;
    }

}
