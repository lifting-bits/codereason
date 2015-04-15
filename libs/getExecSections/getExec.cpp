#include "getExec.h"
#include "parse.h"

#include <elfio/elfio.hpp>

#include <machine.h>
#include <loader.h>
#include <fat.h>

using namespace boost;


/* inline methods to handle endianness */
static inline unsigned short bswap_16(unsigned short x) {
	return (x>>8) | (x<<8);
}

static inline unsigned int bswap_32(unsigned int x) {
	return (bswap_16(x&0xffff)<<16) | (bswap_16(x>>16));
}

static inline unsigned long long bswap_64(unsigned long long x) {
	return (((unsigned long long)bswap_32(x&0xffffffffull))<<32) | (bswap_32(x>>32));
}

/* a small struct to get data in and out of the PE Section callback */
struct peCbIO
{
    secVT vec;
    TargetArch peArch;
};

/* the callback run on each PE when searching for executable segments */
int peSecCb(void *  N, VA secBase, std::string  &secName,
            image_section_header  s, bounded_buffer *data)
{

    /* grab our in/out data struct */
    peCbIO * inout = (peCbIO*)N;


    /* determine if this section is executable */
    if(s.Characteristics & IMAGE_SCN_MEM_EXECUTE)
    {
        /* printf("Found executable section! 0x%llx\n", secBase); FIXME: take this away */
        lenAddrT  pv = lenAddrT(data->bufLen, secBase);
        secPT p = secPT(data->buf, pv);
        secAndArchT n = secAndArchT(inout->peArch, p);

        /* add the section to our output vector */
        inout->vec.push_back(n);
    }

    return 0;
}


/*
 * getExecELFSections:
 *      given a linux ELF, extract any executable sections. Called when the
 *      constructor's executable is an ELF file.
 *
 */
secVT ExecCodeProvider::getExecELFSections(void)
{

    ELFIO::elfio *reader = (ELFIO::elfio*)this->elfCtx;
    secVT found;


    ELFIO::Elf_Half seg_num = reader->segments.size();
    for(int i = 0; i < seg_num; i++)
    {
        const ELFIO::segment* pseg = reader->segments[i];

        /* select only executable segments
         * FIXME: maybe change this '1' to a macro def */
        if(pseg->get_flags() & 1)
        {
            lenAddrT  pv = lenAddrT((uint32_t)pseg->get_file_size(),
                                    pseg->get_virtual_address());
            secPT p = secPT((unsigned char *)pseg->get_data(), pv);
            secAndArchT n = secAndArchT(this->arch, p);
            found.push_back(n);
        }
    }

    /* return the executable sections found */
    return found;
}

/*
 * getExectPESections:
 *      given a windows PE, extract any executable sections. Called when the
 *      constructor's executable is a PE file.
 */
secVT ExecCodeProvider::getExecPESections()
{

    peCbIO inout;
    parsed_pe * p = (parsed_pe*)this->peCtx;
    inout.peArch = this->arch;


    /* iterate over all PE sections and find executable segments */
    IterSec(p, peSecCb, &inout);

    /* return the executable sections found */
    return inout.vec;
}

static uint8_t *memMapFile(FILE *f, size_t len, size_t off)
{

	uint8_t *buf = (uint8_t *)malloc(len);
	assert(buf != NULL);

	fseek(f, 0, SEEK_SET);
	fseek(f, off, SEEK_SET);

	size_t read = fread(buf, sizeof(uint8_t), len, f);

	assert(read == len);

	return buf;

}

/*
 * selectArhForFAT:
 *      Select an architecture for FAT binaries, if t is given, try to
 *      find it inside the executable. If t is not here, returns false.
 *
 *  Arguments:
 *      TargetArch t: the architechture to find within our FAT header.
 *
 *  Returns:
 *      true: if the architecture is here
 *      false: otherwise
 */
