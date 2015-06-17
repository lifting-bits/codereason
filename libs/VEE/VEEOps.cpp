#include "VEE.h"

#include <boost/shared_ptr.hpp>

using namespace std;
using namespace boost;

ConstantValue doUSad8(vector<ExpressionPtr> args) {
    ConstantValue           v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    getConstArgs(args, constArgs);

    if( constArgs.size() == 2 ) {
        ConstantValue   a1 = constArgs[0];
        ConstantValue   a2 = constArgs[1];
        //okay, this is CRAZY
        //get 8 values
        uint8_t a11, a12, a13, a14, a21, a22, a23, a24;

        a11 = a1.U32 & 0xff;
        a12 = (a1.U32 >> 8) & 0xff;
        a13 = (a1.U32 >> 16) & 0xff;
        a14 = (a1.U32 >> 24) & 0xff;

        a21 = a2.U32 & 0xff;
        a22 = (a2.U32 >> 8) & 0xff;
        a23 = (a2.U32 >> 16) & 0xff;
        a24 = (a2.U32 >> 24) & 0xff;

        //compute their differences
        int d1, d2, d3, d4;

        d1 = a11 - a21;
        d2 = a12 - a22;
        d3 = a13 - a23;
        d4 = a14 - a24;

        //take their absolute values
        d1 = abs(d1);
        d2 = abs(d2);
        d3 = abs(d3);
        d4 = abs(d4);

        //sum the values 
        uint32_t    res = d1 + d2 + d3 + d4;

        //return the result
        v.valueIsKnown = true;
        v.width = 32;
        v.valueType = ConstantValue::T_I32;
        v.U32 = res;
    }

    return v;
}

ConstantValue doAdd(vector<ExpressionPtr>  args) {
    ConstantValue           v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    getConstArgs(args, constArgs);
    
    if( constArgs.size() == 2 ) {
        //add is a binop, and all args are known
        ConstantValue   a1 = constArgs[0];
        ConstantValue   a2 = constArgs[1];

        assert(a1.width == a2.width);
        unsigned short opW = a1.width;
        switch(opW) {
            case 8:
                v.valueType = ConstantValue::T_I8;
                v.valueIsKnown = true;
                v.width = 8;
                v.U8 = a1.U8 + a2.U8;
                break;
            case 16:
                v.valueType = ConstantValue::T_I16;
                v.valueIsKnown = true;
                v.width = 16;
                v.U16 = a1.U16 + a2.U16;
                break;
            case 32:
                v.valueType = ConstantValue::T_I32;
                v.valueIsKnown = true;
                v.width = 32;
                v.U32 = a1.U32 + a2.U32;
                break;
            case 64:
                v.valueType = ConstantValue::T_I64;
                v.valueIsKnown = true;
                v.width = 64;
                v.U64 = a1.U64 + a2.U64;
                break;
        }
    }

    return v;
}

ConstantValue doSub(vector<ExpressionPtr>   args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    getConstArgs(args, constArgs);
    if( constArgs.size() == 2 ) {
        //sub is a binop
        ConstantValue   a1 = constArgs[0];
        ConstantValue   a2 = constArgs[1];

        assert(a1.width == a2.width);
        unsigned short opW = a1.width;
        switch(opW) {
            case 8:
                v.valueType = ConstantValue::T_I8;
                v.valueIsKnown = true;
                v.width = 8;
                v.U8 = a1.U8 - a2.U8;
                break;
            case 16:
                v.valueType = ConstantValue::T_I16;
                v.valueIsKnown = true;
                v.width = 16;
                v.U16 = a1.U16 - a2.U16;
                break;
            case 32:
                v.valueType = ConstantValue::T_I32;
                v.valueIsKnown = true;
                v.width = 32;
                v.U32 = a1.U32 - a2.U32;
                break;
            case 64:
                v.valueType = ConstantValue::T_I64;
                v.valueIsKnown = true;
                v.width = 64;
                v.U64 = a1.U64 - a2.U64;
                break;
        }
    }

    return v;
}

ConstantValue doMul(vector<ExpressionPtr>   args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    getConstArgs(args, constArgs);
    if( constArgs.size() == 2 ) {
        ConstantValue   v1 = constArgs[0];
        ConstantValue   v2 = constArgs[1];

        assert(v1.width == v2.width);
        uint16_t    opW = v1.width;
        switch(opW) {
            case 8:
                {
                    uint8_t a1 = v1.U8;
                    uint8_t a2 = v2.U8;
                    v.width = 8;
                    v.valueType = ConstantValue::T_I8;
                    v.U8 = a1*a2;
                }
                break;

            case 16:
                {
                    uint16_t    a1 = v1.U16;
                    uint16_t    a2 = v2.U16;
                    v.width = 16;
                    v.valueType = ConstantValue::T_I16;
                    v.U16 = a1*a2;

                }
                break;

            case 32:
                {
                    uint32_t    a1 = v1.U32;
                    uint32_t    a2 = v2.U32;
                    v.width = 32;
                    v.valueType = ConstantValue::T_I32;
                    v.U32 = a1*a2;

                }
                break;

            case 64:
                {
                    uint64_t    a1 = v1.U64;
                    uint64_t    a2 = v2.U64;
                    v.width = 64;
                    v.valueType = ConstantValue::T_I64;
                    v.U64 = a1*a2;

                }
                break;
        }
    }

    return v;
}

