#include "RopLib.h"
#include <decodeLib.h>
#include <stack>

/* these are some graph-theoretic definitions that only have meaning to the 
 * state explorer. 
 */

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>

struct VP
{
    /* block of code at this point */
    BlockPtr                block;
    /* current abstract state at this point */
    CodeExplorationStatePtr state;
};

typedef boost::adjacency_list<  boost::vecS,
                                boost::vecS,
                                boost::bidirectionalS,
                                VP>
            ROPGraph;

extern "C" {
#include <vex/libvex_basictypes.h>
#include <vex/libvex_guest_offsets.h>
#include <vex/libvex_emnote.h>
#include <vex/libvex.h>
}
