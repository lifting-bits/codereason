#include "VEE.h"

#include <boost/shared_ptr.hpp>
#include <iostream>

using namespace boost;
using namespace std;

extern "C" {
#include <libvex_basictypes.h>
#include <libvex_guest_offsets.h>
#include <libvex_emnote.h>
#include <libvex.h>
#include <libvex_guest_x86.h>
#include <libvex_guest_arm.h>
#include <libvex_guest_amd64.h>
}

#include <VexIR.h>

VexExecutionState::VexExecutionState(TargetArch t) : arch(t) { 
    //set up the memory map
    MemLocation dm;
    this->didCall = false;
    this->exitTy = Fallthrough;
    this->memMap.add(
        make_pair(icl::interval<uint64_t>::left_open(0, 4294967296), dm));

    switch( t.ta ) {
        case X86:
            //we need to go all the way up to where VEX puts the segment
            //selectors and stuff
            this->regState.resize(offsetof(VexGuestX86State, padding1));
            break;

        case ARM:
            this->regState.resize(offsetof(VexGuestARMState, padding1));
            break;
        
        case AMD64:
            this->regState.resize(offsetof(VexGuestAMD64State, pad1));
            break;

        default:
            assert( false && "Nonsupported architecture");
    }

    for(unsigned int i = 0; i < this->regState.size(); i++) 
    {
        ConstantValue   v;
        v.valueIsKnown = false;
        RegConstant     rc = { i, 8, v};
        this->regState[i] = rc; 
    }

    return;
}

bool canMerge(regMapT &a, regMapT &b) {
    //is there any overlap in terms of defined ConstantValues between a and b?
    bool                f = true; 
    regMapT::iterator   ait = a.begin();
    regMapT::iterator   bit = b.begin();

    assert(a.size() == b.size() );
    while( ait != a.end() && bit != b.end() ) {
        ConstantValue   &v1 = (*ait).v;
        ConstantValue   &v2 = (*bit).v;

        //both v1 and v2 known could be false, only a problem
        //when both are true
        if( v1.valueIsKnown == true && v2.valueIsKnown == true ) {
            //there is overlap, they both define something at the same position
            f = false;
            break;
        }
        
        ++ait;
        ++bit;
    }

    return f;
}

regMapT merge(regMapT &a, regMapT &b) {
    regMapT merged;

    assert(a.size() == b.size());
    assert(a.size() > 0);
    merged.resize(a.size());

    //we know a and b are disjoint, so we can do this one after the other
    //or more aptly, the set of things whos value is known is disjoint
   for( int i = 0; i < a.size(); i++ ) {
        if( a[i].v.valueIsKnown == true ) {
            merged[i] = a[i];
        }
    }

   for( int i = 0; i < b.size(); i++ ) {
        if( b[i].v.valueIsKnown == true ) {
            merged[i] = b[i];
        }
    }
    
    return merged;
}

void VexExecutionState::addRegPermutations(list<regMapT>    n) {

    if( this->permutations.size() == 0 ) {
        //if we have no permutations currently, this list is now our list of 
        //permutations. all hail. 
        this->permutations.swap(n);
    } else {
        list<regMapT>   built;
        //iterate over each element in n
        list<regMapT>::iterator nit = n.begin();
        while( nit != n.end() ) {
            regMapT &a = *nit;
            //then for each a in n, compare it to each b in permutations
            list<regMapT>::iterator pit = this->permutations.begin();
            while( pit != this->permutations.end() ) {
                regMapT &b = *pit;
                
                //can we merge a and b?
                if( canMerge(a, b) ) {
                    //if we can merge them, do it and then add that to built
                    regMapT m = merge(a, b);
                    built.push_back(m);
                }

                ++pit;
            }
            ++nit;
        }

        //store the set that we just built
        this->permutations.swap(built);
    }

    return;
}

bool blockReadsFromState(BlockPtr block, regMapT &regs) {
    bool        doTest = true;
    TargetArch  ta = block->getArch();
   
    //iterate over the registers in the regMapT
    //for( regMapT::iterator it = regs.begin(); it != regs.end(); ++it) {
    for( int i = 0; i < regs.size(); i++) {
        //if the current register value is defined, then turn it into a 
        //Register object and see if there is a read for it
        RegConstant &c = regs[i]; 
       
        if( c.v.valueIsKnown ) {
            //we know a value for this register, see what the width is
            Register    r;
            uint32_t    regWidth = c.originalWidth;
            uint32_t    regOffset = c.originalOffset;

            //make a register with this width and offset
            r = guestOffsetToRegister(regOffset, regWidth, ta);

            //check and see if the current block writes to this register
            if( !transfersRegister( block, r ) ) {
                doTest = false;
                break;
            }
        }
    }

    return doTest;
}

