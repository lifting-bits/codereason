#include <getExec.h>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <VEE.h>
#include <decodeLib.h>
#include <RopLib.h>
#include <ExprParser.h>

namespace udis {
#include <udis86.h>
}

#include <boost/program_options/config.hpp>
#include <boost/unordered_map.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/regex.hpp>

#include <llvm/MC/MCDisassembler.h>
#include <llvm/MC/MCAsmInfo.h>
#include <llvm/MC/MCInst.h>
#include <llvm/MC/MCInstPrinter.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/MemoryObject.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ADT/OwningPtr.h>

using namespace boost;
using namespace boost::program_options;
using namespace std;

//for disassembling
class BufferMemoryObject : public llvm::MemoryObject {
private:
    const uint8_t *Bytes;
    uint64_t Length;
public:
    BufferMemoryObject(const uint8_t *bytes, uint64_t length) :
        Bytes(bytes), Length(length) { }

    uint64_t getBase() const { return 0; }
    uint64_t getExtent() const { return Length; }

    int readByte(uint64_t addr, uint8_t *byte) const {
        if (addr > getExtent())
            return -1;
        *byte = Bytes[addr];
        return 0;
    }
};

class BlankBlockProvider : public BlockProvider {
public:
    BlankBlockProvider() { return; }
       virtual BlockPtr getNextBlock(boost::uint64_t VA);
};

typedef shared_ptr<BlankBlockProvider> BlankBlockProviderPtr;
BlockPtr BlankBlockProvider::getNextBlock(boost::uint64_t VA) {
    BlockPtr    b;

    return b;
}

class MatchesVEE : public RopLibVisitor {
    Condition       *cond;
    unsigned int    followCount;
    unsigned int    maxFollow;
public:
    virtual VisitorResult keepBlock(BlockPtr b);
    virtual CodeExplorationResult exploreBlock(BlockPtr b, CodeExplorationStatePtr p);
    virtual CodeExplorationStatePtr initialState(void);

    MatchesVEE(Condition *c, unsigned int max) :    cond(c),
                                                    followCount(0),
                                                    maxFollow(max)
    {
        applyPreConditions(this->cond->getState(), this->cond);
        return;
    }
};

CodeExplorationStatePtr MatchesVEE::initialState(void)
{
    return CodeExplorationStatePtr();
}

CodeExplorationResult MatchesVEE::exploreBlock(BlockPtr b, CodeExplorationStatePtr p)
{
    return CodeExplorationResult();
}

VisitorResult MatchesVEE::keepBlock(BlockPtr b) {
    VisitorResult           r = Discard;
   
    //early check to see if we have gone too deep
    if( this->followCount > this->maxFollow ) {
        this->followCount = 0;
        return r;
    } else {
        this->followCount++;
    }

    //get us some state
    VexExecutionStatePtr    vssFromCond = this->cond->getState();
    BlankBlockProviderPtr   bpp(new BlankBlockProvider());

    //make a copy of the 'blank' VSS environment
    VexExecutionStatePtr    vss =
        VexExecutionStatePtr(new VexExecutionState(*vssFromCond.get()));
    VexExecutionEngine      vee(bpp, b, vss);

    VexExecutionEngine::StepState s = VexExecutionEngine::StepOK;
    while( s == VexExecutionEngine::StepOK ) {
        s = vee.step();
    }

    if( s == VexExecutionEngine::StepEnd ) {
        //check and see if we met conditions
        if( checkPostConditions(vss, this->cond) ) {
            r = Accept;
            this->followCount = 0;
        } else {
            r = Keep;
        }
    }

    return r;
}

typedef pair<string, vector<StatementPtr> >    nameAndRule;

class RuleMatchingVisitor : public RopLibVisitor {
private:
    vector<nameAndRule>    rules;
public:

    RuleMatchingVisitor(vector<nameAndRule> r) : rules(r) { }

    virtual VisitorResult keepBlock(BlockPtr b);
    virtual CodeExplorationResult exploreBlock(BlockPtr b, CodeExplorationStatePtr p);
    virtual CodeExplorationStatePtr initialState(void);

};

typedef pair<Register,Register> regPair;

CodeExplorationStatePtr RuleMatchingVisitor::initialState(void)
{
    return CodeExplorationStatePtr();
}

CodeExplorationResult RuleMatchingVisitor::exploreBlock(BlockPtr b, CodeExplorationStatePtr p)
{
    return CodeExplorationResult();
}

