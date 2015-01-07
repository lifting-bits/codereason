#include "RopLib_internal.h"

#include <math.h>
#include <boost/regex.hpp>

using namespace std;
using namespace boost;

static
void print_progress(double Total, double Done, ostream &s) {
    const int tdot = 40;

    double pctDone = Done/Total;

    int print = floor(pctDone * tdot);

    int i = 0;
    s << setprecision(5) << fixed << pctDone*100 << "[";
    for( ; i < print; i++ ) {
        s << "=";
    }
    for( ; i < tdot; i++ ) {
        s << " ";
    }

    s << "]\r";
    s.flush();

    return;
}

RopLibSearcher::RopLibSearcher( string        sourceFile,
                                string        fnSearch,
                                FileFormat    fmt,
                                TargetArch    t,
                                unsigned int  m) :
    codeProvider(new ExecCodeProvider(sourceFile, fmt, t)),
    tarch(t),
    totalBlocks(0),
    maxNumStatements(m)
{
    TargetInfo  ti;
    //also, initialize the VEX context
    if( tarch.ta == X86 ) {
        ti.guestHWcaps =
            VEX_HWCAPS_X86_SSE1|VEX_HWCAPS_X86_SSE2|VEX_HWCAPS_X86_SSE3;
        ti.hostHWcaps =
            VEX_HWCAPS_X86_SSE1|VEX_HWCAPS_X86_SSE2|VEX_HWCAPS_X86_SSE3;
    } else if( tarch.ta == ARM ) {
        ti.guestHWcaps = 5|VEX_HWCAPS_ARM_VFP3;
        ti.hostHWcaps =
            VEX_HWCAPS_X86_SSE1|VEX_HWCAPS_X86_SSE2|VEX_HWCAPS_X86_SSE3;
    } else {
        ti.guestHWcaps = 0;
        ti.hostHWcaps = 0;
    }

    ti.maxInstructions = 99;
    ti.chaseThreshold = 10;
    ti.opLevel = FullOpt;
    ti.tarch = tarch;

    this->decodeCtx = initDecodeLib(ti, true, false);

    assert(this->decodeCtx != NULL); 
    //initialize the execCode fields by searching through all the file 
    //names in codeProvider, comparing them to a regex
    string  filter;
    if( fnSearch.size() > 0 ) {
        filter = fnSearch;
    } else {
        filter = ".*";
    }

    regex   regexFilter(filter);
    smatch  sm;
    
    list<string>    names = this->codeProvider->filenames();
    for( list<string>::iterator it = names.begin(); it != names.end(); ++it ) {
        string  curName = *it;
        if( regex_search(curName, sm, regexFilter) ) {
            secVT   tmp = this->codeProvider->sections_in_file(curName);
            
            this->execCode.insert(this->execCode.end(), tmp.begin(), tmp.end());
        }
    }

    return;
}

RopLibSearcher::RopLibSearcher( RopLibVisitorPtr    v,
                                list<BlockPtr>  blocks,
                                void            *ctx,
                                TargetArch      t,
                                unsigned int    m) :
    decodedBlocks(blocks),
    visitor(v),
    tarch(t),
    totalBlocks(blocks.size()),
    decodeCtx(ctx),
    translatedBlocks(blocks.size()),
    evaluatedBlocks(0),
    maxNumStatements(m)
{
  //populate the BlockMap based on the blocks object we were given 
  for(list<BlockPtr>::iterator it = blocks.begin(), e = blocks.end();
      it != e;
      ++it)
  {
    BlockPtr  B = *it;
    boost::uint64_t addr = B->getBlockBase();
    this->blockMap.insert(pair<boost::uint64_t,BlockPtr>(addr, B)); 
  }

  return;
}

