#include "BasicIR.h"

bool exprsMatch(ExpressionPtr cmpr, ExpressionPtr src) {
    // cmpr is an Expression which potentially contains existential
    // or boolean quantifiers. we need to search 'src' and determine if
    // it matches 'cmpr'
    //
    // strategy: where possible, use recursion
    //

    //cmpr could contain a qualifier. if it does, we need to interpret that
    //qualifier

    if( ExLogicPtr l = boost::dynamic_pointer_cast<ExLogic>(cmpr) ) {
        switch(l->getType()) {
            case ExLogic::And:
                return  exprsMatch(l->getLhs(), src) && 
                        exprsMatch(l->getRhs(), src);
                break;

            case ExLogic::Or:
                return  exprsMatch(l->getLhs(), src) ||
                        exprsMatch(l->getRhs(), src); 
                break;

            case ExLogic::Eq:
                break;

            case ExLogic::Range:
                break;

            case ExLogic::LessThan:
                break;

            case ExLogic::GreaterThan:
                break;
        }
    } else {
        //no logic, so the two expressions are at the same 'level'
        //compare them structurally
        //we should have eliminated any ExRdTmps before getting here, 
        //because comparing them doesn't make much sense

        //ExGet
        if( ExGetPtr cmprG = boost::dynamic_pointer_cast<ExGet>(cmpr) ) {
            if( ExGetPtr srcG = boost::dynamic_pointer_cast<ExGet>(src) ) {
                //are they getting the same register? 
                Register    r1 = cmprG->getSrcReg();
                Register    r2 = srcG->getSrcReg();

                return r1 == r2;
            }
        }

        //ExLoad
        if( ExLoadPtr cmprL = boost::dynamic_pointer_cast<ExLoad>(cmpr) ) {
            if( ExLoadPtr srcL = boost::dynamic_pointer_cast<ExLoad>(src) ) {
                //are they loading the same address?
                ExpressionPtr   e1 = cmprL->getAddr();
                ExpressionPtr   e2 = srcL->getAddr();

                return exprsMatch(e1, e2);
            }
        }

        //ExConst
        if( ExConstPtr cmprC = boost::dynamic_pointer_cast<ExConst>(cmpr) ) {
            if( ExConstPtr srcC = boost::dynamic_pointer_cast<ExConst>(src) ) {
                //are they the same constant?
                ConstantValue   v1 = cmprC->getVal();
                ConstantValue   v2 = srcC->getVal();
                
                if(v1.valueIsKnown && v2.valueIsKnown)
                  return v1 == v2;
                else
                  return true;
            }
        }

        //ExOp
        if( ExOpPtr cmprO = boost::dynamic_pointer_cast<ExOp>(cmpr) ) {
            if( ExOpPtr srcO = boost::dynamic_pointer_cast<ExOp>(src) ) {
                //both the operator and every operand must match
                bool    matched = true;

                if( cmprO->getOp()->getOp() == srcO->getOp()->getOp())  {
                    std::vector<ExpressionPtr>  args1 = cmprO->getArgs();
                    std::vector<ExpressionPtr>  args2 = srcO->getArgs();

                    assert(args1.size() == args2.size());

                    //each operand must match also
                    for(std::vector<ExpressionPtr>::iterator 
                        i1 = args1.begin(), i2 = args2.begin();
                        i1 != args1.end() && i2 != args2.end();
                        ++i1, ++i2)
                    {
                        ExpressionPtr   e1 = *i1;
                        ExpressionPtr   e2 = *i2;

                        matched &= exprsMatch(e1, e2);
                    }

                } else {
                    matched = false;
                }

                return matched;
            }
        }

    }

    return false;
}
