#include "Scheduler.h"

#include <iostream>
#include <limits>

#include "Utils.h"


Scheduler::Scheduler() : m_funcMap(), m_threadPool(std::thread::hardware_concurrency())
{

}

Scheduler::~Scheduler() {
    m_threadPool.stop();
}

long Scheduler::addFunc(SchedFunc func, double maxTime, double minTime) {
    if (minTime > maxTime) {
        std::cerr << __func__ << ":"
                  << __LINE__ << "error : maxTime "
                  << maxTime << "must be greater or equal than minTime "
                  << minTime << std::endl;
        return LONG_MIN;
    }
    long id = getUniqueId();
    m_funcMap.insert({id, {func, maxTime, minTime}});

    auto it = m_timeIdList.begin();

    while (it != m_timeIdList.end() && maxTime <= it->first) { ++it; }

    m_timeIdList.insert(it, {maxTime, id});

    return id;
}

bool Scheduler::triggerFunc(long id) {
    if (m_funcMap.count(id) == 0)
        return false;

    if (m_funcMap[id].minTime < timeMark(id, false)) {
        if (m_threadPool.n_idle() > 0)
            m_threadPool.push(std::ref(m_funcMap[id].func));
        else
            return false;
    }

    return true;
}

bool Scheduler::removeFunc(long id) {
    auto elemIt = m_funcMap.find(id);

    if (elemIt == m_funcMap.end())
        return false;

    m_funcMap.erase(elemIt);
    return true;
}

void Scheduler::schedule() {
    if (m_threadPool.queue_size() > 0)
        return;

    long id;
    for (auto& pTimeId : m_timeIdList) {
        id = pTimeId.second;
        if (m_funcMap.count(id) == 0) {
            // should never happen
            abort();
        } else {
            SchedFuncPack& pack = m_funcMap[id];
            if (pack.maxTime < timeMark(id, false)) {
                std::cout << "Pushing maxTime=" << pack.maxTime << std::endl;
                m_threadPool.push(std::ref(pack.func));
                timeMark(id);

                // Tasks are inserted in the threadPool
                // by decreasing maxTime order, because m_timeIdList is sorted that way
                // (meaning greater maxTime are inserted first)
                //
                // This is to ensure that task that takes more time to complete
                // are inserted, because the threadPool could be spammed/filled with
                // short tasks otherwise.
                //
                // If the threadPool has no place left (i.e all its thread are actually running)
                // break from the loop

                if (m_threadPool.n_idle() == 0)
                    break;
            }
        }
    }
}