ConstantValue doMulU(vector<ExpressionPtr>   args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    getConstArgs(args, constArgs);
    if( constArgs.size() == 2 ) {
        //mulU is a binop
        ConstantValue   a1 = constArgs[0];
        ConstantValue   a2 = constArgs[1];

        assert(a1.width == a2.width);
        unsigned short opW = a1.width;
        switch(opW) {
            case 8:
                {
                    v.valueIsKnown = true;
                    v.width = 16;
                    v.valueType = ConstantValue::T_I16;
                    uint8_t i1 = a1.U8;
                    uint8_t i2 = a2.U8;
                    v.U16 = i1*i2;
                }
                break;
            case 16:
                {
                    v.valueIsKnown = true;
                    v.width = 32;
                    v.valueType = ConstantValue::T_I32;
                    uint16_t i1 = a1.U16;
                    uint16_t i2 = a2.U16;
                    v.U32 = i1*i2;
                }
                break;
            case 32:
                {
                    v.valueIsKnown = true;
                    v.width = 64;
                    v.valueType = ConstantValue::T_I64;
                    uint32_t i1 = a1.U32;
                    uint32_t i2 = a2.U32;
                    v.U64 = i1*i2;
                }
                break;
            case 64:
                //something went wrong, we can't do this
                throw StepErr("mulU of unusual size");
                break;
        }
    }

    return v;
}

ConstantValue doMulS(vector<ExpressionPtr>   args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    getConstArgs(args, constArgs);
    if( constArgs.size() == 2 ) {
        //mulS is a binop
        ConstantValue   a1 = constArgs[0];
        ConstantValue   a2 = constArgs[1];
        
        assert(a1.width == a2.width);
        unsigned short opW = a1.width;
        switch(opW) {
            case 8:
                {
                    v.width = 16;
                    v.valueType = ConstantValue::T_I16;
                    v.valueIsKnown = true;
                    int8_t i1 = a1.U8;
                    int8_t i2 = a2.U8;
                    v.U16 = i1*i2;
                }
                break;
            case 16:
                {
                    v.width = 32;
                    v.valueType = ConstantValue::T_I32;
                    v.valueIsKnown = true;
                    int16_t i1 = a1.U16;
                    int16_t i2 = a2.U16;
                    v.U32 = i1*i2;
                }
                break;
            case 32:
                {
                    v.width = 64;
                    v.valueType = ConstantValue::T_I64;
                    v.valueIsKnown = true;
                    int32_t i1 = a1.U32;
                    int32_t i2 = a2.U32;
                    v.U64 = i1*i2;
                }
                break;
            case 64:
                //can't do this either
                throw StepErr("mulS of unusual size");
                break;
        }
    }

    return v;
}

ConstantValue doOr(vector<ExpressionPtr>   args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    getConstArgs(args, constArgs);
    if( constArgs.size() == 2 ) {
        ConstantValue   a1 = constArgs[0];
        ConstantValue   a2 = constArgs[1];

        assert(a1.width == a2.width);
        unsigned short opW = a1.width;
        switch(opW) {
            case 8:
                v.valueIsKnown = true;
                v.width = 8;
                v.valueType = ConstantValue::T_I8;
                v.U8 = a1.U8 | a2.U8;
                break;
            case 16:
                v.valueIsKnown = true;
                v.width = 16;
                v.valueType = ConstantValue::T_I16;
                v.U16 = a1.U16 | a2.U16;
                break;
            case 32:
                v.valueIsKnown = true;
                v.width = 32;
                v.valueType = ConstantValue::T_I32;
                v.U32 = a1.U32 | a2.U32;
                break;
            case 64:
                v.valueIsKnown = true;
                v.width = 64;
                v.valueType = ConstantValue::T_I64;
                v.U64 = a1.U64 | a2.U64;
                break;
        }
    }
   
    return v;
}