list<VexExecutionStatePtr> VexExecutionState::getPermutations(BlockPtr b) {
    list<VexExecutionStatePtr>  states;
    
    //do we even have any permutations? if we don't, just return a set with
    //only us in it
    int p = this->permutations.size();
    if( p ) {
        //okay, we are going to generate p permutations 
        std::list<regMapT>::iterator it = this->permutations.begin();
        while( it != this->permutations.end() ) {
            regMapT             permRegs = *it;
            //if a permutation has all writes to state that are not 
            //read by this block, then we can discard it, so count how many
            //writes to state in this regMap matter
            
            if( b ) {
                if( blockReadsFromState(b, permRegs) ) {
                    VexExecutionStatePtr    vss = 
                        VexExecutionStatePtr(new VexExecutionState(*this));
                    vss->regState = permRegs;
                    states.push_back(vss);
                }
            } else {
                VexExecutionStatePtr    vss = 
                    VexExecutionStatePtr(new VexExecutionState(*this));
                vss->regState = permRegs;
                states.push_back(vss);
            }

            ++it;
        }
    } else {
        states.push_back(shared_from_this());
    }

    return states;
}

void VexExecutionState::setMemFromConst(uint64_t addr, ConstantValue v) {
	// XXX: hack
	if(v.width == 128) return;
    //break the ConstantValue out 
    assert(v.width == 8 || v.width == 16 || v.width == 32 || v.width == 64);

    switch(v.width) {
        case 8:
            {
                ConstantValue   cv;

                cv.valueIsKnown = v.valueIsKnown;
                cv.width = 8;
                
                if( v.valueIsKnown ) {
                    cv.U8 = v.U8;
                }
                
                MemLocation m1(cv);

                this->memMap.add(make_pair(
                    icl::interval<uint64_t>::right_open(addr, addr+1),
                    m1));
            }
            break;

        case 16:
            {
                ConstantValue   cv1;
                ConstantValue   cv2;

                cv1.valueIsKnown = v.valueIsKnown;
                cv2.valueIsKnown = v.valueIsKnown;

                if( v.valueIsKnown ) {
                    cv1.U8 = v.U16 & 0xff;
                    cv2.U8 = (v.U16 >> 8) & 0xff;
                }

                MemLocation m1(cv1);
                MemLocation m2(cv2);

                this->memMap.add(make_pair(
                    icl::interval<uint64_t>::right_open(addr, addr+1),
                    m1));
                this->memMap.add(make_pair(
                    icl::interval<uint64_t>::right_open(addr+1, addr+2),
                    m2));
            }
            break;

        case 32:
            {   
                ConstantValue   cv1;
                ConstantValue   cv2;
                ConstantValue   cv3;
                ConstantValue   cv4;
                
                cv1.valueIsKnown = v.valueIsKnown;
                cv2.valueIsKnown = v.valueIsKnown;
                cv3.valueIsKnown = v.valueIsKnown;
                cv4.valueIsKnown = v.valueIsKnown;

                if( v.valueIsKnown ) {
                    cv1.U8 = v.U32 & 0xff;
                    cv2.U8 = (v.U32 >> 8) & 0xff;
                    cv3.U8 = (v.U32 >> 16 ) & 0xff; 
                    cv4.U8 = (v.U32 >> 24 ) & 0xff;
                }

                MemLocation m1(cv1);
                MemLocation m2(cv2);
                MemLocation m3(cv3);
                MemLocation m4(cv4);

                this->memMap.add(make_pair(
					icl::interval<uint64_t>::right_open(addr, addr+1),
                    m1));
                this->memMap.add(make_pair(
                    icl::interval<uint64_t>::right_open(addr+1, addr+2),
                    m2));
                this->memMap.add(make_pair(
                    icl::interval<uint64_t>::right_open(addr+2, addr+3),
                    m3));
                this->memMap.add(make_pair(
                    icl::interval<uint64_t>::right_open(addr+3, addr+4),
                    m4));
            }
            break;

        case 64:
            {
                ConstantValue   cv1;
                ConstantValue   cv2;
                ConstantValue   cv3;
                ConstantValue   cv4;
                ConstantValue   cv5;
                ConstantValue   cv6;
                ConstantValue   cv7;
                ConstantValue   cv8;

                cv1.valueIsKnown = v.valueIsKnown;
                cv2.valueIsKnown = v.valueIsKnown;
                cv3.valueIsKnown = v.valueIsKnown;
                cv4.valueIsKnown = v.valueIsKnown;
                cv5.valueIsKnown = v.valueIsKnown;
                cv6.valueIsKnown = v.valueIsKnown;
                cv7.valueIsKnown = v.valueIsKnown;
                cv8.valueIsKnown = v.valueIsKnown;

                if( v.valueIsKnown ) {
                    cv1.U8 = v.U32 & 0xff;
                    cv2.U8 = (v.U64 >> 8) & 0xff;
                    cv3.U8 = (v.U64 >> 16 ) & 0xff; 
                    cv4.U8 = (v.U64 >> 24 ) & 0xff;
                    cv5.U8 = (v.U64 >> 32 ) & 0xff;
                    cv6.U8 = (v.U64 >> 40 ) & 0xff;
                    cv7.U8 = (v.U64 >> 48 ) & 0xff;
                    cv8.U8 = (v.U64 >> 56 ) & 0xff;
                }
    
                MemLocation m1(cv1);
                MemLocation m2(cv2);
                MemLocation m3(cv3);
                MemLocation m4(cv4);
                MemLocation m5(cv5);
                MemLocation m6(cv6);
                MemLocation m7(cv7);
                MemLocation m8(cv8);

                this->memMap.add(make_pair(
                    icl::interval<uint64_t>::right_open(addr, addr+1),
                    m1));
                this->memMap.add(make_pair(
                    icl::interval<uint64_t>::right_open(addr+1, addr+2),
                    m2));
                this->memMap.add(make_pair(
                    icl::interval<uint64_t>::right_open(addr+2, addr+3),
                    m3));
                this->memMap.add(make_pair(
                    icl::interval<uint64_t>::right_open(addr+3, addr+4),
                    m4));
                this->memMap.add(make_pair(
                    icl::interval<uint64_t>::right_open(addr+4, addr+5),
                    m5));
                this->memMap.add(make_pair(
                    icl::interval<uint64_t>::right_open(addr+5, addr+6),
                    m6));
                this->memMap.add(make_pair(
                    icl::interval<uint64_t>::right_open(addr+6, addr+7),
                    m7));
                this->memMap.add(make_pair(
                    icl::interval<uint64_t>::right_open(addr+7, addr+8),
                    m8));
            }
            break;
    }


    return;
}