VisitorResult RuleMatchingVisitor::keepBlock(BlockPtr b) {
    VisitorResult       r = Discard;
    //get the transfers for this specific block 
    list<Transfer> block_transfers = b->getTransfers();

    //iterate over every rule that we have
    //if any of these rules hold, then we accept
    for(vector<nameAndRule>::iterator it = this->rules.begin();
        it != this->rules.end();
        ++it)
    {  
        nameAndRule             nr = *it;
        vector<StatementPtr>    these_rules = nr.second;
        bool                    all_accepted = true;

        //all of these rules must hold for us to accept
        for(vector<StatementPtr>::iterator sit = these_rules.begin();
            sit != these_rules.end();
            ++sit)
        {
            StatementPtr    s = *sit;
            bool            broken = false;
            bool            tested = false;

            for(list<Transfer>::iterator   tit = block_transfers.begin();
                tit != block_transfers.end();
                ++tit)
            {
                Transfer        t = *tit;
                SinkType        sink = t.first;

                //if our rules statement is a put or a store
                if( StPutPtr p = dynamic_pointer_cast<StPut>(s) ) {
                    //a put rule matches with a reg sink
                    if( sink.isReg() ) {
                        Register        ruleReg = p->getDstRegister();
                        ExpressionPtr   ruleData = p->getData();
                        Register        tstReg = sink.getReg();
                        ExpressionPtr   tstData = t.second;

                        if( ruleReg == tstReg ) {
                            tested = true;
                            if( !exprsMatch(ruleData, tstData) ) {
                                all_accepted = false;
                                broken = false;
                                break;
                            }
                        }
                    }

                } else if(StStorePtr st = dynamic_pointer_cast<StStore>(s)) {
                    //a store rule matches with a mem sink
                    if( sink.isMem() ) {
                        ExpressionPtr   ruleAddr = st->getAddr();
                        ExpressionPtr   ruleData = st->getData();
                        ExpressionPtr   tstAddr = sink.getMem();
                        ExpressionPtr   tstData = t.second;

                        tested = true;
                        if( !exprsMatch(ruleAddr, tstAddr) ||
                            !exprsMatch(ruleData, tstData)) {
                            all_accepted = false;
                            broken = true;
                            break;
                        }
                    }
                }
            }

            if( tested == false ) {
                all_accepted = false;
                broken = false;
            }

            if( broken ) {
                break;
            }
        }

        if( all_accepted ) {
            r = Accept;
            break;
        }
    }

    return r;
}

struct SinkHash : unary_function<SinkType, size_t>
{
    size_t operator()(SinkType const &x) const
    {
      if(x.isReg()) 
      {
        Register  r = x.getReg();
        switch(r.arch.ta)
        {
          case X86:
            switch(r.width)
            {
              case 8:
                return (8<<16)+r.Reg8;
                break;
              case 16:
                return (16<<16)+r.Reg16;
                break;
              case 32:
                return (32<<16)+r.Reg32;
                break;
              case 64:
                return (64<<16)+r.Reg64;
                break;
            }
            break;
          case AMD64:
            switch(r.width)
            {
              case 8:
                return (8<<16)+r.Reg8;
                break;
              case 16:
                return (16<<16)+r.Reg16;
                break;
              case 32:
                return (32<<16)+r.Reg32;
                break;
              case 64:
                return (64<<16)+r.Reg64;
                break;
            } 
            break;

          case ARM:
            return r.RegArm;
            break;
        }
      }
      else
      {
        ExpressionPtr addr = x.getMem();
        return ((size_t)addr.get());
      }
    }
};


//symbolic state is a mapping of sinks to expressions as they have been
//observed so far
class SymbolicState : public CodeExplorationState {
  typedef unordered_map<SinkType, ExpressionPtr, SinkHash>  StateMap;

  StateMap  state;
  int       jumps;

  ExpressionPtr updateExpr(SinkType, ExpressionPtr, ExpressionPtr);
public:
  SymbolicState(void) : jumps(0), CodeExplorationState() { }

  virtual CodeExplorationState  *clone(void);

  void applyTransfers(list<Transfer>  &);

  int getJumps(void) { return this->jumps; }
  void incJumps(void) { this->jumps++; }

  string dump(void);

  bool stateMatches(vector<nameAndRule> &);
};

typedef boost::shared_ptr<SymbolicState>  SymbolicStatePtr;

