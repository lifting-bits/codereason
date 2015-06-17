#include "BasicIR.h"

using namespace boost;
using namespace std;

list<Transfer> Block::getTransfers(void) {
    return this->transfer;
}

/*Transfer Block::getTransferForReg(Register r) {
    Transfer t;
    
    //lookup the register in our list 
    list<Transfer>::iterator it = this->transfer.begin();
    while( it != this->transfer.end() ) {
        Transfer    k = *it;

        if( k.first == r ) {
            t = k;
            break;
        }
        ++it;
    }

    //return it

    return t;
}*/