ConstantValue VexExecutionState::getConstFromMem(  uint64_t addr, 
                                                    unsigned long stride) 
{
    ConstantValue   v;

    v.valueIsKnown = false;

    assert(stride == 1 || stride == 2 || stride == 4 || stride == 8);

    addrMapT::const_iterator  it = 
        this->memMap.find(
            icl::interval<uint64_t>::closed(addr, addr+stride));
    
    if( it != this->memMap.end() ) {
        vector<ConstantValue>   constV;
        //okay, well, it's there... but see if everything is a known
		//hack, make sure we only read stride bytes
		int r = 0;
        uint64_t    curAddr = addr;
        while( it != this->memMap.end() && r < stride) {
            MemLocation ml = (*it).second;
            ConstantValue vl = ml.get();

            if( vl.valueIsKnown && curAddr == (*it).first.lower() ) {
                constV.push_back(ml.get());
            }

            ++it;
			++r;
            ++curAddr;
        }

        //okay good now make sure that the number of things we got
        //are equal to the stride
        if( constV.size() == stride ) {
            switch(stride) {
                case 1:
                    {
                        ConstantValue   a1 = constV[0];
                        //easy case
                        v.valueIsKnown = true;
                        v.width = 8;
                        v.valueType = ConstantValue::T_I8;
                        v.U8 = a1.U8;
                    }
                    break;

                case 2:
                    {
                        ConstantValue   a1 = constV[0];
                        ConstantValue   a2 = constV[1];

                        v.valueIsKnown = true;
                        v.width = 16;
                        v.valueType = ConstantValue::T_I16;

                        uint32_t    v1 = a1.U8;
                        uint32_t    v2 = a2.U8;

                        v.U16 = (uint16_t)((v2 << 8) +
                                v1);
                    }
                    break;

                case 4:
                    {
                        ConstantValue   a1 = constV[0];
                        ConstantValue   a2 = constV[1];
                        ConstantValue   a3 = constV[2];
                        ConstantValue   a4 = constV[3];

                        v.valueIsKnown = true;
                        v.width = 32;
                        v.valueType = ConstantValue::T_I32;
                        
                        uint32_t    v1 = a1.U8;
                        uint32_t    v2 = a2.U8;
                        uint32_t    v3 = a3.U8;
                        uint32_t    v4 = a4.U8;

                        v.U32 = (uint32_t)((v4 << 24 ) + 
                            (v3 << 16 ) + 
                            (v2 << 8 ) +
                            v1);

                    }
                    break;

                case 8:
                    {
                        ConstantValue   a1 = constV[0];
                        ConstantValue   a2 = constV[1];
                        ConstantValue   a3 = constV[2];
                        ConstantValue   a4 = constV[3];
                        ConstantValue   a5 = constV[4];
                        ConstantValue   a6 = constV[5];
                        ConstantValue   a7 = constV[6];
                        ConstantValue   a8 = constV[7];

                        v.valueIsKnown = true;
                        v.width = 64;
                        v.valueType = ConstantValue::T_I64;

                        uint64_t    v1 = a1.U8;
                        uint64_t    v2 = a2.U8;
                        uint64_t    v3 = a3.U8;
                        uint64_t    v4 = a4.U8;
                        uint64_t    v5 = a5.U8;
                        uint64_t    v6 = a6.U8;
                        uint64_t    v7 = a7.U8;
                        uint64_t    v8 = a8.U8;
                        
                        v.U64 = (uint64_t)((v8 << 56) + 
                            (v7 << 48 ) + 
                            (v6 << 40 ) + 
                            (v5 << 32 ) +
                            (v4 << 24 ) + 
                            (v3 << 16 ) + 
                            (v2 << 8 ) +
                            v1);
                    }
                    break;
            }
        }
    }

    return v;
}