bool ExecCodeProvider::selectArchForFAT(TargetArch t)
{
    bool found = false;
    fat_header *n = (fat_header *)this->buf;
    fat_arch *fats = (fat_arch*)(((ptrdiff_t)this->buf)+sizeof(fat_header));

    /* make sure this is really a FAT */
    if( bswap_32(n->magic) != FAT_MAGIC )
    {
        std::cerr << "[Error] This is not a FAT MachO" << std::endl;
        this->err = true;
        return false;
    }

    /* find target arch; do the seeking now */
    uint32_t numArches = bswap_32(n->nfat_arch);


    for(int i = 0; i < numArches; i++) {

        uint32_t fatsOff = bswap_32(fats[i].offset);
        uint8_t	*newBuff = (uint8_t*) ( ((ptrdiff_t)this->buf) + fatsOff);
        mach_header *mhd = (mach_header *)newBuff;

        /* check if this a 32/64 bit little endian MachO */
        if(mhd->magic == MH_MAGIC || mhd->magic == MH_MAGIC_64) {

            /* Populate the mach-o header and verify is the one we want for our target
             * architechture. If he's the one, there is no need to continue.
             */
            seekArch = convertMachArch(mhd->cputype);

            if (t.ta != seekArch.ta) {
                continue;
            } else {
                this->machoCtx = mhd;
                this->arch = t;
                break;
            }
        }
    }

    return found;
}

/* TODO: clean this up, trim it down, remove unecessary args, rename
 * getExecMachSectionsFromBuff:
 *      locates the executable sections of a mach file from a binary buffer.
 *      returns a vector containing the found sections.
 *
 * Arguments:
 *      uint8 *buf: the binary buffer to seek in
 *      uint32_t len: the length of the binary buffer.
 *      TargetArch t: not used at the moment
 *
 * returns:
 *      secVT: a vector containing the executable sections for this buffer
 */
secVT ExecCodeProvider::getExecMachSectionsFromBuff(uint8_t *buf, uint32_t len, TargetArch t)
{
	secVT found;

    /* find all executable sections in a 32bit MachO */
	mach_header	*m32 = (mach_header *)buf;
	if( this->arch.ta == X86 || this->arch.ta == ARM )
    {
        /* walk over all the loader commands */
        uint8_t	*cur = (uint8_t *) (((ptrdiff_t)buf) + sizeof(mach_header));

        uint32_t	numCmds = m32->ncmds;
        for( int i = 0; i < numCmds; i++ ) {

            load_command *l = (load_command *)cur;
            uint32_t cmd_sz = l->cmdsize;

            switch(l->cmd) {

                case LC_SEGMENT: {
                    segment_command	*sc =
                        (segment_command *) (((ptrdiff_t)cur) );

                    uint32_t numSects = sc->nsects;

                    section	*sec =
                        (section *)(((ptrdiff_t)sc)+sizeof(segment_command));

                    /* iterate over the number of sections */
                    for( int k = 0; k < numSects; k++ ) {

                        if(sc->initprot & VM_PROT_EXECUTE) {

                            /* save this one */
                            uint32_t addr = sec->addr;
                            uint32_t bytesOff = sec->offset;
                            uint32_t size = sec->size;

                            /* copy from sourceBytes */
                            uint8_t	 *sourceBytes =
                                (uint8_t *) ( ((ptrdiff_t)buf) + bytesOff);
                            uint8_t	*dstBytes = (uint8_t *) malloc(size);
                            assert(dstBytes != NULL);
                            memcpy(dstBytes, sourceBytes, size);

                            /* build up the entries for secVT and append */
                            lenAddrT pv = lenAddrT(size, addr);
                            secPT p = secPT(dstBytes, pv);
                            secAndArchT n = secAndArchT(t, p);
                            found.push_back(n);
                        }

                        sec++;
                    }

                }
                    break;
            }

            /* advance */
            cur = (uint8_t *) ( ((ptrdiff_t)cur) + cmd_sz);
        }

	} else {

        mach_header_64	*m64;
        m64 = (( mach_header_64 *)buf);

	    if(this->arch.ta == AMD64) {

            /* walk over all the loader commands */
            struct load_command * cur =
            (struct load_command *) (((ptrdiff_t)buf) +
            sizeof(mach_header_64));

            for( int i = 0; i < m64->ncmds; i++ ) {

                switch(cur->cmd) {

                    case LC_SEGMENT_64: {

                        segment_command_64 *sc =
                                    (segment_command_64 *) (((ptrdiff_t)cur) );

                        uint32_t numSects = sc->nsects;

                        section_64	*sec =
                            (section_64 *)(((ptrdiff_t)sc) +
                                    sizeof(segment_command_64));

                        /* iterate over the number of sections */
                        for( int k = 0; k < numSects; k++ ) {

                            if( sc->initprot & VM_PROT_EXECUTE ) {

                                /* save this one */
                                uint32_t addr = sec->addr;
                                uint32_t bytesOff = sec->offset;
                                uint32_t size = sec->size;

                                /* copy from sourceBytes */
                                uint8_t	*sourceBytes =
                                    (uint8_t *) ( ((ptrdiff_t)buf) + bytesOff);
                                uint8_t *dstBytes = (uint8_t *) malloc(size);
                                assert(dstBytes != NULL);
                                memcpy(dstBytes, sourceBytes, size);

                                /* build up the entries for secVT and append */
                                lenAddrT pv = lenAddrT(size, addr);
                                secPT p = secPT(dstBytes, pv);
                                secAndArchT n = secAndArchT(t, p);
                                found.push_back(n);
                            }

                            sec++;
                        }

                    }
                        break;
                }

                cur = (struct load_command *)(((ptrdiff_t)cur) + cur->cmdsize);
            }

        }

	}

	return found;
}


