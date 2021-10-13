#include "Scheduler.h"

#include <iostream>
#include <limits>

#include "Utils.h"


Scheduler::Scheduler() : m_map(), m_threadPool(std::thread::hardware_concurrency())
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
    m_map.insert({id, {func, maxTime, minTime}});
    return id;
}

bool Scheduler::triggerFunc(long id) {
    if (m_map.count(id) == 0)
        return false;

    if (m_map[id].minTime < timeMark(id, false)) {
        if (m_threadPool.n_idle() > 0)
            m_threadPool.push(std::ref(m_map[id].func));
        else
            return false;
    }

    return true;
}

bool Scheduler::removeFunc(long id) {
    auto elemIt = m_map.find(id);

    if (elemIt == m_map.end())
        return false;

    m_map.erase(elemIt);
    return true;
}

void Scheduler::schedule() {
    for(auto& pairIt : m_map) {
        long id = pairIt.first;
        SchedFuncPack& pack = pairIt.second;
        double time = timeMark(id, false);
        if (pack.minTime < time && pack.maxTime < time) {
            if (m_threadPool.n_idle() > 0) {
                m_threadPool.push(std::ref(pack.func));
                timeMark(id);
            }
        }
    }
}