RopLibSearcher::RopLibSearcher( RopLibVisitorPtr    v, 
                                string          sourceFile,
                                string          fnSearch, 
                                FileFormat      fmt,
                                TargetArch      t,
                                unsigned int    m) :
    /*execCode(getExecSections(fn, fmt, t)),*/
    codeProvider(new ExecCodeProvider(sourceFile, fmt, t)),
    visitor(v),
    tarch(t),
    translatedBlocks(0),
    maxNumStatements(m)
{
    this->totalBlocks = 0;
    TargetInfo  ti;
    //also, initialize the VEX context
    if( tarch.ta == X86 ) {
        ti.guestHWcaps =
            VEX_HWCAPS_X86_SSE1|VEX_HWCAPS_X86_SSE2|VEX_HWCAPS_X86_SSE3;
        ti.hostHWcaps =
            VEX_HWCAPS_X86_SSE1|VEX_HWCAPS_X86_SSE2|VEX_HWCAPS_X86_SSE3;
    } else if( tarch.ta == ARM ) {
        ti.guestHWcaps = 5|VEX_HWCAPS_ARM_VFP3;
        ti.hostHWcaps =
            VEX_HWCAPS_X86_SSE1|VEX_HWCAPS_X86_SSE2|VEX_HWCAPS_X86_SSE3;
    } else {
        ti.guestHWcaps = 0;
        ti.hostHWcaps = 0;
    }

    ti.maxInstructions = 99;
    ti.chaseThreshold = 10;
    ti.opLevel = FullOpt;
    ti.tarch = tarch;

    this->decodeCtx = initDecodeLib(ti, true, false);

    assert(this->decodeCtx != NULL); 
    //initialize the execCode fields by searching through all the file 
    //names in codeProvider, comparing them to a regex
    string  filter;
    if( fnSearch.size() > 0 ) {
        filter = fnSearch;
    } else {
        filter = ".*";
    }

    regex   regexFilter(filter);
    smatch  sm;
    
    list<string>    names = this->codeProvider->filenames();
    for( list<string>::iterator it = names.begin(); it != names.end(); ++it ) {
        string  curName = *it;

        if( regex_search(curName, sm, regexFilter) ) {
            secVT   tmp = this->codeProvider->sections_in_file(curName);

            this->execCode.insert(this->execCode.end(), tmp.begin(), tmp.end());
            for(secVT::iterator sit = tmp.begin(); sit != tmp.end(); ++sit)
            {
              secAndArchT i = *sit;
              secPT       j = i.second;
              lenAddrT    k = j.second;
              this->totalBlocks += k.first;
            }
        }
    }

    return;
}

list<BlockPtr> RopLibSearcher::decodeBlocks(void *c, secVT code, ostream &s, unsigned int maxNumStatements) {
    list<BlockPtr>  blocks;
    //we have a vector of executable code
    //translate this into a vector of BlockPtrs
    int curSec = 1;
    for(secVT::iterator it = code.begin(); it != code.end(); ++it) {
        secAndArchT saa = *it;
        TargetArch arch = saa.first;
        secPT secAndLen = saa.second;
        lenAddrT len = secAndLen.second;
        uint8_t *buf = secAndLen.first;
        uint64_t baseAddr = len.second;
        uint32_t bufLen = len.first;

        s << "sec " << curSec << " of " << code.size() << endl;
        curSec++;
        for( int i = 0; i < bufLen; i++ ) {
            uint8_t *curBuf = buf+i;
            uint64_t baseVA = baseAddr + i;
            BlockPtr    b;
            bool        r;
            
            r = convertToOneBlock(  c,
                                    curBuf,
                                    bufLen-i,
                                    baseVA,
                                    arch,
                                    maxNumStatements,
                                    b);

            if( r ) {
                blocks.push_front(b);
            }

            print_progress(bufLen, i, s);
        }
    }

    s << endl;

    return blocks;
}

