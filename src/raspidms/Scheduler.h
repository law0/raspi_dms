#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <functional>
#include <deque>
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
     * @brief addFunc add a function func that will be called every "period" (in seconds) by this scheduler
     * func. Don't use priority for now
     * @param func
     * @param priority
     * @param period
     * @return the unique id of the function to call in triggerFunc
     */
    long addFunc(SchedFunc func, int priority = 1);

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
    struct SchedFuncPack {
        SchedFunc func;
        int priority;
        double time_acc;
    };

    /**
     * @brief timingCb get called back when func with given "id" has finished within "time"
     * @param id
     * @param time
     */
    void timingCb(long id, double time);

    // map of SchedFuncPack
    std::unordered_map<long /*id*/, SchedFuncPack> m_funcMap;

    // id sorted by decreasing runtime
    typedef std::pair<double /*time*/, long /*id*/> id_timing_t;
    std::priority_queue<id_timing_t, std::vector<id_timing_t>, std::function<bool(id_timing_t, id_timing_t)>> m_idPQ;

    ThreadPool m_threadPool;
};

#endif // SCHEDULER_H