/*
 * getExecMachSections:
 *      Parses a possible mach header and searches for the executable sections.
 *      this file might or might not be a FAT object, so parsin is done accordingly.
 *
 */
secVT ExecCodeProvider::getExecMachSections()
{

    secVT   found;

#ifndef WIN32
    /* what kind of magic is at the beginning of the file buffer? */
    fat_header	*n = (fat_header *)this->buf;
    if(bswap_32(n->magic) == FAT_MAGIC) {

        /* this is a FAT file */
        fat_arch *fats =
            (fat_arch*)( ((ptrdiff_t)this->buf)+sizeof(fat_header) );

        /* call getExecMachSectionsFromBuff on each
         * FIXME, do we really need to call this on each of these?
         */
        uint32_t	numArches = bswap_32(n->nfat_arch);

        for(int i = 0; i < numArches; i++) {

            uint32_t fatsOff = bswap_32(fats[i].offset);
            uint8_t	*newBuff = (uint8_t*) ( ((ptrdiff_t)this->buf) + fatsOff);

            secVT tmp = this->getExecMachSectionsFromBuff(newBuff,
                    this->bufLen-fatsOff, this->arch);

            found.insert( found.end(), tmp.begin(), tmp.end() );
        }

    } else {
        /* not a FAT file, parse the MACH-o normally */
        found = this->getExecMachSectionsFromBuff(this->buf, this->bufLen, this->arch);
    }
#endif

    /* FIXME If this is windows, we return an uninitialized variable from the stack? */
    return found;
}

/*
 * getRaw
 *      returns the 'one' virtual segment of our raw blob. The virtual equivalent
 *      of the previous section-parsing methods.
 *
 *  returns:
 *      secVT: a virtual section from this binary blob.
 */
secVT ExecCodeProvider::getRaw()
{
    secVT s;

    /*
     * the base address for raw blobs is set to 0x1000
     * as the VEX engine will complain if it starts at address 0
     */
    lenAddrT pv = lenAddrT(this->bufLen, 0x1000);
    secPT p = secPT(this->buf, pv);
    secAndArchT at = secAndArchT(this->arch, p);
    s.push_back(at);

    return s;
}

/*
 * convertPEArch:
 *      convert the windows PE arch field to our TargetArch type.
 *
 *  Arguments:
 *      uint32_t: machine_type, the desired machine_type
 *
 *  Returns:
 *      TargetArch result: the resulting architechture struct.
 */