//get at most count blocks
bool RopLibSearcher::getBlocks(uint32_t count) 
{
  uint32_t  soFar = 0;
  //we should start at the last place we left off, this is told to us
  //by the translatedBlocks field member. it tells us the count of the
  //number of blocks translated so far

  uint8_t   *curBuf = NULL;
  uint64_t  curBaseAddr = 0;
  uint32_t  curBufLen = 0;
  uint32_t  start = 0;

  for(secVT::iterator it = this->execCode.begin();
      it != this->execCode.end();
      ++it)
  {
      secAndArchT saa = *it;
      TargetArch arch = saa.first;
      secPT secAndLen = saa.second;
      lenAddrT len = secAndLen.second;
      uint8_t *buf = secAndLen.first;
      uint64_t baseAddr = len.second;
      uint32_t bufLen = len.first;

      if(this->translatedBlocks >= soFar)
      {
        if(this->translatedBlocks < (soFar+bufLen))
        {
          //figure out what the offset is
          ptrdiff_t delta = this->translatedBlocks - soFar;
          curBuf = buf;
          curBaseAddr = baseAddr;
          curBufLen = bufLen;
          start = delta;
          break;
        }
      }

      soFar += bufLen;
  }

  //assert(curBuf != NULL && curBufLen != 0);
  if(curBuf == NULL && curBufLen == 0) {
    cout << "curBuf is null, this is bad" << endl;
    return false;
  }
  assert(start < curBufLen);

  //cout << "retreiving up to " << count << " blocks" << endl;
  //now, go until we either run out of elts in the current buffer, or, 
  //we get up to count blocks
  uint32_t  blocksDone = 0;
  for( uint32_t i = start; i < curBufLen; i++ ) {
    uint8_t     *c = curBuf + i;
    uint64_t    baseVA = curBaseAddr + i;
    BlockPtr    b;
    bool        r;

    if(blocksDone == count) {
      break;
    }
     
    r = convertToOneBlock(  this->decodeCtx,
                            c,
                            curBufLen-i,
                            baseVA,
                            tarch,
                            this->maxNumStatements,
                            b);

    //we update this because we at least made an attempt at translation
    this->translatedBlocks++;
    if( r ) {
        //and we update THIS, HERE, so that we actually get the number
        //of blocks that we assert we want
        blocksDone++;
        assert(b.get() != NULL);
        this->decodedBlocks.push_front(b);
        //this->blockMap.insert(pair<uint64_t,BlockPtr>(baseVA, b));
    } else {
        //this isn't actually a block, so take our total down
        this->totalBlocks--;
    }

    //print_progress(count, blocksDone, cout);
  }

  //cout << endl;

  return true;
}

void RopLibSearcher::getBlocks(void) {
    //we have a vector of executable code
    //translate this into a vector of BlockPtrs
    int curSec = 1;
    for(secVT::iterator it = this->execCode.begin();
        it != this->execCode.end();
        ++it)
    {
        secAndArchT saa = *it;
        TargetArch arch = saa.first;
        secPT secAndLen = saa.second;
        lenAddrT len = secAndLen.second;
        uint8_t *buf = secAndLen.first;
        uint64_t baseAddr = len.second;
        uint32_t bufLen = len.first;

        cout << "sec " << curSec << " of " << this->execCode.size() << endl;
        curSec++;
        for( int i = 0; i < bufLen; i++ ) {
            uint8_t *curBuf = buf+i;
            uint64_t baseVA = baseAddr + i;
            BlockPtr    b;
            bool        r;
            
            r = convertToOneBlock(  this->decodeCtx,
                                    curBuf,
                                    bufLen-i,
                                    baseVA,
                                    tarch,
                                    this->maxNumStatements,
                                    b);

            if( r ) {
                this->decodedBlocks.push_front(b);
                this->translatedBlocks++;
                this->blockMap.insert(pair<uint64_t,BlockPtr>(baseVA, b));
            }

            print_progress(bufLen, i, cout);
        }
    }

    cout << endl;

    return;
}