class StatefulRuleMatchingVisitor : public RopLibVisitor {
private:
    vector<nameAndRule> rules;
    int                 maxJumps;

    //bool stateMatches(SymbolicStatePtr);
    set<boost::uint64_t>  getNextAddrs(BlockPtr B, SymbolicStatePtr);
public:

    StatefulRuleMatchingVisitor(vector<nameAndRule> r) : maxJumps(5), rules(r) { }

    virtual VisitorResult keepBlock(BlockPtr b);
    virtual CodeExplorationResult exploreBlock(BlockPtr b, CodeExplorationStatePtr p);
    virtual CodeExplorationStatePtr initialState(void);
};
    
set<boost::uint64_t> StatefulRuleMatchingVisitor::getNextAddrs(BlockPtr B, 
                                                            SymbolicStatePtr S)
{
  set<boost::uint64_t>  next;

  //sweep through the block for all of the exit statements, since each
  //block is single-entry, multiple-exit
  for(vector<StatementPtr>::iterator it = B->begin(), e = B->end();
      it != e;
      ++it)
  {
    StatementPtr  S = *it;
  }

  //let's look at the exit conditions of B and figure out what those are
  if(B->getExitKind() == Fallthrough) 
  {
    ExpressionPtr nextE = B->getNext();
    ConstantValue V = getValue(nextE);
    if(V.valueIsKnown)
    {
      uint32_t  a = V.getValue<uint32_t>();
      next.insert(a);
    }
  }

  return next;
}

//descend through exp and see if it reads from S
static
bool containsRead(ExpressionPtr exp, SinkType S)
{
  //what is exp?
  if(ExGetPtr get = dynamic_pointer_cast<ExGet>(exp))
  {
    //check the register against SinkType if SinkType is appropriate
    if(S.isReg())
      return S.getReg() == get->getSrcReg();
  }
  else if(ExOpPtr op = dynamic_pointer_cast<ExOp>(exp))
  {
    vector<ExpressionPtr> args = op->getArgs();
    //recurse into all of the operands 
    for(vector<ExpressionPtr>::iterator it =args.begin();
        it != args.end();
        ++it)
    {
      ExpressionPtr opArg = *it;
      
      if(containsRead(opArg, S))
        return true;
    }
  }
  else if(ExLoadPtr load = dynamic_pointer_cast<ExLoad>(exp))
  {
    //recurse into the address operand
    return containsRead(load->getAddr(), S);
  }
  else if(ExMux0XPtr mux = dynamic_pointer_cast<ExMux0X>(exp))
  {
    //recurse into the operands
    return  containsRead(mux->getCondition(), S) ||
            containsRead(mux->getTrue(), S) ||
            containsRead(mux->getFalse(), S);
  }

  return false;
}

static
ExpressionPtr replaceWith(SinkType S, ExpressionPtr O, ExpressionPtr N)
{
  ExpressionPtr newExpr = O;

  //recurse through to find the use of S and replace it with N
  if(ExGetPtr get = dynamic_pointer_cast<ExGet>(O))
  {
    //check the register against SinkType if SinkType is appropriate
    if(S.isReg() && get->getSrcReg() == S.getReg())
    {
      //it is a read, replace with N
      newExpr = N;
    }
  }
  else if(ExOpPtr op = dynamic_pointer_cast<ExOp>(O))
  {
    vector<ExpressionPtr> newOps;
    //recurse into all of the operands, replace each operand with the
    //result of recursively evaluating ourselves on that operand 
    vector<ExpressionPtr> args = op->getArgs();
    for(vector<ExpressionPtr>::iterator it = args.begin();
        it != args.end();
        ++it)
    {
      ExpressionPtr c = *it;
      ExpressionPtr n = replaceWith(S, c, N);
      newOps.push_back(n);
    }

    newExpr = ExpressionPtr(new ExOp(op->getOp(), newOps));
  }
  else if(ExLoadPtr load = dynamic_pointer_cast<ExLoad>(O))
  {
    //set the address to the result of calling us on the value
    ExpressionPtr la = load->getAddr();
    newExpr = ExpressionPtr(new ExLoad(replaceWith(S, la, N), load->getTy()));
  }
  else if(ExMux0XPtr mux = dynamic_pointer_cast<ExMux0X>(O))
  {
    //recurse into the operands
    ExpressionPtr cond = replaceWith(S, mux->getCondition(), N);
    ExpressionPtr tr = replaceWith(S, mux->getTrue(), N);
    ExpressionPtr fa = replaceWith(S, mux->getFalse(), N);

    newExpr = ExpressionPtr(new ExMux0X(cond, tr, fa));
  }

  return newExpr;
}