TargetArch ExecCodeProvider::convertPEArch(uint32_t machine_type)
{

    TargetArch result;

    switch(machine_type) {

        case IMAGE_FILE_MACHINE_I386: {

            result.ta = X86;
        }
            break;

        case IMAGE_FILE_MACHINE_AMD64: {
            result.ta = AMD64;
        }
            break;

        /* TODO: find some elegant way to switch between arm/thumb
                 rather than just hardcoding thumb :| */
        case IMAGE_FILE_MACHINE_ARM: {
            result.ta = ARM;
            result.tm = THUMB;
        }
            break;

        default: {
            result.ta = INVALID;
        }
    }

    return result;

}

/*
 *  convertELFArch:
 *      convert the linux ELF arch field to our TargetArch type.
 *
 *  Arguments:
 *      uint32_t machine_type: the desired machne type:
 *
 *  Returns:
 *      TargetArch result: the resulting architechture struct.
 */
TargetArch ExecCodeProvider::convertELFArch(uint32_t machine_type)
{

    TargetArch result;

    switch(machine_type) {

        case EM_386: {
            result.ta = X86;
        }
            break;

        case EM_X86_64: {
            result.ta = AMD64;
        }
            break;

        /* TODO: find some elegant way to switch between arm/thumb
                 rather than just hardcoding thumb :| */
        case EM_ARM: {
            result.ta = ARM;
            result.tm = THUMB;
        }
            break;

        default: {
            result.ta = INVALID;
        }
    }

    return result;

}

/*
 *  convertMachArch:
 *      convert the windows PE arch field to our TargetArch type
 *
 *  Arguments:
 *      uint32_t machine_type: the desired machine type.
 *
 *  Returns:
 *      TargetArch result: a structure conforming to the target arch
 */
TargetArch ExecCodeProvider::convertMachArch(uint32_t machine_type)
{

    TargetArch result;

    switch(machine_type) {

        case CPU_TYPE_X86: {

            result.ta = X86;

        }
            break;

        case (CPU_TYPE_X86_64 | CPU_ARCH_ABI64): {

            result.ta = AMD64;

        }
            break;

        /* TODO: find some elegant way to switch between arm/thumb
                 rather than just hardcoding thumb :| */
        case CPU_TYPE_ARM: {

            result.ta = ARM;
            result.tm = THUMB;

        }
            break;

        default: {

            result.ta = INVALID;

        }
    }

    return result;

}

/*
 * class ExecCodeProvider:
 *   our super executable wrapper that abstracts everything away!
 *
 * Constructor Arguments:
 *      std::string p: the filename
 *
 *      targetArch t: The target architechture definition, only required for
 *                    multi-architecture binaries
 *
 *      bool raw: Whether the file to read is a binary blob (raw data)
 */
