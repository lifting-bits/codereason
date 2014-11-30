#include <gtest/gtest.h>
#include <getExec.h>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <VEE.h>
#include <decodeLib.h>

extern "C" {
#include <libvex_basictypes.h>
#include <libvex_guest_offsets.h>
#include <libvex_emwarn.h>
#include <libvex.h>
#include <libvex_guest_x86.h>
}

using namespace boost;
using namespace std;

/* utilities */
void *getX86DecodeLib(void) {
    TargetArch  tarch = {X86};
    TargetInfo  ti;
    ti.maxInstructions = 99;
    ti.chaseThreshold = 10;
    ti.opLevel = FullOpt;
    ti.tarch = tarch;
    if( tarch.ta == X86 ) {
        ti.guestHWcaps =
            VEX_HWCAPS_X86_SSE1|VEX_HWCAPS_X86_SSE2|VEX_HWCAPS_X86_SSE3;
        ti.hostHWcaps =
            VEX_HWCAPS_X86_SSE1|VEX_HWCAPS_X86_SSE2|VEX_HWCAPS_X86_SSE3;
    } else if( tarch.ta == ARM ) {
        ti.guestHWcaps = 7|VEX_HWCAPS_ARM_VFP3;
        ti.hostHWcaps =
            VEX_HWCAPS_X86_SSE1|VEX_HWCAPS_X86_SSE2|VEX_HWCAPS_X86_SSE3;
    }else {
        ti.guestHWcaps = 0;
        ti.hostHWcaps = 0;
    }

    return initDecodeLib(ti, true, true);
}

class BlankBlockProvider : public BlockProvider {
    private:
    public:
        BlankBlockProvider() { return; }
        virtual BlockPtr getNextBlock(uint64_t VA);
};

typedef boost::shared_ptr<BlankBlockProvider> BlankBlockProviderPtr;

BlockPtr BlankBlockProvider::getNextBlock(uint64_t VA) {
    BlockPtr    b;

    return b;
}

bool nextIsKnown(VexExecutionEngine &vee, ConstantValue &next) {
    ExpressionPtr   e = vee.getNext();
    ConstantValue   cn;

    cn.valueIsKnown = false;

    if(ExRdTmpPtr r = dynamic_pointer_cast<ExRdTmp>(e) ) {
        cn = r->getTmp()->getVal(); 
    }

    if( ExConstPtr r = dynamic_pointer_cast<ExConst>(e) ) {
        cn = r->getVal();
    }

    if( cn.valueIsKnown ) {
        next = cn;
    }

    return cn.valueIsKnown;
}

VexExecutionEngine::StepState
runBlobX86(uint8_t *buf, size_t len, VexExecutionStatePtr vss) {
    void *ctx = getX86DecodeLib(); 

    EXPECT_NE(((void*)0), ctx); 

    //decode a block
    BlockPtr    b;
    TargetArch  ta = {X86};
    bool r = convertToOneBlock( ctx,
                                buf,
                                len,
                                0x1000,
                                ta,
                                b);

    EXPECT_EQ(true, r);
    //set up a translation context for the block
    BlankBlockProviderPtr bpp =
        BlankBlockProviderPtr(new BlankBlockProvider());

    VexExecutionEngine  vee(bpp, b, vss);
    //step the block until exit
    VexExecutionEngine::StepState   ss;
    ConstantValue                   next;
    while( true ) {
        bool                            over=false;

        ss = vee.step();
        switch(ss) {
            case VexExecutionEngine::StepOK:
                break;
            case VexExecutionEngine::StepEnd:
                //can we get the next target of this block?
                if( nextIsKnown(vee, next) ) {
                    //ptr targets should be 32-bits on x86
                    EXPECT_EQ(32, next.width);
                    //lets decode a block at that position
                    uint32_t    base = next.U32-0x1000;
                    BlockPtr    newB;
                    r = convertToOneBlock(  ctx,
                                            (buf+base),
                                            (len-base),
                                            base,
                                            ta,
                                            newB);
                    if( r ) {
                        vee = VexExecutionEngine(bpp, newB, vss);
                    } else {
                        over = true;
                    }
                } else {
                    over = true;
                }
                break;
            case VexExecutionEngine::StepERR:
            case VexExecutionEngine::StepClientERR:
                over = true;
                break;
        }

        if( over ) {
            break;
        }
    }

    finiDecodeLib(ctx);
    return ss;
}