void RopLibSearcher::evalOneBlock(void) {
    //get the next block off of the head of the list
    assert(this->decodedBlocks.size() > 0);
    BlockPtr    curBlock = this->decodedBlocks.front();

    assert(curBlock.get() != NULL);
    this->decodedBlocks.pop_front();
    this->evaluatedBlocks++;

    //check and see if our visitor likes this block
    VisitorResult   visitRes = this->visitor->keepBlock(curBlock);

    list<BlockPtr>  foundBlocks;
    
    if( visitRes == Accept || visitRes == Keep ) {
        foundBlocks.push_back(curBlock);
    }

    //our visitor will step the block or otherwise analyze it 
    //and when we come back, we can check it 

    BlockPtr    checkBlock = curBlock;
    while( visitRes == Keep ) {
        ExpressionPtr   next = checkBlock->getNext();
        //if we can, see if we can get a block from that target address
        ConstantValue   nextPtr = getValue(next);

        //if we can, overwrite checkBlock with that and ask the visitor 
        //if it is appropriate
        if( nextPtr.valueIsKnown ) { 
            uint64_t    VA;
            assert(nextPtr.width == 32 || nextPtr.width == 64);

            switch(nextPtr.width) {
                case 32:
                    VA = nextPtr.U32;
                    break;
                
                case 64:
                    VA = nextPtr.U64;
                    break;
            }

            checkBlock = this->getBlockWithBaseVA(VA);
            
            if( checkBlock ) {
                foundBlocks.push_back(checkBlock);
                visitRes = this->visitor->keepBlock(checkBlock);
            } else {
                visitRes = Discard;
            }
        } else {
            visitRes = Discard;
        }
    }

    if( visitRes == Accept ) {
        //store this block away
        this->foundBlocks.push_front(foundBlocks);
    }

    return;
}

StatefulRopLibSearcher::StatefulRopLibSearcher( RopLibVisitorPtr   v,
                                                string sourceFile, 
                                                string f, 
                                                FileFormat fmt, 
                                                TargetArch arch,
                                                unsigned int m) :
                                   RopLibSearcher(v, sourceFile, f, fmt, arch, m) 
{
    
    return;
}

StatefulRopLibSearcher::StatefulRopLibSearcher( RopLibVisitorPtr    v,
                                list<BlockPtr>  blocks,
                                void            *ctx,
                                TargetArch      t,
                                unsigned int    m) :
                                RopLibSearcher(v, blocks, ctx, t, m)
{
  return;
}
 
static
void addResultsToGraph( map<uint64_t, BlockPtr>             &blockMap,
                        CodeExplorationResult               &res, 
                        ROPGraph::vertex_descriptor         &curVert,
                        ROPGraph                            &g,
                        set<uint64_t>                       &visited,
                        stack<ROPGraph::vertex_descriptor>  &unvisited)
{
    set<BlockPtr>   blockResultsToAdd;

    if(res.doAll)
    {
        /* add all block VAs from the environment, as long as they are not 
         * in visited.
         */
        for(map<uint64_t, BlockPtr>::iterator it = blockMap.begin();
            it != blockMap.end();
            ++it)
        {
            BlockPtr    b = (*it).second;
            
            if(visited.find(b->getBlockBase()) == visited.end())
            {
                blockResultsToAdd.insert(b); 
            }
        }
    }
    else
    {
        /* add only the blocks in the set from the result, if non-empty,
         * and also if they are not in visited
         */
        set<uint64_t>   VAs = res.addrs;
        for(set<uint64_t>::iterator it = VAs.begin(); it != VAs.end(); ++it)
        {
            map<uint64_t, BlockPtr>::iterator   k = blockMap.find(*it);
            
            /* tools should only give us VAs of blocks that they know are in 
             * the map 
             */

            if(k != blockMap.end())
            {
              BlockPtr    b = (*k).second;
              blockResultsToAdd.insert(b);
            }
        }
    }

    /* fast-path return, why not */
    if(blockResultsToAdd.size() == 0)
        return;

    /* add each of these as vertices to the graph and connect them to the 
     * current vertex
     */
    for(set<BlockPtr>::iterator it = blockResultsToAdd.begin();
        it != blockResultsToAdd.end();
        ++it)
    {
        BlockPtr                    b = *it;
        ROPGraph::vertex_descriptor rv = add_vertex(g);

        unvisited.push(rv);
        g[rv].block = b;
        g[rv].state = res.newState;

        add_edge(curVert, rv, g);
    }

    return;
}

