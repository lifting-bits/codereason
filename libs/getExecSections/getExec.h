#ifndef _GET_EXEC_H
#define _GET_EXEC_H

#include <boost/cstdint.hpp>
#include <BasicIR.h>

enum FileFormat {
    Invalid,
    PEFmt,
    MachOFmt,
    RawFmt,
    DyldCacheFmt
};

///////////////////////////////////////////////////////////////////////////////
// the old-style interface to getting executable sections
// this assumes that there is one file in the path
// this breaks down when we could have multiple files, i.e. in the iOS 
// kernel / user cache
///////////////////////////////////////////////////////////////////////////////

//(baseAddress, length)
typedef std::pair<boost::uint32_t, boost::uint64_t> lenAddrT;
//(pointer to data, (basAddress, length))
typedef std::pair<boost::uint8_t *, lenAddrT> secPT;
typedef std::pair<TargetArch, secPT> secAndArchT;
//vector of sections
typedef std::vector<secAndArchT> secVT;

secVT getExecSections(std::string path, FileFormat f, TargetArch t);

///////////////////////////////////////////////////////////////////////////////
// the new interface for getting executable sections
// supports both the old style and new style
///////////////////////////////////////////////////////////////////////////////

class ExecCodeProvider {
private:
    bool        err;    
    FileFormat  fmt;
    TargetArch  arch;
    void        *peCtx;
    void        *dyldCtx;
    uint8_t     *buf;
    uint32_t    bufLen;
    std::string fName;
public:
    ExecCodeProvider() : err(true) { return; }
    ExecCodeProvider(std::string, FileFormat, TargetArch);

    bool get_err(void) { return this->err; }

    std::list<std::string> filenames(void);

    secVT sections_in_file(std::string file);
};

typedef boost::shared_ptr<ExecCodeProvider> ExecCodeProviderPtr;

#endif