/* lets have some simple instruction decoding test cases */

uint8_t testAddBuf[] = {    
0x89, 0xC8, //      mov eax,ecx
0x83, 0xC0, 0x01, //add eax,byte +0x1
0xC3 //             ret
};

TEST(Decode, testAddX86) {
    void *ctx = getX86DecodeLib(); 

    EXPECT_NE(((void*)0), ctx); 

    //decode a block
    BlockPtr    b;
    TargetArch  ta = {X86};
    bool r = convertToOneBlock( ctx,
                                testAddBuf,
                                sizeof(testAddBuf),
                                0x1000,
                                ta,
                                b);

    EXPECT_EQ(true, r);
    //test and see if this block has the properties we look for
    EXPECT_EQ(6, b->getBlockLen());

    finiDecodeLib(ctx);
}

uint8_t testMemBuf[] = {
    0x8b, 0x01, // mov eax, [ecx]
    0xc3        // ret
};

TEST(Decode, testMemX86) {
    void *ctx = getX86DecodeLib();

    EXPECT_NE(((void*)0), ctx);

    BlockPtr    b;
    TargetArch  ta = {X86};

    bool r = convertToOneBlock( ctx,
                                testMemBuf,
                                sizeof(testMemBuf),
                                0x1000,
                                ta,
                                b);

    EXPECT_EQ(true, r);

    EXPECT_EQ(3, b->getBlockLen());

    finiDecodeLib(ctx);
}

uint8_t testAndBuf[] = {
    0x89, 0xC8,//       mov eax, ecx
    0x83, 0xE0, 0x08,// and eax, byte +0x8
    0xC3//              ret 
};

TEST(Decode, testAndX86) {
    void *ctx = getX86DecodeLib(); 

    EXPECT_NE(((void*)0), ctx); 

    //decode a block
    BlockPtr    b;
    TargetArch  ta = {X86};
    bool r = convertToOneBlock( ctx,
                                testAndBuf,
                                sizeof(testAndBuf),
                                0x1000,
                                ta,
                                b);

    EXPECT_EQ(true, r);
    //test and see if this block has the properties we look for
    EXPECT_EQ(6, b->getBlockLen());

    finiDecodeLib(ctx);
}

/* some emulation test cases from the same data with different inputs */

TEST(Concolic, testConcolicAdd) {
    TargetArch  ta = {X86};
    VexExecutionStatePtr    vss = 
        VexExecutionStatePtr( new VexExecutionState(ta));

    ConstantValue   cv;
    cv.valueIsKnown = true;
    cv.width = 32;
    cv.U32 = 2;
    RegConstant rc = {4, 32, cv};
    vss->setStateFromConst(4, rc);

    VexExecutionEngine::StepState   ss = 
        runBlobX86(testAddBuf, sizeof(testAddBuf), vss);
    //test that we left with state OK
    EXPECT_EQ( VexExecutionEngine::StepEnd, ss);

    //test that the postconditions are sane
    ConstantValue   v = vss->getConstFromState(0, 32);

    EXPECT_EQ(true, v.valueIsKnown);
    EXPECT_EQ(32, v.width);
    EXPECT_EQ(3, v.U32);
}

/*TEST(Concolic, testConcolicListAdd) {

}*/