set<uint64_t>
populateVisited(ROPGraph::vertex_descriptor &curV, ROPGraph &g)
{
    set<uint64_t>               visited;
    ROPGraph::vertex_descriptor v = curV;

    pair<ROPGraph::in_edge_iterator, ROPGraph::in_edge_iterator> p = 
        in_edges(v, g);
    
    BlockPtr    curBlock = g[v].block;
    visited.insert(curBlock->getBlockBase());

    while(p.first != p.second)
    {
        v = source(*p.first, g);
        curBlock = g[v].block;
        visited.insert(curBlock->getBlockBase());
        p = in_edges(v, g);
    }

    return visited;
}

list<BlockPtr>
buildBlockChain(ROPGraph::vertex_descriptor &begV, ROPGraph &g)
{
    list<BlockPtr>  blocks;
    ROPGraph::vertex_descriptor v = begV;

    pair<ROPGraph::in_edge_iterator, ROPGraph::in_edge_iterator> p = 
        in_edges(v, g);
    
    blocks.push_front(g[v].block);

    while(p.first != p.second)
    {
        v = source(*p.first, g);
        blocks.push_front(g[v].block);
        p = in_edges(v, g);
    }


    return blocks;
}

void StatefulRopLibSearcher::evalOneBlock(void)
{
    /* This is where the bulk of our tactic is implemented. 
     * We have two lists: 
     *   A list of blocks that we are evaluating as the 'beginning' of the 
     *   potential gadget sequence
     *
     *   A map of all blocks that are available, indexed by their VA
     *
     *   We'll take the next block off the top of the first list and check to
     *   see if it meshes with what our current view of what program state 
     *   should be.
     */
    assert(this->decodedBlocks.size() > 0);

    BlockPtr    firstBlock = this->decodedBlocks.front();
    this->decodedBlocks.pop_front();
    this->evaluatedBlocks++;

    ROPGraph                            g;
    ROPGraph::vertex_descriptor         ropRoot = add_vertex(g);
    ROPGraph::vertex_descriptor         curRoot = ropRoot;
    stack<ROPGraph::vertex_descriptor>  unvisitedVerts;
    CodeExplorationStatePtr             firstState = 
        this->visitor->initialState();

    g[ropRoot].block = firstBlock;
    g[ropRoot].state = firstState;

    /* start by asking for the first result */

    CodeExplorationResult   firstRes = 
      this->visitor->exploreBlock(firstBlock, firstState);
    
    /* we could accept on the first block, in which case, don't do anything
     * more... */
    if(firstRes.res == Accept) 
    {
      list<BlockPtr>  foundChain;

      foundChain.push_back(firstBlock);
      this->foundBlocks.push_back(foundChain);
      return;
    }

    if(firstRes.res == Discard) 
    {
      return;
    }

    set<uint64_t>           n;
    map<uint64_t, BlockPtr> bm = this->getBlockMap();
    addResultsToGraph(bm, firstRes, ropRoot, g, n, unvisitedVerts);

    /* so this always terminates because we never add something to the graph 
     * twice on one path (no loops), for better or for worse. 
     */
    while(unvisitedVerts.size() > 0)
    {
        set<uint64_t>               visitedVAs;
        ROPGraph::vertex_descriptor curV = unvisitedVerts.top();
        unvisitedVerts.pop();
       
        BlockPtr                b = g[curV].block;
        CodeExplorationStatePtr s = g[curV].state;
        
        /* visitedVAs is the set of blocks in the path from the root to 
         * curV
         */
        visitedVAs = populateVisited(curV, g);

        /* do the query for this vertex */
        CodeExplorationResult   r = this->visitor->exploreBlock(b, s);

        if(r.stop) //client has gone for too long
          break;

        if(r.res == Keep)
        {
          /* add the results to the graph, and keep going */
          addResultsToGraph(bm, r, curV, g, visitedVAs, unvisitedVerts);
        }
        else if(r.res == Accept)
        {
          /* stash this path off to the side, and then don't add anything 
           * more to the graph from this node. we just found a matching chain!
           */
          list<BlockPtr>  foundChain = buildBlockChain(curV, g);
          this->foundBlocks.push_back(foundChain);
        }
    }

    return;
}
