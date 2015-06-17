#include <BasicIR.h>

#include "VEEOps.h"

#include <map>
#include <boost/icl/interval.hpp>
#include <boost/icl/interval_map.hpp>

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

class VexExecutionEngine;

class BlockProvider : public boost::enable_shared_from_this<BlockProvider> { 
private:
    
public:
  BlockProvider() { return; }

	virtual BlockPtr getNextBlock(boost::uint64_t VA)=0;
};

typedef boost::shared_ptr<BlockProvider> BlockProviderPtr;

class MemLocation {
private:
    ConstantValue   v;
public:
    MemLocation() {
        this->v.valueIsKnown = false;
        return;
    }

    MemLocation(ConstantValue cv) : v(cv) {
        return;
    }

    ~MemLocation() {
        return;
    }

    ConstantValue get(void) { return this->v; }

    MemLocation operator+=(const MemLocation &rhs) {
        this->v = rhs.v;
        return *this;
    }

    MemLocation operator=(const MemLocation &rhs) {
        this->v = rhs.v;
        return *this;
    }

    bool operator==(const MemLocation &rhs) const {
        return false;
    }
};

typedef boost::icl::interval_map<boost::uint64_t, MemLocation>    addrMapT;

//this is thrown by VEE code to indicate that VEE doesn't know how to proceed
//catching this should terminate a VEE simulation of code
//it could also terminate the program to trap and debug
class StepErr {
private:
    std::string txt;
public:
    StepErr(std::string txt) {
        this->txt = txt;
        return;
    }

    std::string get(void) { return this->txt; }
};

//this is thrown by VEE while emulating
//it indicates that the code did "something bad"
//things like executing IN at an invalid CPL or doing a deref that would
//fault (this might be allowed)
class StepClientErr {
private:
    std::string txt;
public:
    StepClientErr(std::string t) : txt(t) { }

    std::string get(void) { return this->txt; }
};

struct RegConstant {
    uint32_t        originalOffset;
    uint32_t        originalWidth;
    ConstantValue   v;
};

typedef std::vector<RegConstant>  regMapT;

class VexExecutionState;
typedef boost::shared_ptr<VexExecutionState>    VexExecutionStatePtr;

class VexExecutionState : 
    public boost::enable_shared_from_this<VexExecutionState> {
private:
    ExitType                exitTy;
    addrMapT                memMap;
    regMapT                 regState;
    bool                    didCall;
    std::list<regMapT>      permutations; 
    TargetArch              arch;
public:
    static void setStateS(boost::uint32_t off, RegConstant v, regMapT &s);
    static ConstantValue getStateS(boost::uint32_t off, boost::uint32_t stride, regMapT &s);
    void setStateFromConst(unsigned long off, RegConstant v);
    void setMemFromConst(boost::uint64_t addr, ConstantValue v);
	ConstantValue getConstFromMem(boost::uint64_t addr, unsigned long stride);
    ConstantValue getConstFromState(unsigned long off, unsigned long stride);
    void setState(unsigned long , unsigned short , unsigned long long );
    void setMem(unsigned long , unsigned short , unsigned long long );
    std::string printRegState(void);
    std::string printMemState(void);
    ExitType getVEEExit(void) { return this->exitTy; }
    bool getDidCall(void) { return this->didCall; }
    void setVEEExit(ExitType t) { this->exitTy = t; }
    void setDidCall(bool b) { this->didCall = b; }
    TargetArch getArch(void) { return this->arch; }
   
    VexExecutionState(TargetArch);
    VexExecutionState(const VexExecutionState &n) { 
        this->memMap = n.memMap;
        this->regState = n.regState;
        this->didCall = false;
        this->exitTy = Fallthrough;
        this->arch = n.arch;
        this->permutations = n.permutations;
    }

    std::list<regMapT> getRegPermutations(void) {
        return this->permutations;
    }

    void addRegPermutations(std::list<regMapT> k);
    std::list<VexExecutionStatePtr> getPermutations(BlockPtr);
};


#include "VEElua.h"
#include "Conditions.h"

class VexExecutionEngine {
private:
    void stepStmt(StatementPtr s);
    ConstantValue stepOp(ExOpPtr op);
    ConstantValue doLoad(ExLoadPtr load);
    ConstantValue stepCCall(ExCCallPtr cc);
    void stepBlockExit(ExpressionPtr exit);
    void doDirtyStep(StDirtyPtr dirty);
    void doCAS(StCASPtr cas);
    void doLLSC(StLLSCPtr );
    ConstantValue doExMux0X(ExMux0XPtr mux);

    ExpressionPtr                       nextBlock;
    bool                                steppedExit;
    VexExecutionStatePtr                state;
    std::vector<StatementPtr>::iterator curStmt;
    BlockPtr                            block;
    BlockProviderPtr                    blockProvider;
public:
    enum StepState {
        StepOK, //we can call step() again
        StepEnd, //we can't call step() again, the EE is over
        StepERR, //we can't call step() again, EE is over due to err in VEE
        StepClientERR //EE is over due to err in generated code
    };
    VexExecutionEngine(BlockProviderPtr p, VexExecutionStatePtr vss);
    VexExecutionEngine(BlockProviderPtr p, FlowPtr f);
    VexExecutionEngine(BlockProviderPtr p, BlockPtr b, VexExecutionStatePtr ves);

    StatementPtr getCurStmt(void) { return *this->curStmt; } 
    ExpressionPtr   getNext(void) { return this->nextBlock; }
    StepState step();
    std::string printState(void);
    VexExecutionStatePtr getState(void) { return this->state; }
};

#include "VEEHelpers.h"
