#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <functional>
#include <list>
#include <unordered_map>
#include <tuple>

#include "ThreadPool.h"

typedef std::function<void(int)> SchedFunc;

class Scheduler
{
public:
    Scheduler();
    ~Scheduler();

    /**
     * @brief addFunc add a function func that will be called every maxTime by this scheduler
     * func. Can also be called upon call of triggerFunc below, unless minTime has not elapsed
     * @param func
     * @param maxTime
     * @param minTime
     * @return the unique id of the function to call in triggerFunc
     */
    long addFunc(SchedFunc func, double maxTime, double minTime);

    /**
     * @brief triggerFunc, given the right id, called immediately the mapped function,
     * unless minTime has not elapsed (see addFunc)
     * @param id
     * @return true if function has been called, false otherwise
     */
    bool triggerFunc(long id);

    /**
     * @brief removeFunc, given the right id, remove the mapped function from the list
     * of function to be called
     * @param id
     * @return true if a function has been found and remove, false otherwise
     */
    bool removeFunc(long id);

    /**
     * @brief schedule, need to be called periodically for this Scheduler to work
     */
    void schedule();

private:
    typedef struct SchedFuncPack_t {
        SchedFunc func;
        double maxTime;
        double minTime;
    } SchedFuncPack;

    // map of SchedFuncPack
    std::unordered_map<long /*id*/, SchedFuncPack> m_funcMap;

    // id sorted by decreasing max time
    std::list<std::pair<double /*maxTime*/, long /*id*/>> m_timeIdList;
    ThreadPool m_threadPool;
};

#endif // SCHEDULER_H