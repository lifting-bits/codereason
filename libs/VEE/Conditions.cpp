#include "VEE.h"
#include <fstream>

using namespace std;
using namespace boost;

Condition::Condition(string s, TargetArch t) : cachedLoad(false) {
    //create a new LUA state
    this->state = initScript(s);
    this->veeState = VexExecutionStatePtr(new VexExecutionState(t));
    return;
}

Condition::~Condition(void) {
    finiScript(this->state);
    return;
}

void Condition::runPreVee(VexExecutionStatePtr vss) {
	boost::mutex::scoped_lock  l(this->stateLock);
    if( l ) {
        runPre(this->state, vss);
    }
    return;
}
bool Condition::runPostVee(VexExecutionStatePtr vss) {
    bool res = false;
	boost::mutex::scoped_lock  l(this->stateLock);
    if( l ) {
        runPost(this->state, vss, res);
    }
    return res;
}

Condition *getConditionsFromFile(string fn, TargetArch t) {
    //simpler now!
    return new Condition(fn, t);
}

bool Condition::loaded(void) {
	if( this->state != NULL ) {
		return true;
	}
	return false;
}
