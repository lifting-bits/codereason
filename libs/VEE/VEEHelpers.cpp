#include "VEE.h"

#include <boost/shared_ptr.hpp>

using namespace std;
using namespace boost;

uint16_t getArgLen(ExpressionPtr opArg) {
    uint16_t    width = 0;
   
    do {
        if( ExConstPtr cst = dynamic_pointer_cast<ExConst>(opArg) ) {
            ConstantValue   cv = cst->getVal();
            
            assert(cv.width ==8 || cv.width == 16 || cv.width == 32 || cv.width == 64 || cv.width == 1 );
            width = cv.width;
            break;
        }

        if( ExRdTmpPtr rdt = dynamic_pointer_cast<ExRdTmp>(opArg) ) {
            TempValPtr tv = rdt->getTmp();
            ConstantValue cv = tv->getVal();

            assert(cv.width == 8 || cv.width == 16 || cv.width == 32 || cv.width == 64 || cv.width == 1 );
            width = cv.width;
            break;
        }

        assert(!"BAD STATE");
    } while( false );

    return width;
}

void getConstArgs(vector<ExpressionPtr> args, vector<ConstantValue> &consts ) {

    for(vector<ExpressionPtr>::iterator it = args.begin();
        it != args.end();
        ++it)
    {  
        ExpressionPtr   exp = *it;
        if( ExConstPtr cst = dynamic_pointer_cast<ExConst>(exp) ) {
            ConstantValue cv = cst->getVal();
            if( cv.valueIsKnown ) {
                consts.push_back(cv);
            }
        }

        if( ExRdTmpPtr rdt = dynamic_pointer_cast<ExRdTmp>(exp) ) {
            TempValPtr  tv = rdt->getTmp();
            ConstantValue cv = tv->getVal();
            if( cv.valueIsKnown ) {
                consts.push_back(cv);
            }
        }
    }

    return;
}

void applyPreConditions(VexExecutionStatePtr vss, Condition *c) {
    if( c ) {
        c->runPreVee(vss);
    }
    return;
}

bool checkPostConditions(VexExecutionStatePtr vss, Condition *c) {
    if( c ) {
        return c->runPostVee(vss);
    } else {
        return false;
    }
}

uint16_t getStrideFromWidth(uint16_t width) {
    switch(width) {
        case 8:
            return 1;
            break;
        case 16:
            return 2;
            break;
        case 32:
            return 4;
            break;
        case 64:
            return 8;
            break;
    }

    return 0;
}