TEST(Concolic, testConcolicAnd) {
    TargetArch  ta = {X86};
    VexExecutionStatePtr    vss = 
        VexExecutionStatePtr( new VexExecutionState(ta));

    ConstantValue   cv;
    cv.valueIsKnown = true;
    cv.width = 32;
    cv.U32 = 8;
    RegConstant rc = {4, 32, cv};
    vss->setStateFromConst(4, rc);

    VexExecutionEngine::StepState   ss = 
        runBlobX86(testAndBuf, sizeof(testAndBuf), vss);
    //test that we left with state OK
    EXPECT_EQ( VexExecutionEngine::StepEnd, ss);

    //test that the postconditions are sane
    ConstantValue   v = vss->getConstFromState(0, 32);

    EXPECT_EQ(true, v.valueIsKnown);
    EXPECT_EQ(32, v.width);
    EXPECT_EQ(8, v.U32);
}

/* some multi-arch test cases */

/* test the VSS infrastructure independently */
TEST(VSS, RegsT1) {
    TargetArch  ta = {X86};

    VexExecutionStatePtr    vss = 
        VexExecutionStatePtr(new VexExecutionState(ta));

    ConstantValue   t0;
    t0.valueIsKnown = true;
    t0.width = 32;
    t0.U32 = 80000000;
    RegConstant rc = {0, 32, t0};

    vss->setStateFromConst(0, rc);

    ConstantValue t1 = vss->getConstFromState(0, 32);

    EXPECT_EQ(true, t1.valueIsKnown);
    EXPECT_EQ(32, t1.width);
    EXPECT_EQ(80000000, t1.U32);
}

TEST(VSS, RegsT2) {
    TargetArch  ta = {X86};

    VexExecutionStatePtr    vss = 
        VexExecutionStatePtr(new VexExecutionState(ta));

    ConstantValue   t0;
    t0.valueIsKnown = true;
    t0.width = 32;
    t0.U32 = 80000000;
    RegConstant rc = {0, 32, t0};

    vss->setStateFromConst(0, rc);
    ConstantValue t2;
    t2.valueIsKnown = false;
    t2.width = 8;
    RegConstant rc2 = {0, 8, t2};
    vss->setStateFromConst(0, rc2);

    ConstantValue t1 = vss->getConstFromState(0, 32);

    EXPECT_FALSE(t1.valueIsKnown);
}

TEST(VSS, MemT1) {
    TargetArch  ta = {X86};

    VexExecutionStatePtr    vss =
        VexExecutionStatePtr(new VexExecutionState(ta));

    ConstantValue   t0;
    t0.valueIsKnown = true;
    t0.width = 8;
    t0.U8 = 0x80;

    vss->setMemFromConst(0x4000, t0);
    vss->setMemFromConst(0x4001, t0);
    vss->setMemFromConst(0x4002, t0);
    vss->setMemFromConst(0x4003, t0);

    ConstantValue   t1 = vss->getConstFromMem(0x4000, 1);

    EXPECT_EQ(true, t1.valueIsKnown);
    EXPECT_EQ(8, t1.width);
    EXPECT_EQ(0x80, t1.U8);

    ConstantValue   t2 = vss->getConstFromMem(0x4000, 4);

    EXPECT_EQ(true, t2.valueIsKnown);
    EXPECT_EQ(32, t2.width);
    EXPECT_EQ(0x80808080, t2.U32);

    ConstantValue   t3 = vss->getConstFromMem(0x1000, 4);

    EXPECT_FALSE(t3.valueIsKnown);
}

TEST(VSS, MemT2) {
    TargetArch  ta = {X86};

    VexExecutionStatePtr    vss =
        VexExecutionStatePtr(new VexExecutionState(ta));
    
    ConstantValue   t0;

    t0.valueIsKnown = true;
    t0.width = 32;
    t0.U32 = 0x40000000;

    vss->setMemFromConst(0x8000, t0);

    ConstantValue   t1 = vss->getConstFromMem(0x8002, 1);

    EXPECT_EQ(true, t1.valueIsKnown);
    EXPECT_EQ(8, t1.width);
    EXPECT_EQ(0, t1.U8);

    ConstantValue   t2;

    t2.valueIsKnown = false;
    t2.width = 8;

    vss->setMemFromConst(0x8002, t2);

    ConstantValue   t3 = vss->getConstFromMem(0x8000, 4);

    EXPECT_FALSE(t3.valueIsKnown);
}

