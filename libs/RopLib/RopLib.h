#ifndef _ROPLIB_H
#define _ROPLIB_H
#include <BasicIR.h>
#include <getExec.h>
#include <list>

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

enum VisitorResult {
    Discard,
    Keep,
    Accept
};

/* This is a virtual class that has no meaning to the analysis framework other
 * than "state that I should give to the client". it should be copyable and 
 * deep-movable, because one of these will be maintained for every node in the 
 * state space. Ideally it should also be compact ... 
 */
class CodeExplorationState {
public:
    CodeExplorationState() { }

    virtual CodeExplorationState *clone()=0;
};

typedef boost::shared_ptr<CodeExplorationState> CodeExplorationStatePtr;

/* this is the result function that the explorers give back when we have visited
 * a block with state. It tells us whether or not we should stop the exploration
 * here, whether we should add everything to the set, or whether we should add
 * a subset of everything to the set. 
 */
class CodeExplorationResult {
public:
    CodeExplorationStatePtr     newState;
    bool                        doAll;
    bool                        stop;
    std::set<boost::uint64_t>   addrs;
    VisitorResult               res;

    CodeExplorationResult() : doAll(false), stop(false), res(Discard) { }
};

class RopLibVisitor {
public:
    RopLibVisitor(void) { return; }

    virtual VisitorResult keepBlock(BlockPtr b)=0;
    virtual CodeExplorationStatePtr initialState(void)=0;
    virtual CodeExplorationResult exploreBlock(BlockPtr b, CodeExplorationStatePtr s)=0;
};

typedef boost::shared_ptr<RopLibVisitor> RopLibVisitorPtr;

class RopLibSearcher {
protected:
    secVT                               execCode;
    RopLibVisitorPtr                    visitor;
    std::list<BlockPtr>                 decodedBlocks;
    std::map<boost::uint64_t, BlockPtr> blockMap;
    std::list<std::list<BlockPtr> >     foundBlocks;
    boost::uint64_t                     totalBlocks;
    boost::uint64_t                     translatedBlocks;
    boost::uint64_t                     evaluatedBlocks;
    TargetArch                          tarch;
    void                                *decodeCtx;
    ExecCodeProviderPtr                 codeProvider;
public:
    RopLibSearcher(std::string sourceFile, std::string f, FileFormat fmt, TargetArch t);
    RopLibSearcher(RopLibVisitorPtr v, std::string sourceFile, std::string f, FileFormat fmt, TargetArch t);
    RopLibSearcher(RopLibVisitorPtr v, std::list<BlockPtr>, void *, TargetArch);

    boost::uint64_t getNumBlocks(void) { return this->totalBlocks; }

    boost::uint64_t getBlocksDone(void) { return this->evaluatedBlocks; }

    boost::uint64_t getBlocksLeft(void) { 
      return this->totalBlocks-this->evaluatedBlocks;
    }

    bool needsMoreBlocks(void) { return this->decodedBlocks.size() == 0; }

    bool canSearch(void) { return this->totalBlocks != this->evaluatedBlocks; }

    std::list<BlockPtr> getDecodedBlocks(void) { return this->decodedBlocks; }

    void getBlocks(void);

    void getBlocks(boost::uint32_t blockCount);

    virtual void evalOneBlock(void);

    std::list<std::list<BlockPtr> > getBlocksFound(void) 
        { return this->foundBlocks; }

    secVT getSections(void) { return this->execCode; }
    
    BlockPtr getBlockWithBaseVA(uint64_t VA) {
        BlockPtr    b;
        std::map<boost::uint64_t, BlockPtr>::iterator f = 
            this->blockMap.find(VA);
        if( f != this->blockMap.end() ) {
            b = (*f).second;
        }
        return b;
    }
    std::map<boost::uint64_t, BlockPtr> getBlockMap(void) { return this->blockMap; }
    static std::list<BlockPtr> decodeBlocks(void *, secVT, std::ostream &);
};

typedef boost::shared_ptr<RopLibSearcher>   RopLibSearcherPtr;

/* we'll subclass the RopLibSearcher for each new search tactic that we 
 * think of that doesn't fit into a stateless search. 
 */

class StatefulRopLibSearcher : public RopLibSearcher {
private:
    
public:
    StatefulRopLibSearcher(RopLibVisitorPtr, std::string, std::string, FileFormat, TargetArch);
    StatefulRopLibSearcher(RopLibVisitorPtr, std::list<BlockPtr>, void *, TargetArch);
    virtual void evalOneBlock();
};

#endif