ConstantValue doAnd(vector<ExpressionPtr>   args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    getConstArgs(args, constArgs);
    if( constArgs.size() == 2 ) {
        ConstantValue   a1 = constArgs[0];
        ConstantValue   a2 = constArgs[1];

        assert(a1.width == a2.width);
        unsigned short opW = a1.width;
        switch(opW) {
            case 8:
                v.valueIsKnown = true;
                v.width = 8;
                v.valueType = ConstantValue::T_I8;
                v.U8 = a1.U8 & a2.U8;
                break;
            case 16:
                v.valueIsKnown = true;
                v.width = 16;
                v.valueType = ConstantValue::T_I16;
                v.U16 = a1.U16 & a2.U16;
                break;
            case 32:
                v.valueIsKnown = true;
                v.width = 32;
                v.valueType = ConstantValue::T_I32;
                v.U32 = a1.U32 & a2.U32;
                break;
            case 64:
                v.valueIsKnown = true;
                v.width = 64;
                v.valueType = ConstantValue::T_I64;
                v.U64 = a1.U64 & a2.U64;
                break;
        }
    }

    return v;
}

ConstantValue doXor(vector<ExpressionPtr>   args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    getConstArgs(args, constArgs);
    if( constArgs.size() == 2 ) {
        ConstantValue   a1 = constArgs[0];
        ConstantValue   a2 = constArgs[1];

        assert(a1.width == a2.width);
        unsigned short opW = a1.width;
        switch(opW) {
            case 8:
                v.valueIsKnown = true;
                v.width = 8;
                v.valueType = ConstantValue::T_I8;
                v.U8 = a1.U8 ^ a2.U8;
                break;
            case 16:
                v.valueIsKnown = true;
                v.width = 16;
                v.valueType = ConstantValue::T_I16;
                v.U16 = a1.U16 ^ a2.U16;
                break;
            case 32:
                v.valueIsKnown = true;
                v.width = 32;
                v.valueType = ConstantValue::T_I32;
                v.U32 = a1.U32 ^ a2.U32;
                break;
            case 64:
                v.valueIsKnown = true;
                v.width = 64;
                v.valueType = ConstantValue::T_I64;
                v.U64 = a1.U64 ^ a2.U64;
                break;
        }
    }

    return v;
}

ConstantValue doShl(vector<ExpressionPtr>   args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;

    getConstArgs(args, constArgs);
    if( constArgs.size() == 2 ) {
        //Shl is a binop
        ConstantValue   a1 = constArgs[0];
        ConstantValue   a2 = constArgs[1];
        uint64_t        shiftWidth;

        v.width = getArgLen(args[0]);
        v.valueType = getITyFromWidth(v.width);


        switch(a2.width) {
            case 1:
                shiftWidth = a2.U1;
                break;
            case 8:
                shiftWidth = a2.U8;
                break;
            case 16:
                shiftWidth = a2.U16;
                break;
            case 32:
                shiftWidth = a2.U32;
                break;
            case 64:
                shiftWidth = a2.U64;
                break;
            default:
                throw StepErr("Invalid State");
        }

        switch(a1.width) {
            case 8:
                {
                    uint8_t i1 = a1.U8;
                    v.valueIsKnown = true;
                    v.valueType = ConstantValue::T_I8;
                    v.width = 8;
                    v.U8 = i1 << shiftWidth;
                }
                break;
            case 16:
                {
                    uint16_t i1 = a1.U16;
                    v.valueIsKnown = true;
                    v.U16 = i1 << shiftWidth;
                }
                break;
            case 32:
                {
                    uint32_t i1 = a1.U32;
                    v.valueIsKnown = true;
                    v.U32 = i1 << shiftWidth;
                }
                break;
            case 64:
                {
                    uint64_t i1 = a1.U64;
                    v.valueIsKnown = true;
                    v.U64 = i1 << shiftWidth;
                }
                break;
        }
    }

    return v;
}

ConstantValue doShr(vector<ExpressionPtr>   args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    v.width = getArgLen(args[0]);
    v.valueType = getITyFromWidth(v.width);

    getConstArgs(args, constArgs);
    if( constArgs.size() == 2 ) {
        //Shr is a binop
        ConstantValue   a1 = constArgs[0];
        ConstantValue   a2 = constArgs[1];
        uint64_t        shiftWidth;

        switch(a2.width) {
            case 1:
                shiftWidth = a2.U1;
                break;
            case 8:
                shiftWidth = a2.U8;
                break;
            case 16:
                shiftWidth = a2.U16;
                break;
            case 32:
                shiftWidth = a2.U32;
                break;
            case 64:
                shiftWidth = a2.U64;
                break;
            default:
                throw StepErr("Invalid State");
        }

        switch(a1.width) {
            case 8:
                {
                    uint8_t i1 = a1.U8;
                    v.valueIsKnown = true;
                    v.valueType = ConstantValue::T_I8;
                    v.width = 8;
                    v.U8 = i1 >> shiftWidth;
                }
                break;
            case 16:
                {
                    uint16_t i1 = a1.U16;
                    v.valueIsKnown = true;
                    v.U16 = i1 >> shiftWidth;
                }
                break;
            case 32:
                {
                    uint32_t i1 = a1.U32;
                    v.valueIsKnown = true;
                    v.U32 = i1 >> shiftWidth;
                }
                break;
            case 64:
                {
                    uint64_t i1 = a1.U64;
                    v.valueIsKnown = true;
                    v.U64 = i1 >> shiftWidth;
                }
                break;
        }
    }

    return v;

}