TEST(VSS, Permutations) {
    TargetArch          ta = {X86};
    VexExecutionState   *ves = new VexExecutionState(ta);
    list<regMapT>       regPerms;

    EXPECT_NE(((void*)0), ves);

    uint32_t    regs[] = {  OFFSET_x86_EAX, 
                            OFFSET_x86_EBX,
                            OFFSET_x86_ECX
                            /*OFFSET_x86_EDX,
                            OFFSET_x86_EDI,
                            OFFSET_x86_ESI,
                            OFFSET_x86_EBP};    */ };

    for( int i = 0; i < sizeof(regs)/sizeof(uint32_t); i++ ) {
        uint32_t        reg = regs[i];
        ConstantValue   cv;
        regMapT         blank;

        cv.valueIsKnown = true;
        cv.width = 32;
        cv.U32 = 0x1;
        RegConstant rc = {reg, 32, cv};

        blank.resize(offsetof(VexGuestX86State, padding1));

        VexExecutionState::setStateS(reg, rc, blank);

        regPerms.push_back(blank);
    }

    ves->addRegPermutations(regPerms);

    list<VexExecutionStatePtr>  perms = ves->getPermutations(BlockPtr());

    EXPECT_EQ(3, perms.size());

    list<regMapT>   regPerms2;
    for( int i = 0; i < sizeof(regs)/sizeof(uint32_t); i++ ) {
        uint32_t        reg = regs[i];
        ConstantValue   cv;
        regMapT         blank;

        cv.valueIsKnown = true;
        cv.width = 32;
        cv.U32 = 0x1;
        RegConstant rc = {reg, 32, cv};

        blank.resize(offsetof(VexGuestX86State, padding1));

        VexExecutionState::setStateS(reg, rc, blank);

        regPerms2.push_back(blank);
    }

    ves->addRegPermutations(regPerms2);

    list<VexExecutionStatePtr>  perms2 = ves->getPermutations(BlockPtr());
    EXPECT_EQ(6, perms2.size());
   
    delete ves;
}

const char *s1 = "function onPre(v)\n"
"    vee.putreg(v, ESI, 32, 80808080)\n"
"end\n"
"function onPost(v)\n"
"    eip = vee.getreg(v, EIP, 32)\n"
"    if eip == nil then\n"
"        return false\n"
"    end\n"
"    if eip == 80808080 then\n"
"        return true\n"
"    end\n"
"    return false\n"
"end";

TEST(Script, InitScript) {
    std::string  scriptSrc(s1);
    
    ScriptState *s = initScriptFromString(scriptSrc);

    EXPECT_NE(((void*)0), s);

    free(s);
}

TEST(Script, ScriptAbstrReg) {
    //I can test that the whole combinations thing works the way that 
    //I think it should
    TargetArch          ta = {X86};
    VexExecutionState   *ves = new VexExecutionState(ta);
    list<regMapT>       regPerms;

    EXPECT_NE(((void*)0), ves);

    uint32_t    regs[] = {OFFSET_x86_EAX, OFFSET_x86_EBX};    

    for( int i = 0; i < sizeof(regs)/sizeof(uint32_t); i++ ) {
        uint32_t        reg = regs[i];
        ConstantValue   cv;
        regMapT         blank;

        cv.valueIsKnown = true;
        cv.width = 32;
        cv.U32 = 0x42424242;
        RegConstant rc = {reg, 32, cv};

        blank.resize(offsetof(VexGuestX86State, padding1));

        VexExecutionState::setStateS(reg, rc, blank);

        regPerms.push_back(blank);
    }

    ves->addRegPermutations(regPerms);

    BlockPtr    b = BlockPtr(); //TODO: create a block for these tests
    list<VexExecutionStatePtr>  perms = ves->getPermutations(b);

    EXPECT_EQ(2, perms.size());

    regMapT n1;
    list<regMapT>   l1;

    n1.resize(offsetof(VexGuestX86State, padding1));

    ConstantValue   c1;

    c1.valueIsKnown = true;
    c1.width = 32;
    c1.U32 = 0x81818181;
    RegConstant spr = {OFFSET_x86_ESI, 32, c1};

    VexExecutionState::setStateS(OFFSET_x86_ESI, spr, n1);

    l1.push_back(n1);
    ves->addRegPermutations(l1);

    list<VexExecutionStatePtr>  perms2 = ves->getPermutations(b);

    EXPECT_EQ(2, perms2.size());
    //check that in these 2 permutations that ESI&EAX, ESI&EBX
    list<VexExecutionStatePtr>::iterator it = perms2.begin();
    VexExecutionStatePtr    k1 = *it;
    ++it;
    VexExecutionStatePtr    k2 = *it;
    string outs = k1->printRegState();
    EXPECT_STREQ("State 0 66\nState 1 66\nState 2 66\nState 3 66\n"
                "State 24 129\nState 25 129\nState 26 129\nState 27 129\n",
                outs.c_str());

    delete ves;
}

