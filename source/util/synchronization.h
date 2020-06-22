#ifndef __SYNCHRONIZATION_H__
#define __SYNCHRONIZATION_H__

#include "knativesynchronization.h"

class BoxedWineCondition;

class BoxedWineConditionChild {
public:
    BoxedWineConditionChild(BoxedWineCondition* cond, const std::function<void(void)>& doneWaitingCallback) : cond(cond), doneWaitingCallback(doneWaitingCallback) {}
    BoxedWineCondition* cond;
    std::function<void(void)> doneWaitingCallback;
};

#ifdef BOXEDWINE_MULTI_THREADED
class BoxedWineMutex {
public:
    void lock();
    void unlock();

private:
    KNativeMutex m;
};

class BoxedWineCriticalSection {
public:
    BoxedWineCriticalSection(BoxedWineMutex* mutex);
    ~BoxedWineCriticalSection();

private:
    BoxedWineMutex* mutex;
};

class BoxedWineCondition {
public:
    BoxedWineCondition(std::string name);
    BoxedWineCondition();

    bool tryLock();
    void lock();
    void signal();
    void signalAll();
    void signalAllLock();
    void wait();
    void waitWithTimeout(U32 ms);
    void unlock();
    void addChildCondition(BoxedWineCondition& cond, const std::function<void(void)>& doneWaitingCallback);
    void unlockAndRemoveChildren();
    U32 waitCount();

    const std::string name;

private:
    std::vector<BoxedWineCondition*> parents;
    std::vector<BoxedWineConditionChild> children;  

    KNativeMutex m;
    KNativeCondition c;
    U32 lockOwner;
};

class BoxedWineCriticalSectionCond {
public:
    BoxedWineCriticalSectionCond(BoxedWineCondition* cond);
    ~BoxedWineCriticalSectionCond();

private:
    BoxedWineCondition* cond;
};

#define BOXEDWINE_CRITICAL_SECTION static BoxedWineMutex csMutex; BoxedWineCriticalSection boxedWineCriticalSection(&csMutex);
#define BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(csMutex) BoxedWineCriticalSection boxedWineCriticalSection(&csMutex);
#define BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(csCond) BoxedWineCriticalSectionCond boxedWineCriticalSection(&csCond);

#define BOXEDWINE_MUTEX BoxedWineMutex
#define BOXEDWINE_MUTEX_LOCK(mutex) mutex.lock()
#define BOXEDWINE_MUTEX_UNLOCK(mutex) mutex.unlock()

#define BOXEDWINE_CONDITION BoxedWineCondition
#define BOXEDWINE_CONDITION_LOCK(cond) cond.lock()
#define BOXEDWINE_CONDITION_UNLOCK(cond) cond.unlock()
#define BOXEDWINE_CONDITION_SIGNAL(cond) cond.signal()
#define BOXEDWINE_CONDITION_SIGNAL_ALL(cond) cond.signalAll()
#define BOXEDWINE_CONDITION_WAIT(cond) cond.wait()
#define BOXEDWINE_CONDITION_WAIT_TIMEOUT(cond, t) cond.waitWithTimeout(t)
#define BOXEDWINE_CONDITION_WAIT_TYPE(x, type) cond.wait()
#define BOXEDWINE_CONDITION_ADD_CHILD_CONDITION(parent, cond, doneWaitingCallback) (parent).addChildCondition(cond, doneWaitingCallback)
#define BOXEDWINE_CONDITION_SIGNAL_ALL_NEED_LOCK(cond) (cond).signalAllLock()

#define BoxedWineConditionTimer BoxedWineCondition
#else
#define BOXEDWINE_CRITICAL_SECTION
#define BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(csMutex)
#define BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(csCond)

typedef void* BOXEDWINE_MUTEX;
#define BOXEDWINE_MUTEX_LOCK(mutex)
#define BOXEDWINE_MUTEX_UNLOCK(mutex)

class KThread;
class BoxedWineCondition;

class BoxedWineConditionTimer : public KTimer { 
public:
    BoxedWineConditionTimer() : cond(0){};
    bool run();

    BoxedWineCondition* cond;
};

class BoxedWineCondition {
public:
    BoxedWineCondition(std::string name);
    ~BoxedWineCondition();

    void signal();
    void signalAll();
    U32 wait();
    U32 waitWithTimeout(U32 ms);
    U32 waitCount();

    void addChildCondition(BoxedWineCondition& cond, const std::function<void(void)>& doneWaitingCallback);
    void unlockAndRemoveChildren();

    const std::string name;
private:
    KList<KThread*> waitingThreads;

    std::vector<BoxedWineCondition*> parents;
    std::vector<BoxedWineConditionChild> children;

    friend BoxedWineConditionTimer;
    void signalThread(bool all);
};

typedef BoxedWineCondition BOXEDWINE_CONDITION;
#define BOXEDWINE_CONDITION_LOCK(cond)
#define BOXEDWINE_CONDITION_UNLOCK(cond)
#define BOXEDWINE_CONDITION_SIGNAL(cond) (cond).signal()
#define BOXEDWINE_CONDITION_SIGNAL_ALL(cond) (cond).signalAll()
#define BOXEDWINE_CONDITION_SIGNAL_ALL_NEED_LOCK(cond) (cond).signalAll()
#define BOXEDWINE_CONDITION_WAIT(cond) return (cond).wait()
#define BOXEDWINE_CONDITION_WAIT_TIMEOUT(cond, ms) return (cond).waitWithTimeout(ms)
#define BOXEDWINE_CONDITION_ADD_CHILD_CONDITION(parent, cond, doneWaitingCallback) (parent).addChildCondition(cond, doneWaitingCallback)

#endif

#endif