string SymbolicState::dump(void)
{
  string  r = "";
  //print out the symbolic state by iterating over every key
  for(StateMap::const_iterator k = this->state.cbegin(), e = this->state.cend();
      k != e;
      ++k)
  {
    SinkType      S = (*k).first;
    ExpressionPtr E = (*k).second;

    r = r + "m[";
    //dump the SinkType
    if(S.isReg())
    {
      Register  reg = S.getReg();
      r = r + regToStr(reg, reg.arch);
    }
    else if(S.isMem())
    {
      ExpressionPtr mem = S.getMem();
      r = r + mem->printExpr();
    }

    r = r + "]=";

    //dump the Expression
    r = r + E->printExpr();
    r = r + "\n";
  }
  
  return r;
}

ExpressionPtr SymbolicState::updateExpr(SinkType      Sink, 
                                        ExpressionPtr OldExpr, 
                                        ExpressionPtr NewExpr)
{
  ExpressionPtr updatedExpr = NewExpr;

  if(containsRead(NewExpr, Sink))
    updatedExpr = replaceWith(Sink, OldExpr, NewExpr);
  
  return updatedExpr;
}

CodeExplorationState *SymbolicState::clone(void)
{
  SymbolicState *newS = new SymbolicState();

  //just copy the state over
  newS->state = this->state;
  newS->jumps = this->jumps;

  return newS;
}

bool SymbolicState::stateMatches(vector<nameAndRule>  &rules)
{
  //here, we can just check and see if our current symbolic state 
  //matches the rules that we have

  //cout << "stateMatches: dumping state" << endl;
  //cout << endl << this->dump() << endl << endl;

  for(vector<nameAndRule>::iterator it = rules.begin();
        it != rules.end();
        ++it)
  {  
    nameAndRule             nr = *it;
    vector<StatementPtr>    these_rules = nr.second;
    bool                    all_accepted = true;

    //all of these rules must hold for us to accept
    for(vector<StatementPtr>::iterator sit = these_rules.begin();
        sit != these_rules.end();
        ++sit)
    {
      StatementPtr  s = *sit;
      bool          broken = false;
      bool          tested = false;

      if( StPutPtr p = dynamic_pointer_cast<StPut>(s) ) {
        //look up the target of the put in the state map 
        SinkType            sink(p->getDstRegister());
        StateMap::iterator  k = this->state.find(sink);

        if(k != this->state.end())
        {
          ExpressionPtr expFromState = (*k).second;
          ExpressionPtr expFromRule = p->getData();
          
          //cout << expFromState->printExpr() << endl;
          //cout << expFromRule->printExpr() << endl;

          tested = true;
          if(!exprsMatch(expFromState, expFromRule))
          {
            all_accepted = false;
            broken = false;
            break;
          }
        }
      } else if(StStorePtr st = dynamic_pointer_cast<StStore>(s)) {
        //look up the target of the store in the state map
      }

      if( tested == false ) {
          all_accepted = false;
          broken = false;
      }

      if( broken ) {
          break;
      }
    }

    if( all_accepted ) {
        return true;
        break;
    }
  }

  return false;
}

void SymbolicState::applyTransfers(list<Transfer> &transfers)
{
  for(list<Transfer>::iterator it = transfers.begin(); 
      it != transfers.end();
      ++it)
  {
    Transfer      t = *it;
    SinkType      sink = t.first;
    ExpressionPtr transferE = t.second;

    //look up the sink type in the map
    //if it doesn't exist, just put the expression in the map
    //if it does exist, update the expression with this new expression
    StateMap::iterator k = this->state.find(sink);

    if(k == this->state.end())
    {
      //does not exist, store expression in map
      this->state[sink] = transferE;
    }
    else
    {
      //exists, update expression and re-store
      ExpressionPtr newE = this->updateExpr(sink, (*k).second, transferE);
      this->state[sink] = newE;
    }
  }

  return;
}

VisitorResult StatefulRuleMatchingVisitor::keepBlock(BlockPtr B)
{
  //don't put anything here, the stateful visitors don't use this interface
  return Discard;
}

