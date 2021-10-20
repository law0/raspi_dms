#ifndef ISTAGE_H
#define ISTAGE_H

class IStage {
public:
    virtual ~IStage() {}

    /**
     * @brief operator ()
     * to be called by the Scheduler/ThreadPool. The whole object must be pushable
     * as threadPool.push(std::ref(object))
     * This function must purposely terminates, and so needs to be repushed regularly
     */
    virtual void operator()(int) = 0;

    /**
     * @brief averageTime
     * @return the averageTime (in seconds) it takes to complete this stage
     */
    virtual double averageTime() = 0;
};

#endif // ISTAGE_H