ConstantValue doSar(vector<ExpressionPtr>   args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    v.width = getArgLen(args[0]);
    v.valueType = getITyFromWidth(v.width);

    getConstArgs(args, constArgs);
    if( constArgs.size() == 2 ) {
        //binop
        ConstantValue   v1 = constArgs[0];
        ConstantValue   v2 = constArgs[1];

        v.valueIsKnown = true;
        
        switch(v1.width) {
            case 8:
                v.U8 = v1.U8;
                for( uint64_t i = 0; i < v2.U8; i++ ) {
                    v.U8 *= 2;
                }
                break;
            case 16:
                v.U16 = v1.U16;
                for( uint64_t i = 0; i < v2.U16; i++ ) {
                    v.U16 *= 2;
                }
                break;
            case 32:
                v.U32 = v1.U32;
                for( uint64_t i = 0; i < v2.U32; i++ ) {
                    v.U32 *= 2;
                }
                break;
            case 64:
                v.U64 = v1.U64;
                for( uint64_t i = 0; i < v2.U64; i++ ) {
                    v.U64 *= 2;
                }
                break;
        }
    }

    return v;
}

ConstantValue doCmpEQ(vector<ExpressionPtr>   args) {
     ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.width = 1;
    v.valueType = ConstantValue::T_I1;
    v.valueIsKnown = false;
    getConstArgs(args, constArgs);

    if( constArgs.size() == 2 ) {
        ConstantValue   a1 = constArgs[0];
        ConstantValue   a2 = constArgs[1];

        v.valueIsKnown = true;
        assert(a1.width == a2.width);
        unsigned short opW = a1.width;

        switch(opW) {
            case 1:
                if( a1.U1 == a2.U1 ) {
                    v.U1 = 1;
                } else {
                    v.U1 = 0;
                }
                break;
            case 8:
                if( a1.U8 == a2.U8 ) {
                    v.U1 = 1;
                } else {
                    v.U1 = 0;
                }
                break;
            case 16:
                if( a1.U16 == a2.U16 ) {
                    v.U1 = 1;
                } else {
                    v.U1 = 0;
                }
                break;
            case 32:
                if( a1.U32 == a2.U32 ) {
                    v.U1 = 1;
                } else {
                    v.U1 = 0;
                }
                break;
            case 64:
                if( a1.U64 == a2.U64 ) {
                    v.U1 = 1;
                } else {
                    v.U1 = 0;
                }
                break;
        }
    }

    return v;
}

ConstantValue doCmpNE(vector<ExpressionPtr>   args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.width = 1;
    v.valueType = ConstantValue::T_I1;
    v.valueIsKnown = false;
    getConstArgs(args, constArgs);

    if( constArgs.size() == 2 ) {
        ConstantValue   a1 = constArgs[0];
        ConstantValue   a2 = constArgs[1];

        v.valueIsKnown = true;
      
        assert(a1.width == a2.width);
        unsigned short opW = a1.width;

        switch(opW) {
            case 1:
                if( a1.U1 == a2.U1 ) {
                    v.U1 = 0;
                } else {
                    v.U1 = 1;
                }
                break;
            case 8:
                if( a1.U8 == a2.U8 ) {
                    v.U1 = 0;
                } else {
                    v.U1 = 1;
                }
                break;
            case 16:
                if( a1.U16 == a2.U16 ) {
                    v.U1 = 0;
                } else {
                    v.U1 = 1;
                }
                break;
            case 32:
                if( a1.U32 == a2.U32 ) {
                    v.U1 = 0;
                } else {
                    v.U1 = 1;
                }
                break;
            case 64:
                if( a1.U64 == a2.U64 ) {
                    v.U1 = 0;
                } else {
                    v.U1 = 1;
                }
                break;
        }
    }

    return v;
}

