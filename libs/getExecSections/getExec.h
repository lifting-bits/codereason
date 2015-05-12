#ifndef _GET_EXEC_H
#define _GET_EXEC_H

#include <boost/cstdint.hpp>
#include <BasicIR.h>

/* Machine type definitions (windows) */
#define IMAGE_FILE_MACHINE_I386     0x014c
#define IMAGE_FILE_MACHINE_ARM      0x01c0
#define IMAGE_FILE_MACHINE_THUMB    0x01c2
#define IMAGE_FILE_MACHINE_AMD64    0x8664


/* File format definitions (i.e., Executable types) */
enum FileFormat {
    Invalid,
    PEFmt,
    ELFFmt,
    MachOFmt,
    RawFmt
};

/*
 * Type Definitions
 */

/* (baseAddress, length) tuple */
typedef std::pair<boost::uint32_t, boost::uint64_t> lenAddrT;

/* (pointer to data, (baseAddress, length)) */
typedef std::pair<boost::uint8_t *, lenAddrT> secPT;
typedef std::pair<TargetArch, secPT> secAndArchT;

/* vector of sections */
typedef std::vector<secAndArchT> secVT;


/*
 * class ExecCodeProvider:
 *      the new interface for getting executable sections from PE, ELF, Mach-o
 *      and RAW file formats
 */
class ExecCodeProvider {
private:
    bool        err;
    FileFormat  fmt;
    TargetArch  arch;
    void        *peCtx;
    void        *elfCtx;
    void        *machoCtx;
    uint8_t     *buf;
    uint32_t    bufLen;
    std::string fName;

    secVT getExecPESections();
    secVT getExecELFSections();
    secVT getExecMachSections();
    secVT getExecMachSectionsFromBuff(uint8_t *, uint32_t, TargetArch);
    secVT getRaw();

public:
    ExecCodeProvider() : err(true) { return; }
    ExecCodeProvider(std::string, TargetArch, bool);

    TargetArch getArch(void) { return this->arch; }
    bool getError(void) { return this->err; }
    bool selectArchForFAT(TargetArch t);
    std::list<std::string> filenames(void);
    secVT getExecSections(void);

    /* handy executable arch to TargetArch converters */
    TargetArch convertPEArch(uint32_t machine_type);
    TargetArch convertELFArch(uint32_t machine_type);
    TargetArch convertMachArch(uint32_t machine_type);
};

typedef boost::shared_ptr<ExecCodeProvider> ExecCodeProviderPtr;

#endif
