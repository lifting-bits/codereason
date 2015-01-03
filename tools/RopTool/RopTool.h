/* apparently this isn't even used... */

#include <getExec.h>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <VEE.h>
#include <decodeLib.h>
#include <inttypes.h>

namespace udis {
#include <udis86.h>
}

#include <boost/program_options/config.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/regex.hpp>

#include <capstone/capstone.h>

/*class SearchInstance : public boost::enable_shared_from_this<SearchInstance> {
private:
    boost::mutex                progressLock;
    boost::mutex                isDoneLock;
    boost::condition_variable   isDone; 
    //ez::ezRateProgressBar       progress;
    boost::uint64_t             curProgress;
    boost::uint64_t             done;
    boost::mutex                &ctxLock;
    void                        *ctx;
    TargetArch                  tarch;
    int                         maxTransitions;
    void doSearchWithState(  boost::uint8_t *, 
                    boost::uint64_t, 
                    boost::uint64_t, 
                    BlockPtr,
                    std::vector<std::vector<BlockPtr> > &,
                    boost::mutex &,
                    Condition *,
                    VexExecutionStatePtr);


    BlockPtr getNextBlock(  ExpressionPtr,
                            boost::uint8_t *,
                            boost::uint64_t,
                            boost::uint64_t);
public:
    SearchInstance(boost::uint32_t l, void *c, boost::mutex &m, TargetArch t, int mt) : maxTransitions(mt), curProgress(0), ctx(c), ctxLock(m), tarch(t), done(l) { }

    void doSearch(  boost::uint8_t *, 
                    boost::uint64_t, 
                    boost::uint64_t, 
                    BlockPtr,
                    std::vector<std::vector<BlockPtr> > &,
                    boost::mutex &,
                    Condition *);

    void waitForCompletion(void) { 
        boost::unique_lock<boost::mutex> l(this->isDoneLock); 
        this->isDone.wait(l); 
    }
};*/