ConstantValue doNot(vector<ExpressionPtr>   args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    assert(args.size() > 0);
    v.width = getArgLen(args[0]);
    v.valueType = getITyFromWidth(v.width);

    getConstArgs(args, constArgs);
    if( constArgs.size() == 1 ) {
        //UNOP
        ConstantValue   v1 = constArgs[0];
        switch( v1.width ) {
            case 1:
                {
                    uint8_t r = v1.U1;
                    r = ~r;
                    r = r & 1;
                    v.U1 = r;
                }
                break;
            case 8:
                v.U8 = ~v1.U8;
                break;
            case 16:
                v.U16 = ~v1.U16;
                break;
            case 32:
                v.U32 = ~v1.U32;
                break;
            case 64:
                v.U64 = ~v1.U64;
                break;
        }
    }

    return v;
}

ConstantValue doC64To8(std::vector<ExpressionPtr>   args) {
    ConstantValue   v;

    v.valueIsKnown = false;

    throw StepErr("C64To8");

    return v;
}

ConstantValue doC32To8(std::vector<ExpressionPtr>   args) {
    vector<ConstantValue> constArgs;
    ConstantValue   v;

    v.valueIsKnown = false;
    v.width = 8;
    v.valueType = ConstantValue::T_I8;

    getConstArgs(args, constArgs);
    if( constArgs.size() == 1 ) {
        ConstantValue v1 = constArgs[0];
        v.valueIsKnown = true;
       
        assert( v1.width == 32 );

        v.U8 = (v1.U32 & 0xFF);
    }

    return v;
}

ConstantValue doC64To16(std::vector<ExpressionPtr>  args) {
    ConstantValue   v;

    v.valueIsKnown = false;

    throw StepErr("C64To16");

    return v;
}

ConstantValue doC64LOto32(std::vector<ExpressionPtr>  args) {
    ConstantValue           v;
    vector<ConstantValue>   constVec;

    v.valueType = ConstantValue::T_I32;
    v.width = 32;
    v.valueIsKnown = false;

    getConstArgs(args, constVec);
    if( constVec.size() == 1 ) {
        //C64LOto32 is a unop
        ConstantValue   a1 = constVec[0];

        assert(a1.width == 64);

        //we have enough information to set v
        v.valueIsKnown = true;
        uint64_t k = a1.U64;
        uint64_t l = k & ~0xFFFFFFFF00000000;
        v.U32 = (uint32_t) l;
    }

    return v;
}
ConstantValue doC64HIto32(std::vector<ExpressionPtr>  args) {
    ConstantValue   v;
    vector<ConstantValue>   constVec;

    v.valueType = ConstantValue::T_I32;
    v.width = 32;
    v.valueIsKnown = false;

    getConstArgs(args, constVec);
    if( constVec.size() == 1 ) {
        //C64HIto32 is a unop
        ConstantValue   a1 = constVec[0];

        assert(a1.width == 64);

        //we have enough information to set v
        v.valueIsKnown = true;
        uint64_t k = a1.U64;
        uint64_t l = k & ~0x00000000FFFFFFFF;
        v.U32 = (uint32_t) l;
    }

    return v;
}

ConstantValue doC8UTo16(std::vector<ExpressionPtr> args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    v.width = 16;
    v.valueType = ConstantValue::T_I16;
    getConstArgs(args, constArgs);

    if( constArgs.size() == 1 ) {
        ConstantValue v1 = constArgs[0];

        v.valueIsKnown = true;
        v.U16 = v1.U8;
    }

    return v;
}

ConstantValue doC8UTo64(std::vector<ExpressionPtr> args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    v.width = 64;
    v.valueType = ConstantValue::T_I64;
    getConstArgs(args, constArgs);

    //throw StepErr("C8Uto64");
    if( constArgs.size() == 1 ) {
        ConstantValue   v1 = constArgs[0];

        v.valueIsKnown = true;
        v.U64 = v1.U8;
    }

    return v;
}

ConstantValue doC8STo32(std::vector<ExpressionPtr> args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.width = 32;
    v.valueType = ConstantValue::T_I32;
    v.valueIsKnown = false;
    getConstArgs(args, constArgs);

    if( constArgs.size() == 1 ) {
        ConstantValue a1 = constArgs[0];
        
        assert(a1.width == 8);
        v.valueIsKnown = true;
        int32_t *t = (int32_t *)&v.U32;
        *t = ((int8_t)a1.U8);
    }

    return v;
}

ConstantValue doC8UTo32(std::vector<ExpressionPtr> args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.width = 32;
    v.valueType = ConstantValue::T_I32;
    v.valueIsKnown = false;
    getConstArgs(args, constArgs);

    if( constArgs.size() == 1 ) {
        ConstantValue a1 = constArgs[0];
        
        assert(a1.width == 8);
        v.valueIsKnown = true;
        v.U32 = a1.U8;
    }

    return v;
}