CodeExplorationResult StatefulRuleMatchingVisitor::exploreBlock(BlockPtr  B,
                                                    CodeExplorationStatePtr P)
{
  CodeExplorationResult r;
  //we get the set of transfers out of this block
  list<Transfer>  transfers = B->getTransfers();

  /*cout << endl << "exploreBlock: Block transfers " << endl << endl;
  cout << B->printBlock() << endl;
  for(list<Transfer>::iterator ti = transfers.begin(), e = transfers.end();
      ti != e;
      ++ti)
  {
    cout << B->printTransfer(*ti) << endl;
  }
  cout << endl;*/

  //we should be able to cast the CodeExplorationState into a SymbolicState
  SymbolicStatePtr  Sold = dynamic_pointer_cast<SymbolicState>(P);

  assert(Sold != NULL);

  //then, we copy the incoming state into a new state via clone()
  CodeExplorationStatePtr Ctmp(Sold->clone());

  //make make a visible copy of the new state for our purposes...
  SymbolicStatePtr  S = dynamic_pointer_cast<SymbolicState>(Ctmp);
  
  assert(S != NULL);

  //increment the jump count in our state...
  S->incJumps();

  //and give our result a copy of the state...
  r.newState = Ctmp;

  //apply the transfers to the state 
  S->applyTransfers(transfers);

  //check and see if the new state matches our rules so far
  if(S->stateMatches(this->rules))
  {
    //we found a winner! note up to our caller to accept this block chain
    //note that by saying Accept here, we terminate the search
    r.res = Accept;
  }
  else
  {
    //we maybe should continue to explore. Ask the state how many jumps
    //we've made so far, and if that is outside of our threshold, then 
    //we discard. otherwise, we keep going. 
    //unless, there is nowhere to keep going. so first, we should ask
    //if there are any targets we can discover from this block...
    if(S->getJumps() < this->maxJumps)
    {
      set<boost::uint64_t>  nextAddrs = this->getNextAddrs(B, S);

      if(nextAddrs.size() > 0)
      {
        //give these to the searcher
        r.addrs = nextAddrs;
        //tell the searcher to keep going
        r.res = Keep;
      }
      else
      {
        //there is nothing that we can statically identify as a target that 
        //follows this block. for example, if the block ends in a return, and,
        //we are returning at a point where there is no rule matching
        r.res = Discard;
      }
    }
    else
    {
      //we have gone on for too long
      r.stop = true;
    }
  }
  
  return r;
}

CodeExplorationStatePtr StatefulRuleMatchingVisitor::initialState(void)
{
  //this should START as empty, since we initially don't know anything 
  //about the state of the world and there is nothing to initially 
  //impact our decisions
  SymbolicState *newS = new SymbolicState();

  return CodeExplorationStatePtr(newS);
}

//print progress
static
void print_progress(unsigned int Total, unsigned int Done, ostream &o) {
    const int tdot = 40;

    double pctDone = ((double)Done)/((double)Total);

    unsigned int print = floor(pctDone * tdot);

    unsigned int i = 0;
    o << setprecision(5) << fixed << pctDone*100 << "[";
    for( ; i < print; i++ ) 
        o << "=";
    for( ; i < tdot; i++ ) 
        o << " ";

    o << "]\r";
    o.flush();

    return;
}

FileFormat fmtFromVM(variables_map &vm) {
    FileFormat  fmt = Invalid;

    if( vm.count("format") ) {
        string  format = vm["format"].as<string>();

        if( format == "PE" )
            fmt = PEFmt;
        else if( format == "RAW" )
            fmt = RawFmt;
    }

    return fmt;
}

TargetArch archFromVM(program_options::variables_map &vm) {
    TargetArch  tarch;

    tarch.ta = INVALID;

    if( vm.count("arch") ) {
        string archStr = vm["arch"].as<string>();

        if( archStr == "X86" ) {
            tarch.ta = X86;
        } else if( archStr == "AMD64" ) {
            tarch.ta = AMD64;
        } else if( archStr == "ARM" ) {
            tarch.ta = ARM;
            tarch.tm = WIDEARM;
        } else if( archStr == "ARM-THUMB") {
            tarch.ta = ARM;
            tarch.tm = THUMB;
        } else if( archStr == "AMD64" ) {
            tarch.ta = AMD64;
        }
    }

    return tarch;
}

static const llvm::Target *findDisTarget(std::string arch) {
    const llvm::Target  *tgt = NULL;

    for(llvm::TargetRegistry::iterator  it = llvm::TargetRegistry::begin(),
        ie = llvm::TargetRegistry::end();
        it != ie;
        ++it)
    {   
        if( arch == it->getName() ) {
            tgt = &*it;
            break;
        }
    }

    return tgt;
}