TEST(Misc, RegMap1) {
    map<Register, ExpressionPtr>    exprMap; 

    ConstantValue   c1;

    c1.valueIsKnown = true;
    c1.width = 32;
    c1.valueType = ConstantValue::T_I32;
    c1.U32 = 0x31337;

    ExpressionPtr   e1 = ExpressionPtr(new ExConst(c1));

    TargetArch           t;
    t.ta = X86;

    Register        r1;
    r1.arch = t;
    r1.regclass = GenericRegister;
    r1.width = 32;
    r1.Reg32 = EAX;

    pair<map<Register, ExpressionPtr>::iterator, bool>  insertRes;

    EXPECT_STREQ("I:U32(0x31337)", e1->printExpr().c_str());
    insertRes = exprMap.insert(pair<Register, ExpressionPtr>(r1, e1));
    EXPECT_EQ(true, insertRes.second);

    ConstantValue   c2;

    c2.valueIsKnown = true;
    c2.width = 32;
    c2.valueType = ConstantValue::T_I32;
    c2.U32 = 8;
   
    ExpressionPtr   e2 = ExpressionPtr(new ExConst(c2));
    
    Register        r2;
    r2.arch = t;
    r2.regclass = GenericRegister;
    r2.width = 32;
    r2.Reg32 = EBX;

    insertRes = exprMap.insert(pair<Register, ExpressionPtr>(r2, e2));
    EXPECT_EQ(true, insertRes.second);

    //inserts work, how about lookup?
    Register    lr1;

    lr1.arch = t;
    lr1.regclass = GenericRegister;
    lr1.width = 32;
    lr1.Reg32 = EAX;

    map<Register, ExpressionPtr>::iterator  lookupRes;

    lookupRes = exprMap.find(lr1);
    EXPECT_NE(exprMap.end(), lookupRes);
    //so we found it, did we find the right expression?
    ExpressionPtr   foundExpr = (*lookupRes).second;
    EXPECT_STREQ("I:U32(0x31337)", foundExpr->printExpr().c_str());

    Register    lr2;

    lr2.arch = t;
    lr2.regclass = GenericRegister;
    lr2.width = 32;
    lr2.Reg32 = EBX;

    lookupRes = exprMap.find(lr2);
    EXPECT_NE(exprMap.end(), lookupRes);
    //so we found it, did we find the right expression?
    foundExpr = (*lookupRes).second;
    EXPECT_STREQ("I:U32(0x8)", foundExpr->printExpr().c_str());

    //if we look up something not set, do we miss?
    Register    lr3;

    lr3.arch = t;
    lr3.regclass = GenericRegister;
    lr3.width = 32;
    lr3.Reg32 = EDI;
    
    lookupRes = exprMap.find(lr3);
    EXPECT_EQ(exprMap.end(), lookupRes);
}