ConstantValue doC32Uto64(std::vector<ExpressionPtr> args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    v.width = 64;
    v.valueType = ConstantValue::T_I64;

    getConstArgs(args, constArgs);
    if( constArgs.size() == 1 ) {
        ConstantValue v1 = constArgs[0];

        v.valueIsKnown = true;

        v.U64 = v1.U32;
    }

    return v;
}

ConstantValue doC16UTo64(std::vector<ExpressionPtr> args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    v.width = 64;
    v.valueType = ConstantValue::T_I64;
    getConstArgs(args, constArgs);

    throw StepErr("C16Uto64");
    if( constArgs.size() == 1 ) {
        ConstantValue   v1 = constArgs[0];

        v.valueIsKnown = true;
    }

    return v;
}

ConstantValue doC32To1(std::vector<ExpressionPtr> args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;
    
    v.width = 1;
    v.valueType = ConstantValue::T_I1;
    v.valueIsKnown = false;
    getConstArgs(args, constArgs);

    if( constArgs.size() == 1 ) {
        ConstantValue   a1 = constArgs[0];
        //C32To1 is a unop
        assert(a1.width == 32);
        assert(a1.valueType == ConstantValue::T_I32);
        v.valueIsKnown = true;
        v.U1 = a1.U32 & ~0xfffffffe;
    }

    return v;
}

ConstantValue doC64To1(std::vector<ExpressionPtr> args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;
    
    v.width = 1;
    v.valueType = ConstantValue::T_I1;
    v.valueIsKnown = false;
    getConstArgs(args, constArgs);
    
    if( constArgs.size() == 1 ) {
        ConstantValue   a1 = constArgs[0];
        //C64To1 is a unop
        assert(a1.width == 64);
        assert(a1.valueType == ConstantValue::T_I64);
        v.valueIsKnown = true;
        v.U1 = a1.U64 & ~0xfffffffffffffffe;
    }

    return v;
}

ConstantValue doC1UTo8(std::vector<ExpressionPtr> args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;
    v.width = 8;
    v.valueType = ConstantValue::T_I8;
    v.valueIsKnown = false;
    getConstArgs(args, constArgs);
   
    if( constArgs.size() == 1 ) {
        ConstantValue a1 = constArgs[0];
        v.valueIsKnown = true;
        v.U8 = a1.U1;
    }

    return v;
}

ConstantValue doC1UTo32(std::vector<ExpressionPtr> args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;
    v.width = 32;
    v.valueType = ConstantValue::T_I32;
    v.valueIsKnown = false;
    getConstArgs(args, constArgs);
   
    if( constArgs.size() == 1 ) {
        ConstantValue a1 = constArgs[0];
        v.valueIsKnown = true;
        v.U32 = a1.U1;
    }

    return v;
}

ConstantValue doC32STo64(std::vector<ExpressionPtr> args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    v.width = 64;
    v.valueType = ConstantValue::T_I64;

    getConstArgs(args, constArgs);
    if( constArgs.size() == 1 ) {
        ConstantValue   v1 = constArgs[0];

        v.valueIsKnown = true;

        int64_t *t = (int64_t *)&v.U64;
        *t = ((int64_t)v1.U32);
    }


    return v;
}

ConstantValue doC16STo32(std::vector<ExpressionPtr> args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    v.width = 32;
    v.valueType = ConstantValue::T_I32;

    getConstArgs(args, constArgs);
    if( constArgs.size() == 1 ) {
        ConstantValue   v1 = constArgs[0];

        v.valueIsKnown = true;

        int32_t *t = (int32_t *)&v.U32;
        *t = ((int32_t)v1.U16);
    }

    return v;
}

ConstantValue doC8STo16(std::vector<ExpressionPtr> args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    v.width = 16;
    v.valueType = ConstantValue::T_I16;

    getConstArgs(args, constArgs);
    if( constArgs.size() == 1 ) {
        ConstantValue v1 = constArgs[0];

        v.valueIsKnown = true;

        int16_t *t = (int16_t *)&v.U16;
        *t = ((int16_t)v1.U8);
    }
     
    return v;
}

ConstantValue doDivModU64to32(std::vector<ExpressionPtr> args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    v.width = 64;
    v.valueType = ConstantValue::T_I64;

    getConstArgs(args, constArgs);
    if( constArgs.size() == 2 ) {
        ConstantValue v1 = constArgs[0];
        ConstantValue v2 = constArgs[1];

        v.valueIsKnown = true;
        assert(v1.width == 64);
        assert(v2.width == 32);
        uint64_t a1 = (uint64_t)v1.U64;
        uint32_t a2 = (uint32_t)v2.U32;
        if( a2 == 0 ) {
            throw StepClientErr("Div by 0");
        }
        uint32_t divRes = a1/a2;
        uint32_t modRes = a1%a2;
        uint64_t final = modRes;
        final = final << 32;
        final += divRes;

        v.U64 = final;
    }
     
    return v;
}

