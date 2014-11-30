#ifndef VEE_COND_H
#define VEE_COND_H
#include <boost/thread.hpp>

class Condition {
private:
    ScriptState             *state;
    boost::mutex            stateLock; 
    bool                    cachedLoad;
    VexExecutionStatePtr    veeState;
public:
    Condition() { }
    Condition(std::string s, TargetArch t);
    ~Condition(void);

    void runPreVee(VexExecutionStatePtr vss);
    bool runPostVee(VexExecutionStatePtr vss);
    bool loaded(void);
    VexExecutionStatePtr getState(void) { return this->veeState; }
};

Condition *getConditionsFromFile(std::string fn, TargetArch t);
#endif