std::string disAtVAInBuff(   uint8_t     *buff,
                        uint64_t    disVA,
                        uint32_t    disLen,
                        TargetArch  tarch)
{
    std::string r = "";

    assert(tarch.ta == X86 || tarch.ta == AMD64 || tarch.ta == ARM);

    const llvm::Target          *t = NULL;
    const llvm::MCSubtargetInfo *STI = NULL;
    const llvm::MCAsmInfo       *AsmInfo = NULL;
#if(LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >=2)
    llvm::MCRegisterInfo        *MRI = NULL;
    const llvm::MCInstrInfo     *MCII = NULL;
#endif

    switch(tarch.ta) {
        case X86:
            t = findDisTarget("x86");
            assert( t != NULL );
            STI = t->createMCSubtargetInfo("i386-unknown-linux-gnu", "", "");
            AsmInfo = t->createMCAsmInfo("i386-unknown-linux-gnu");
#if(LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 2)
            MRI = t->createMCRegInfo("i386-unknown-linux-gnu");
#endif
            break;

        case AMD64:
            t = findDisTarget("x86-64");
            assert( t != NULL );
            STI = t->createMCSubtargetInfo("amd64-unknown-linux-gnu", "", "");
            AsmInfo = t->createMCAsmInfo("amd64-unknown-linux-gnu");
#if(LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 2)
            MRI = t->createMCRegInfo("amd64-unknown-linux-gnu");
#endif
            break;

        case ARM:
            switch(tarch.tm) {
                case THUMB:
                    t = findDisTarget("thumb");
                    assert( t != NULL );
                    STI =
                       t->createMCSubtargetInfo("i386-unknown-linux-gnu","","");
                    AsmInfo = t->createMCAsmInfo("i386-unknown-linux-gnu");
#if(LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 2)
                    MRI = t->createMCRegInfo("i386-unknown-linux-gnu");
#endif
                    break;

                case WIDEARM:
                    t = findDisTarget("arm");
                    assert( t != NULL );
                    STI =
                       t->createMCSubtargetInfo("i386-unknown-linux-gnu","","");
                    AsmInfo = t->createMCAsmInfo("i386-unknown-linux-gnu");
#if(LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 2)
                    MRI = t->createMCRegInfo("i386-unknown-linux-gnu");
#endif
                    break;
                case S_INVALID:
                    assert(!"Invalid subarch!");
                    break;
            }
            break;

        default:
            assert(!"NIY");
    }

    assert(t);
    assert(STI);
    assert(AsmInfo);

    //make a printer object
    int APV = AsmInfo->getAssemblerDialect();
#ifndef LLVM_VERSION_MAJOR
    llvm::MCInstPrinter
        *IP = t->createMCInstPrinter(APV, *AsmInfo, *STI);
#endif
#if(LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 2)
    MCII = t->createMCInstrInfo();
    llvm::MCInstPrinter
        *IP = t->createMCInstPrinter(1, *AsmInfo, *MCII, *MRI, *STI);
#endif

    assert( IP );
    //make a disassembler
    const llvm::MCDisassembler *DisAsm = t->createMCDisassembler(*STI);
    assert( DisAsm );

    //describe the buffer
    BufferMemoryObject  bmo(buff, disLen);

    //do the decode
    uint64_t    Size;
    uint64_t    Index;

    for( Index = 0; Index < disLen; Index += Size ) {
        llvm::MCInst  inst;
        llvm::MCDisassembler::DecodeStatus  s;
        bool                                br=false;
        std::string                              outS;
        llvm::raw_string_ostream            osOut(outS);

        s = DisAsm->getInstruction( inst,
                                    Size,
                                    bmo,
                                    Index,
                                    llvm::nulls(),
                                    llvm::nulls());

        switch(s) {
            case llvm::MCDisassembler::Success:
                //print the bytes 
                for(uint64_t k = 0; k < Size; k++)
                {   
                    uint32_t    bfr;
                    uint8_t     bt;
                    bmo.readByte(Index+k, &bt);
                    bfr = bt;
                    r = r + to_string<uint32_t>(bfr, std::hex) + " ";
                }
                //print the instruction
                IP->printInst(&inst, osOut, "");
                r = r + osOut.str() + "\n";
                break;
            default:
                std::cout << "decode failed" << std::endl;
                br = true;
        }

        if( br ) {
            break;
        }
    }

    return r;
}