ConstantValue VexExecutionState::getConstFromState( unsigned long off,
                                                    unsigned long width)
{
    return VexExecutionState::getStateS(off, width, this->regState);
}

ConstantValue VexExecutionState::getStateS( uint32_t off, 
                                            uint32_t width,
                                            regMapT &s)
{
    ConstantValue   v = {0};
    //switch on the width 
    assert(width == 8 || width == 16 || width == 32 || width == 64);

    v.valueIsKnown = false;
    switch(width) {
        case 8:
            {
                assert( s.size() > off );
                RegConstant cv = s[off];

                v.width = 8;
                v.valueType = ConstantValue::T_I8;
                if( cv.v.valueIsKnown ) {
                    v.valueIsKnown = true;
                    v.U8 = cv.v.U8;
                }
            }
            break;
        case 16:
            {
                assert( s.size() > off+1 );
                RegConstant &cv1 = s[off];
                RegConstant &cv2 = s[off+1];

                v.width = 16;
                v.valueType = ConstantValue::T_I16;

                if( cv1.v.valueIsKnown &&
                    cv2.v.valueIsKnown )
                {
                    v.valueIsKnown = true;
                    uint16_t    t1 = cv1.v.U8;
                    uint16_t    t2 = cv2.v.U8;

                    v.U16 = (t2 << 8) + t1;
                }
            }
            break;
        case 32:
            {
                assert( s.size() > off+3 );
                RegConstant &cv1 = s[off];
                RegConstant &cv2 = s[off+1];
                RegConstant &cv3 = s[off+2];
                RegConstant &cv4 = s[off+3];

                v.width = 32;
                v.valueType = ConstantValue::T_I32;
                if( cv1.v.valueIsKnown &&
                    cv2.v.valueIsKnown &&
                    cv3.v.valueIsKnown &&
                    cv4.v.valueIsKnown )
                {
                    v.valueIsKnown = true;
                    uint32_t    t1 = cv1.v.U8;
                    uint32_t    t2 = cv2.v.U8;
                    uint32_t    t3 = cv3.v.U8;
                    uint32_t    t4 = cv4.v.U8;

                    v.U32 = (t4 << 24) + (t3 << 16) + (t2 << 8) + t1;
                }
            }
            break;
        case 64:
            {
                assert( s.size() > off+7 );
                RegConstant &cv1 = s[off];
                RegConstant &cv2 = s[off+1];
                RegConstant &cv3 = s[off+2];
                RegConstant &cv4 = s[off+3];
                RegConstant &cv5 = s[off+4];
                RegConstant &cv6 = s[off+5];
                RegConstant &cv7 = s[off+6];
                RegConstant &cv8 = s[off+7];

                v.valueType = ConstantValue::T_I64;
                v.width = 64;

                if( cv1.v.valueIsKnown &&
                    cv2.v.valueIsKnown &&
                    cv3.v.valueIsKnown &&
                    cv4.v.valueIsKnown &&
                    cv5.v.valueIsKnown &&
                    cv6.v.valueIsKnown &&
                    cv7.v.valueIsKnown &&
                    cv8.v.valueIsKnown )
                {
                    v.valueIsKnown = true;
                    uint64_t    t1 = cv1.v.U8;
                    uint64_t    t2 = cv2.v.U8;
                    uint64_t    t3 = cv3.v.U8;
                    uint64_t    t4 = cv4.v.U8;
                    uint64_t    t5 = cv5.v.U8;
                    uint64_t    t6 = cv6.v.U8;
                    uint64_t    t7 = cv7.v.U8;
                    uint64_t    t8 = cv8.v.U8;

                    v.U64 = (t8 << 56) + 
                            (t7 << 48 ) + 
                            (t6 << 40 ) + 
                            (t5 << 32 ) +
                            (t4 << 24 ) + 
                            (t3 << 16 ) + 
                            (t2 << 8 ) +
                            t1;
                }
            }
            break;
    }

    return v;
}