ConstantValue doDivModS64to32(std::vector<ExpressionPtr> args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    v.width = 64;
    v.valueType = ConstantValue::T_I64;

    getConstArgs(args, constArgs);
    if( constArgs.size() == 2 ) {
        ConstantValue v1 = constArgs[0];
        ConstantValue v2 = constArgs[1];

        v.valueIsKnown = true;
        assert(v1.width == 64);
        assert(v2.width == 32);
        int64_t a1 = (int64_t)v1.U64;
        int32_t a2 = (int32_t)v2.U32;
        if( a2 == 0 ) {
            throw StepClientErr("Div by 0");
        }
        int32_t divRes = a1/a2;
        int32_t modRes = a1%a2;
        uint64_t final = modRes;
        final = final << 32;
        final += divRes;

        v.U64 = final;
    }
     
    return v;
}

ConstantValue doC32HITo16(std::vector<ExpressionPtr> args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    v.width = 16;
    v.valueType = ConstantValue::T_I16;
   
    getConstArgs(args, constArgs);
    if( constArgs.size() == 1 ) {
        ConstantValue   v1 = constArgs[0];

        assert(v1.width == 32);

        v.valueIsKnown = true;

        uint32_t t = v1.U32;
        t = t >> 16;
        v.U16 = t;
    }
    
    return v;
}

ConstantValue doC32LOTo16(std::vector<ExpressionPtr> args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    v.width = 16;
    v.valueType = ConstantValue::T_I16;
   
    getConstArgs(args, constArgs);
    if( constArgs.size() == 1 ) {
        ConstantValue   v1 = constArgs[0];

        assert(v1.width == 32);
        uint32_t    a1 = v1.U32;
        uint16_t    vt = a1 & 0xFFFF;

        v.U16 = vt;
    }
    
    return v;
}

ConstantValue doC16HITo8(std::vector<ExpressionPtr> args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    v.width = 8;
    v.valueType = ConstantValue::T_I8;
  
    getConstArgs(args, constArgs);
    if( constArgs.size() == 1 ) {
        ConstantValue   v1 = constArgs[0];

        assert(v1.width == 16);
        uint16_t    t = v1.U16;
        t = t >> 8;
        v.valueIsKnown = true;
        v.U16 = t;
    }
    
    return v;
}

ConstantValue doC16LOTo8(std::vector<ExpressionPtr> args) {
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    v.width = 8;
    v.valueType = ConstantValue::T_I8;
  
    getConstArgs(args, constArgs);
    if( constArgs.size() == 1 ) {
        ConstantValue   v1 = constArgs[0];

        assert(v1.width == 8);
        uint16_t a1 = v1.U16;

        v.U8 = a1 & 0xFF;
    }
    
    return v;
}

ConstantValue doCmpLTS(std::vector<ExpressionPtr> args) {
    ConstantValue   v;
    vector<ConstantValue> constArgs;

    v.valueIsKnown = false;
    v.width = 1;
    v.valueType = ConstantValue::T_I1;
    getConstArgs(args, constArgs);
    if( constArgs.size() == 2 ) {
        ConstantValue   v1 = constArgs[0];
        ConstantValue   v2 = constArgs[1];
        
        v.valueIsKnown = true;

        assert(v1.width == v2.width);
        unsigned short opW = v1.width;
        switch(opW) {
            case 32:
                {
                    int32_t    a1 = ((int32_t)v1.U32);
                    int32_t    a2 = ((int32_t)v2.U32);

                    if( a1 < a2 ) {
                        v.U1 = 1;
                    } else {
                        v.U1 = 0;
                    }
                }
                break;
            case 64:
                {
                    int64_t    a1 = ((int64_t)v1.U64);
                    int64_t    a2 = ((int64_t)v2.U64);

                    if( a1 < a2 ) { 
                        v.U1 = 1;
                    } else {
                        v.U1 = 0;
                    }
                }
                break;
        }

    }

    return v;
}

ConstantValue doCmpLTU(std::vector<ExpressionPtr> args) {
    ConstantValue   v;
    vector<ConstantValue> constArgs;

    v.valueIsKnown = false;
    v.width = 1;
    v.valueType = ConstantValue::T_I1;
    getConstArgs(args, constArgs);
    if( constArgs.size() == 2 ) {
        ConstantValue   v1 = constArgs[0];
        ConstantValue   v2 = constArgs[1];
        
        v.valueIsKnown = true;

        assert(v1.width == v2.width);
        unsigned short opW = v1.width;
        switch(opW) {
            case 32:
                {
                    uint32_t    a1 = v1.U32;
                    uint32_t    a2 = v2.U32;

                    if( a1 < a2 ) {
                        v.U1 = 1;
                    } else {
                        v.U1 = 0;
                    }
                }
                break;
            case 64:
                {
                    uint64_t    a1 = v1.U64;
                    uint64_t    a2 = v2.U64;

                    if( a1 < a2 ) { 
                        v.U1 = 1;
                    } else {
                        v.U1 = 0;
                    }
                }
                break;
        }
    }

    return v;

}