ExecCodeProvider::ExecCodeProvider(std::string p, TargetArch t, bool raw)
{

    this->arch = t;
    this->err = false;
    this->buf = NULL;
    this->bufLen = 0;
    this->fName = p;
    this->peCtx = NULL;
    this->elfCtx = NULL;
    this->machoCtx = NULL;

    /* open and map the executable */
    FILE *fp = fopen(p.c_str(), "rb");
    if(fp) {

        uint32_t len;
        fseek(fp, 0L, SEEK_END);
        len = ftell(fp);
        fseek(fp, 0L, SEEK_SET);

        uint8_t	*fileBuf = memMapFile(fp, len, 0);
        assert(fileBuf != NULL);

        /* save relevant details of the mapped file */
        this->buf = fileBuf;
        this->bufLen = len;

        fclose(fp);

    } else {

        std::cout << "[Error] Could not open file " << p << std::endl;
        this->err = true;
        return;

    }

    /* Raw blob */
    if(raw) {

        this->fmt = RawFmt;
        this->arch = t;

        /* we're not going to guess the architecture of raw data :| */
        if(this->arch.ta == AUTODETECT) {

            std::cout << "[Error] You must specify an architecture when using a raw blob"
                << std::endl;

            this->err = true;
            return;
        }
    }

    /* Windows Executable */
    else if(memcmp(this->buf, "MZ", 2) == 0) {
        this->fmt = PEFmt;
        this->peCtx = ParsePEFromFile(p.c_str());

        /* determine .exe's architecture */
        parsed_pe * p = (parsed_pe*)this->peCtx;
        this->arch = this->convertPEArch(p->peHeader.nt.FileHeader.Machine);

    }

    /* Linux ELF */
    else if(memcmp(this->buf, "\x7f\x45\x4c\x46", 4) == 0) {

        this->fmt = ELFFmt;
        this->elfCtx = new ELFIO::elfio;

        /* parse ELF header */
        ELFIO::elfio * reader = (ELFIO::elfio*)this->elfCtx;
        if(!reader->load(p.c_str())) {

            std::cout << "[Error] Cannot find or process ELF file "
                      << p
                      << std::endl;
            this->err = true;
            return;

        }

        /* determine ELF's architecture */
        this->arch = this->convertELFArch(reader->get_machine());

    }

    /* x86 or x64 MachO */
    else if((memcmp(this->buf, "\xce\xfa\xed\xfe", 4) == 0) ||
            (memcmp(this->buf, "\xcf\xfa\xed\xfe", 4) == 0)) {

        this->fmt = MachOFmt;

        /* TODO: get Mac samples and test stuff */
        mach_header	*mhd = (mach_header *)this->buf;

        //if( bswap_32(m32->magic) == MH_CIGAM ) FIXME: this should be done, I bet.
        if (mhd->magic == MH_MAGIC || mhd->magic == MH_MAGIC_64) {

            TargetArch t = convertMachArch(mhd->cputype);

            /* found our MachO, save the buffer location & arch */
            if(t.ta != INVALID) {
                this->machoCtx = mhd;
                this->arch = t;
                               //return;
            }
        } else {

            std::cout << "[Error] Unknown file magic" << std::endl;

            this->err = true;
            return;
        }
    }

    /* FAT MachO Executable */
    else if(memcmp(this->buf, "\xca\xfe\xba\xbe", 4) == 0) {

        this->fmt = MachOFmt;

        /* check if this is FAT MachO, and if it actually has more than one arch */
        fat_header	*n = (fat_header *)this->buf;
        uint32_t numArches = bswap_32(n->nfat_arch);
        if(this->arch.ta == AUTODETECT && numArches > 1) {

            std::cout << "[Error] This MachO contains code for multiple architectures"
                      << std::endl;

            std::cout << "[Error] Please specify an architecture to use and try again"
                      << std::endl;

            this->err = true;
            return;
        }

        /* select the the user specified architecure for this FAT MachO */
        if(!selectArchForFAT(t)) {

            std::cout << "[Error] Could not find specified architecture in MachO" << std::endl;
            this->err = true;
            return;

        }
    }

    /* error-handle unrecognized binaries */
    else {

        std::cout << "[Error] Could not identify executable format" << std::endl;
        this->err = true;

    }

    return;
}


std::list<std::string> ExecCodeProvider::filenames(void) {

    std::list<std::string>  found;

    switch(this->fmt) {
        /* TODO: trim everything but the file name from the filepath for these */
        case PEFmt:
        {
            /*
             * PE has one 'file name', and that is the file name of the module
             * that we passed
             */
            found.push_back(this->fName);
        }
            break;

        case ELFFmt:
        {
            found.push_back(this->fName);
        }
            break;

        case MachOFmt:
        {
            found.push_back(this->fName);
        }
            break;

        case RawFmt:
        {
            found.push_back(this->fName);
        }
            break;

        case Invalid:
            break;
    }

    return found;
}

secVT ExecCodeProvider::getExecSections() {
    secVT   secs;

    switch(this->fmt) {
        case PEFmt:
        {
            secs = this->getExecPESections();
        }
            break;

        case ELFFmt:
        {
            secs = this->getExecELFSections();
        }
            break;

        case MachOFmt:
        {
            secs = this->getExecMachSections();
        }
            break;

        case RawFmt:
        {
            secs = this->getRaw();
        }
            break;

        case Invalid:
            break;
    }

    return secs;
}