void printScoring(  vector<pair<string, RopLibSearcherPtr> >    p, 
                    ostream                                     &o,
                    uint64_t                                    lowAddr,
                    uint64_t                                    highAddr,
                    secVT                                       exec,
                    bool                                        printDecode) 
{

    for(vector<pair<string, RopLibSearcherPtr> >::iterator it = p.begin();
        it != p.end();
        ++it)
    {
        string                  ruleName = (*it).first;
        RopLibSearcherPtr       rls = (*it).second;
        list<list<BlockPtr> >   found = rls->getBlocksFound();

        cout << "ruleName: " << ruleName << " found candidates: ";
        cout << found.size() << endl;

        /* print out a little histogram of the rules we found 
         * start by making a string that is 70 chars wide. we take each
         * addr 
         */
        string  outStr(78, ' ');
        outStr[0] = '[';
        outStr[77] = ']';

        uint64_t    len = highAddr - lowAddr;
        uint64_t    bucketSize = (len/75)+1;

        /* for a given address, the procedure is: 
         *  t = addr - lowAddr
         *  j = t/bucketSize
         *  outStr[j] = '|'
         */
        for(list<list<BlockPtr> >::iterator n = found.begin();
            n != found.end();
            ++n)
        {
            list<BlockPtr>  gadget = *n;
            for(list<BlockPtr>::iterator  giter = gadget.begin();
                giter != gadget.end();
                ++giter)
            {
              BlockPtr  B = *giter;
              uint64_t  blockBase = B->getBlockBase();
              uint64_t  blockLen = B->getBlockLen();

              //disassemble buffer
              for(secVT::iterator sit = exec.begin(); sit != exec.end(); ++sit)
              {
                secAndArchT saa = *sit;
                TargetArch  arch = saa.first;
                secPT       secAndLen = saa.second;
                lenAddrT    len = secAndLen.second;
                uint8_t     *buf = secAndLen.first;
                uint64_t    baseAddr = len.second;
                uint32_t    bufLen = len.first;

                if( ((blockBase >= baseAddr) &&
                    (blockBase < (baseAddr+bufLen)) ) &&
                    ((blockBase+blockLen <= (baseAddr+bufLen)) ) )
                {
                  uint64_t  delta = blockBase-baseAddr;
                  uint8_t   *disBuf = buf+delta;

                  string  s = disAtVAInBuff(disBuf, blockBase, blockLen, arch);
                  if(printDecode)
                    cout << s << endl << endl;
                }
              }
            }

            list<BlockPtr>::iterator    git = gadget.begin();

            if( git != gadget.end() ) {
                BlockPtr    b = *git;
                uint64_t    addr = b->getBlockBase();
                uint64_t    t = addr - lowAddr;
                uint64_t    j = (t/bucketSize)+1;
                
                assert(j < outStr.size());
                outStr[j] = '|';
            }
        }

        cout << outStr << endl;
    }

    return;
}