void VexExecutionState::setStateFromConst(unsigned long off, RegConstant v) {
    VexExecutionState::setStateS(off, v, this->regState);
}

void VexExecutionState::setStateS(uint32_t off, RegConstant v, regMapT &s) {
    //switch on the width
    assert( v.v.width == 8 || 
            v.v.width == 16 || 
            v.v.width == 32 || 
            v.v.width == 64);

    switch(v.v.width) {
        case 8:
            //simple case
            {
                ConstantValue   cv;

                cv.valueIsKnown = v.v.valueIsKnown;
                cv.width = 8;
                
                if( v.v.valueIsKnown ) {
                    v.v.U8 = v.v.U8;
                }

                RegConstant rc = { off, 8, cv };

                assert( s.size() > off );
                s[off] = rc;
            }
            break;

        case 16:
            {
                ConstantValue   cv1;
                ConstantValue   cv2;

                cv1.valueIsKnown = v.v.valueIsKnown;
                cv1.width = 8;
                cv2.valueIsKnown = v.v.valueIsKnown;
                cv2.width = 8;

                if( v.v.valueIsKnown ) {
                    cv1.U8 = v.v.U16 & 0xff;
                    cv2.U8 = (v.v.U16 >> 8) & 0xff;
                }

                RegConstant    r1 = {off, 16, cv1};
                RegConstant    r2 = {off, 16, cv2};

                assert( s.size() > off+1 );
                s[off] = r1;
                s[off+1] = r2;
            }
            break;

        case 32:
            {   
                ConstantValue   cv1;
                ConstantValue   cv2;
                ConstantValue   cv3;
                ConstantValue   cv4;
                
                cv1.valueIsKnown = v.v.valueIsKnown;
                cv2.valueIsKnown = v.v.valueIsKnown;
                cv3.valueIsKnown = v.v.valueIsKnown;
                cv4.valueIsKnown = v.v.valueIsKnown;
                cv1.width = 8;
                cv2.width = 8;
                cv3.width = 8;
                cv4.width = 8;

                if( v.v.valueIsKnown ) {
                    cv1.U8 = v.v.U32 & 0xff;
                    cv2.U8 = (v.v.U32 >> 8) & 0xff;
                    cv3.U8 = (v.v.U32 >> 16 ) & 0xff; 
                    cv4.U8 = (v.v.U32 >> 24 ) & 0xff;
                }

                RegConstant    rc1 = { off, 32, cv1 };
                RegConstant    rc2 = { off, 32, cv2 };
                RegConstant    rc3 = { off, 32, cv3 };
                RegConstant    rc4 = { off, 32, cv4 };
                
                assert( s.size() > off+3 );
                s[off] = rc1;
                s[off+1] = rc2;
                s[off+2] = rc3;
                s[off+3] = rc4;
            }
            break;

        case 64:
            {
                ConstantValue   cv1;
                ConstantValue   cv2;
                ConstantValue   cv3;
                ConstantValue   cv4;
                ConstantValue   cv5;
                ConstantValue   cv6;
                ConstantValue   cv7;
                ConstantValue   cv8;

                cv1.valueIsKnown = v.v.valueIsKnown;
                cv2.valueIsKnown = v.v.valueIsKnown;
                cv3.valueIsKnown = v.v.valueIsKnown;
                cv4.valueIsKnown = v.v.valueIsKnown;
                cv5.valueIsKnown = v.v.valueIsKnown;
                cv6.valueIsKnown = v.v.valueIsKnown;
                cv7.valueIsKnown = v.v.valueIsKnown;
                cv8.valueIsKnown = v.v.valueIsKnown;
                cv1.width = 8;
                cv2.width = 8;
                cv3.width = 8;
                cv4.width = 8;
                cv5.width = 8;
                cv6.width = 8;
                cv7.width = 8;
                cv8.width = 8;

                if( v.v.valueIsKnown ) {
                    cv1.U8 = v.v.U32 & 0xff;
                    cv2.U8 = (v.v.U64 >> 8) & 0xff;
                    cv3.U8 = (v.v.U64 >> 16 ) & 0xff; 
                    cv4.U8 = (v.v.U64 >> 24 ) & 0xff;
                    cv5.U8 = (v.v.U64 >> 32 ) & 0xff;
                    cv6.U8 = (v.v.U64 >> 40 ) & 0xff;
                    cv7.U8 = (v.v.U64 >> 48 ) & 0xff;
                    cv8.U8 = (v.v.U64 >> 56 ) & 0xff;
                }

                RegConstant    rc1 = { off, 64, cv1 };
                RegConstant    rc2 = { off, 64, cv2 };
                RegConstant    rc3 = { off, 64, cv3 };
                RegConstant    rc4 = { off, 64, cv4 };
                RegConstant    rc5 = { off, 64, cv5 };
                RegConstant    rc6 = { off, 64, cv6 };
                RegConstant    rc7 = { off, 64, cv7 };
                RegConstant    rc8 = { off, 64, cv8 };
                
                assert( s.size() > off+7 );
                s[off]   = rc1;
                s[off+1] = rc2;
                s[off+2] = rc3;
                s[off+3] = rc4;
                s[off+4] = rc5;
                s[off+5] = rc6;
                s[off+6] = rc7;
                s[off+7] = rc8;
            }
            break;
    }

    return;
}