ConstantValue doCmpLES(std::vector<ExpressionPtr> args) {
    ConstantValue   v;
    vector<ConstantValue> constArgs;

    v.valueIsKnown = false;
    v.width = 1;
    v.valueType = ConstantValue::T_I1;
    getConstArgs(args, constArgs);
    if( constArgs.size() == 2 ) {
        ConstantValue   v1 = constArgs[0];
        ConstantValue   v2 = constArgs[1];
        
        v.valueIsKnown = true;

        assert(v1.width == v2.width);
        unsigned short opW = v1.width;
        switch(opW) {
            case 32:
                {
                    int32_t    a1 = ((int32_t)v1.U32);
                    int32_t    a2 = ((int32_t)v2.U32);

                    if( a1 <= a2 ) {
                        v.U1 = 1;
                    } else {
                        v.U1 = 0;
                    }
                }
                break;
            case 64:
                {
                    int64_t    a1 = ((int64_t)v1.U64);
                    int64_t    a2 = ((int64_t)v2.U64);

                    if( a1 <= a2 ) { 
                        v.U1 = 1;
                    } else {
                        v.U1 = 0;
                    }
                }
                break;
        }

    }

    return v;

}

ConstantValue doCmpLEU(std::vector<ExpressionPtr> args) {
    ConstantValue   v;
    vector<ConstantValue> constArgs;

    v.valueIsKnown = false;
    v.width = 1;
    v.valueType = ConstantValue::T_I1;
    getConstArgs(args, constArgs);
    if( constArgs.size() == 2 ) {
        ConstantValue   v1 = constArgs[0];
        ConstantValue   v2 = constArgs[1];
        
        v.valueIsKnown = true;

        assert(v1.width == v2.width);
        unsigned short opW = v1.width;
        switch(opW) {
            case 32:
                {
                    uint32_t    a1 = v1.U32;
                    uint32_t    a2 = v2.U32;

                    if( a1 <= a2 ) {
                        v.U1 = 1;
                    } else {
                        v.U1 = 0;
                    }
                }
                break;
            case 64:
                {
                    uint64_t    a1 = v1.U64;
                    uint64_t    a2 = v2.U64;

                    if( a1 <= a2 ) { 
                        v.U1 = 1;
                    } else {
                        v.U1 = 0;
                    }
                }
                break;
        }

    }

    return v;

}

ConstantValue doC16HLTo32(std::vector<ExpressionPtr> args) {
    ConstantValue   v;
    vector<ConstantValue> constArgs;

    v.valueIsKnown = false;
    v.width = 32;
    v.valueType = ConstantValue::T_I32;
    getConstArgs(args, constArgs);
    if( constArgs.size() == 2 ) {
        ConstantValue   v1 = constArgs[0];
        ConstantValue   v2 = constArgs[1];
        v.valueIsKnown = true;

        assert(v1.width == 16);
        assert(v2.width == 16);

        uint16_t    a1 = v1.U16;
        uint16_t    a2 = v2.U16;
        uint32_t    res = a1;

        res = res << 16;
        res += a2;

        v.U32 = res;
    }

    return v;
}

ConstantValue doC16UTo32(std::vector<ExpressionPtr> args) {
    ConstantValue   v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    v.width = 32;
    v.valueType = ConstantValue::T_I32;
    getConstArgs(args, constArgs);
    if( constArgs.size() == 1 ) {
        ConstantValue   v1 = constArgs[0];

        v.valueIsKnown = true;
        v.U32 = 0;
        v.U32 = v1.U16;
    }

    return v;
}

ConstantValue doC32HLTo64(std::vector<ExpressionPtr> args) { 
    ConstantValue v;
    vector<ConstantValue>   constArgs;

    v.valueIsKnown = false;
    v.width = 64;
    v.valueType = ConstantValue::T_I64;
    getConstArgs(args, constArgs);
    if( constArgs.size() == 2 ) {
        ConstantValue   v1 = constArgs[0];
        ConstantValue   v2 = constArgs[1];

        v.valueIsKnown = true;

        uint64_t    res = v1.U32;
        res = res << 32;
        res += v2.U32;
        v.U64 = res;
    }

    return v;
}