int main(int argc, char *argv[])
{
    /* command line arguments */
    options_description d("options");
    variables_map       vm;

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllDisassemblers();

    d.add_options()
        ("version,v", "show version")
        ("help,h", "show this help")
        ("format,f", value<string>(), "file format <PE|MACH|UCACHE|RAW>")
        ("arch,a", value<string>(), "arch <ARM|X86|AMD64>")
        ("rules,r", value< vector<string> >()->multitoken(), "rules")
      ("conditions,c", value< vector<string> >()->multitoken(), "conditions")
        ("input,i", value<string>(), "file to score")
        ("do-multi-gadget,m", "Do multi-gadget search")
        ("disassembly,d", "show candidate disassembly");

    store(parse_command_line(argc, argv, d), vm);

    if(vm.count("version") || vm.count("help"))
    {
        cout << d << endl;
        return 0;
    }

    bool  doDecode = false;

    if(vm.count("disassembly"))
      doDecode = true;

    if(vm.count("rules") == 0 && vm.count("conditions") ==0)
    {
        cout << "Must supply some rules or conditions" << endl;
        return 0;
    }

    if(vm.count("arch") == 0)
    {
        cout << "Must supply architecture" << endl;
        return 0;
    }

    if(vm.count("input") ==0) {
        cout << "Must supply input file" << endl;
        return 0;
    }

    vector<string>  ruleFiles;
    vector<string>  condFiles;
    string          inputFile = vm["input"].as<string>();

    if(vm.count("rules"))
        ruleFiles = vm["rules"].as<vector<string> >();

    if(vm.count("conditions"))
        condFiles = vm["conditions"].as<vector<string> >();

    /* initialize stuff */

    /* initialize our translator context */
    TargetArch  ta = archFromVM(vm);
    FileFormat  fmt = fmtFromVM(vm);
    void        *decodeCtx = initDecodeLib2(ta, true, false);
    assert(decodeCtx != 0);

    /* get the executable sections for the file we are scoring */
    ExecCodeProviderPtr code(new ExecCodeProvider(inputFile, fmt, ta));

    secVT   execCode = code->sections_in_file(inputFile);

    /* get low and high addresses */
    uint64_t    lowAddress = 0;
    uint64_t    highAddress = 0;
    for(secVT::iterator it = execCode.begin(); it != execCode.end(); ++it) {
        uint64_t    len = (((*it).second).second).first;
        uint64_t    base = (((*it).second).second).second;

        if(lowAddress == 0 || base < lowAddress)
            lowAddress = base;
        
        if(highAddress == 0 || (len+base) > highAddress)
            highAddress = (len+base);
    }

    /* get blocks for the section we're scoring, this involves decoding */
    list<BlockPtr>  blocks = 
        RopLibSearcher::decodeBlocks(decodeCtx, execCode, cout);

    /* make a searcher for every condition or input we have */
    vector<pair<string, RopLibSearcherPtr> >    searchers;

    /* read in our rules files */
    for(vector<string>::iterator it = ruleFiles.begin();
        it != ruleFiles.end();
        ++it)
    {
        string                  input = *it; 
        vector<StatementPtr>    r;

        try {
            r = getExprsForFile(input);
        } catch( SemanticFail &s) {
            cout << "semantic fail in file " << input << endl;
            return 0;
        }

        pair<string, vector<StatementPtr> >  n(input, r);
        vector<pair<string, vector<StatementPtr> > > t;
        t.push_back(n);

        RopLibVisitorPtr    rv;
        if(vm.count("do-multi-gadget"))
          rv = RopLibVisitorPtr(new StatefulRuleMatchingVisitor(t));
        else
          rv = RopLibVisitorPtr(new RuleMatchingVisitor(t));

        RopLibSearcherPtr   s;
        if(vm.count("do-multi-gadget"))
          s = RopLibSearcherPtr(new StatefulRopLibSearcher( rv, 
                                                            blocks, 
                                                            decodeCtx, 
                                                            ta));
        else
          s = RopLibSearcherPtr(new RopLibSearcher(rv, blocks, decodeCtx, ta));

        pair<string, RopLibSearcherPtr> k(input, s);
        searchers.push_back(k);
    }

    /* read in our condition files */
    if(!vm.count("do-multi-gadget"))
    {
      for(vector<string>::iterator it = condFiles.begin();
          it != condFiles.end();
          ++it)
      {
          string  input = *it;

          Condition   *cs = getConditionsFromFile(input, ta);
          assert(cs != NULL);

          if(!cs->loaded()) {
              cout << "Could not load conditions from file" << endl;
              return 0;
          }

          RopLibVisitorPtr    rv(new MatchesVEE(cs, 4));
          RopLibSearcherPtr   s(
              new RopLibSearcher(rv, blocks, decodeCtx, ta));
          pair<string, RopLibSearcherPtr> k(input, s);
          searchers.push_back(k);
      }
    }
    
    /* get data */

    /* see which of our rules match on the executable sections */
    uint64_t    curRule = 1;
    uint64_t    maxRules = searchers.size();
    for(vector<pair<string, RopLibSearcherPtr> >::iterator it = searchers.begin();
        it != searchers.end();
        ++it)
    {
        string              rule = (*it).first;
        RopLibSearcherPtr   rls = (*it).second;
       
        cout << "checking rule " << rule << endl;
        print_progress(maxRules, curRule, cout);
        curRule++;
        cout << endl;

        uint64_t    cur = 1;
        uint64_t    end = rls->getNumBlocks();
        while(rls->canSearch()) {
            print_progress(end, cur, cout);
            cur++;
            rls->evalOneBlock();
        }

        cout << endl;
    }

    cout << endl;

    /* print out the scoring information */
    printScoring(searchers, cout, lowAddress, highAddress, execCode, doDecode);

    return 0;
}
