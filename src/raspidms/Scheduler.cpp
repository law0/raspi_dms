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
    long id;
    for (auto& pTimeId : m_timeIdList) {
        id = pTimeId.second;
        if (m_funcMap.count(id) == 0) {
            // should never happen
            abort();
        } else {
            SchedFuncPack& pack = m_funcMap[id];
            if (pack.maxTime < timeMark(id, false)) {
                if (m_threadPool.queue_size() < 2) { // let's charge a little
                    std::cout << "Pushing maxTime=" << pack.maxTime << std::endl;
                    m_threadPool.push(std::ref(pack.func));
                    timeMark(id);
                } else {
                    // m_timeIdVec being sorted by decreasing time
                    // it makes bigger maxTime prioritary
                    // (if bigger maxTime can't run, then lesser maxTime are not ran
                    // to avoid fill the all the thread)
                    break;
                }
            }
        }
    }
}
