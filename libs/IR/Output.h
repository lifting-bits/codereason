#include <list>

/*class Output : public boost::enable_shared_from_this<Output> {
private:
    Block                   *block;
	bool	                readMem;
	bool	                writeMem;
	bool	                ret;
    bool                    condBranch;
    bool                    anyCalls;
    std::list<TransferInt>  transfer;
public:
	Output(Block *b);

	bool readsMem(void) { return this->readMem; }
	bool writesMem(void) { return this->writeMem; }
	bool returns(void) { return this->ret; }
    bool conditionalBranch(void) { return this->condBranch; }
    bool calls(void) { return this->anyCalls; }

    unsigned long getFollowingAddr(void);

    std::list<Transfer> getTransfers(void);

    Transfer    getTransferForReg(Register r);
};

typedef boost::shared_ptr<Output> OutputPtr;*/