void VexExecutionState::setMem(  unsigned long addr, 
								  unsigned short width, 
								  unsigned long long val) 
{
	ConstantValue   cv = {0};

	cv.valueType = ConstantValue::T_I8;
	cv.width = width;

	switch(cv.width) {
		case 8:
			cv.U8 = (uint8_t)val;
			break;
		case 16:
			cv.U16 = (uint16_t)val;
			break;
		case 32:
			cv.U32 = (uint32_t)val;
			break;
		case 64:
			cv.U64 = (uint64_t)val;
			break;
	}

	cv.valueIsKnown = true;

	this->setMemFromConst(addr, cv);
	return;
}

void VexExecutionState::setState(  unsigned long off, 
                                    unsigned short width, 
                                    unsigned long long val) 
{
    RegConstant cv = {0};

    cv.originalOffset = off;
    cv.originalWidth = width;
    cv.v.valueType = ConstantValue::T_I8;
    cv.v.width = width;

    switch(cv.v.width) {
        case 8:
            cv.v.U8 = (uint8_t)val;
            break;
        case 16:
            cv.v.U16 = (uint16_t)val;
            break;
        case 32:
            cv.v.U32 = (uint32_t)val;
            break;
        case 64:
            cv.v.U64 = (uint64_t)val;
            break;
    }

    cv.v.valueIsKnown = true;

    this->setStateFromConst(off, cv);
    return;
}
