#include "getExec.h"
#include "getExec_int.h"
#include "parse.h"

#include <machine.h>
#include <loader.h>
#include <fat.h>

using namespace boost;

static inline unsigned short bswap_16(unsigned short x) {
	return (x>>8) | (x<<8);
}

static inline unsigned int bswap_32(unsigned int x) {
	return (bswap_16(x&0xffff)<<16) | (bswap_16(x>>16));
}

static inline unsigned long long bswap_64(unsigned long long x) {
	return (((unsigned long long)bswap_32(x&0xffffffffull))<<32) | (bswap_32(x>>32));
}

// a small struct to get data in and out of the PE Section callback
struct peCbIO
{
    secVT vec;
    TargetArch peArch;
};

// the callback run on each PE when searching for executable segments
int peSecCb(void *                  N,
            VA                      secBase, 
            std::string             &secName, 
            image_section_header    s,
            bounded_buffer          *data)
{
    // grab our in/out data struct
    peCbIO * inout = (peCbIO*)N;
    
    // determine if this section is executable
    if(s.Characteristics & IMAGE_SCN_MEM_EXECUTE)
    {
        //printf("Found executable section! 0x%llx\n", secBase);
        lenAddrT  pv = lenAddrT(data->bufLen, secBase);
        secPT p = secPT(data->buf, pv);
        secAndArchT n = secAndArchT(inout->peArch, p);
        
        // add the section to our output vector
        inout->vec.push_back(n);
    }

    return 0;
}


// given a windows PE, extract any executable sections
secVT ExecCodeProvider::getExecPESections() {
    peCbIO inout;
    parsed_pe * p = (parsed_pe*)this->peCtx;
    inout.peArch = this->arch;
    
    // iterate over all PE sections and find executable segments
    IterSec(p, peSecCb, &inout);
   
    // return the executable sections found
    return inout.vec;
}

/*
void findExecMach(MachO *m, secVT &vec, TargetArch t) {
    const vector<segment_command *> &v = m->segments();

    vector<segment_command *>::const_iterator it = v.begin();
    FILE *f = fopen(m->filename().c_str(), "rb");
    while( it != v.end() ) {
        segment_command *s = *it;

        if( s->maxprot | VM_PROT_EXECUTE ) {
            uint32_t    off = s->fileoff + m->offset();
            uint32_t    sz = s->filesize;

            fseek(f, 0, SEEK_SET);
            fseek(f, off, SEEK_SET);

            uint8_t *buf = (uint8_t *)malloc(sz);

            assert(buf != NULL);

            size_t rd = fread(buf, sizeof(uint8_t), sz, f);

            assert(rd == sz);

            uint64_t    va = s->vmaddr;
            lenAddrT    pv = lenAddrT(sz, va);
            secPT       p = secPT(buf, pv);
            secAndArchT at = secAndArchT(t,p);
            vec.push_back(at);
        }

        ++it;
    }
    return;
}*/

static
uint8_t *memMapFile(FILE *f, size_t len, size_t off) {
	uint8_t *buf = (uint8_t *)malloc(len);
	assert(buf != NULL);

	fseek(f, 0, SEEK_SET);
	fseek(f, off, SEEK_SET);

	size_t read = fread(buf, sizeof(uint8_t), len, f);

	assert(read == len);

	return buf;
}

// Select an architecture for FAT binaries
bool ExecCodeProvider::selectArchForFAT(TargetArch t)
{
    bool found = false;
    fat_header *n = (fat_header *)this->buf;
    fat_arch *fats = (fat_arch*)(((ptrdiff_t)this->buf)+sizeof(fat_header));

    // make sure this is really a FAT
    if( bswap_32(n->magic) != FAT_MAGIC )
    {
        std::cerr << "[Error] This is not a FAT MachO" << std::endl;
        this->err = true;
        return false;
    }

    // find target arch
    uint32_t numArches = bswap_32(n->nfat_arch);
    for( int i = 0; i < numArches; i++ ) 
    {
        uint32_t fatsOff = bswap_32(fats[i].offset);
        uint8_t	*newBuff = (uint8_t*) ( ((ptrdiff_t)this->buf) + fatsOff);
        mach_header *mhd = (mach_header *)newBuff;
        
        // check if this a 32/64 bit little endian MachO
        if(mhd->magic == MH_MAGIC || mhd->magic == MH_MAGIC_64)
        {
        
            // flag this MachO if it matches our requested arch
            switch(t.ta) 
            {
                case X86:
                    found = (mhd->cputype == CPU_TYPE_X86);
                    break;
                case AMD64:
                    found = (mhd->cputype == (CPU_TYPE_X86_64 | CPU_ARCH_ABI64));
                    break;
                case ARM:
                    found = (mhd->cputype == CPU_TYPE_ARM);
                    break;
                default:
                    found = false;
            }
            
            // found our MachO, save the location & arch
            if(found)
            {
                this->machoCtx = mhd;
                this->arch = t;
                break;
            }

        }
    }

    return found;
}

//TODO: clean this up, trim it down, remove unecessary args, rename
secVT ExecCodeProvider::findInMachFromBuff(uint8_t *buf, uint32_t len, TargetArch t)
{
	secVT found;
   
    // find all executable sections in a 32bit MachO
	mach_header	*m32 = (mach_header *)buf;
	if( this->arch.ta == X86 || this->arch.ta == ARM )
    {
        //walk over all the loader commands
        uint8_t	*cur = (uint8_t *) (((ptrdiff_t)buf) + sizeof(mach_header));
        
        uint32_t	numCmds = m32->ncmds;
        for( int i = 0; i < numCmds; i++ )
        {
            load_command	*l = (load_command *)cur;
            uint32_t		cmd_sz = l->cmdsize;

            switch( l->cmd ) 
            {
                case LC_SEGMENT:
                {
                    segment_command	*sc = 
                        (segment_command *) (((ptrdiff_t)cur) );
                    
                    uint32_t	numSects = sc->nsects;
                    
                    section	*sec = 
                        (section *)(((ptrdiff_t)sc)+sizeof(segment_command)); 
                    //iterate over the number of sections
                    for( int k = 0; k < numSects; k++ )
                    {

                        if( sc->initprot & VM_PROT_EXECUTE )
                        {
                            //save this one
                            uint32_t	addr = sec->addr;
                            uint32_t	bytesOff = sec->offset;
                            uint32_t	size = sec->size;
                            uint8_t		*sourceBytes = 
                                (uint8_t *) ( ((ptrdiff_t)buf) + bytesOff);
                            //copy from sourceBytes 

                            uint8_t		*dstBytes = (uint8_t *) malloc(size);

                            assert(dstBytes != NULL);

                            memcpy(dstBytes, sourceBytes, size);

                            //build up the entries for secVT and append
                            lenAddrT  pv = lenAddrT(size, addr);
                            secPT p = secPT(dstBytes, pv);
                            secAndArchT n = secAndArchT(t, p);
                            found.push_back(n);
                        }

                        sec++;
                    }
                }
                    break;
            }

            //advance 
            cur = (uint8_t *) ( ((ptrdiff_t)cur) + cmd_sz);
        }
	}
    else 
    {
		mach_header_64	*m64 = (mach_header_64 *)buf;
		if( this->arch.ta == AMD64 )
        {
            //walk over all the loader commands
            struct load_command * cur = 
            (struct load_command *) (((ptrdiff_t)buf) + 
            sizeof(mach_header_64));

            for( int i = 0; i < m64->ncmds; i++ )
            {
                switch(cur->cmd) 
                {
                    case LC_SEGMENT_64:
                    {
                        segment_command_64	*sc = (segment_command_64 *) (((ptrdiff_t)cur) );
                        uint32_t	numSects = sc->nsects;
                        section_64	*sec = (section_64 *)(((ptrdiff_t)sc)+sizeof(segment_command_64)); 
                        
                        //iterate over the number of sections
                        for( int k = 0; k < numSects; k++ )
                        {
                            if( sc->initprot & VM_PROT_EXECUTE )
                            {
                                //save this one
                                uint32_t	addr = sec->addr;
                                uint32_t	bytesOff = sec->offset;
                                uint32_t	size = sec->size;
                                uint8_t		*sourceBytes = (uint8_t *) ( ((ptrdiff_t)buf) + bytesOff);
                                
                                //copy from sourceBytes 
                                uint8_t     *dstBytes = (uint8_t *) malloc(size);
                                assert(dstBytes != NULL);
                                memcpy(dstBytes, sourceBytes, size);

                                //build up the entries for secVT and append
                                lenAddrT  pv = lenAddrT(size, addr);
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

// this file might or might not be a FAT object
secVT ExecCodeProvider::findInMach()
{
    secVT   found;

#ifndef WIN32
    // what kind of magic is at the beginning of the file buffer?
    fat_header	*n = (fat_header *)this->buf;
    if( bswap_32(n->magic) == FAT_MAGIC )
    {
        // this is a FAT file
        fat_arch	*fats = 
            (fat_arch*)( ((ptrdiff_t)this->buf)+sizeof(fat_header) );

        // call findInMachFromBuff on each
        uint32_t	numArches = bswap_32(n->nfat_arch);
        for( int i = 0; i < numArches; i++ ) 
        {
            uint32_t	fatsOff = bswap_32(fats[i].offset);
            uint8_t	*newBuff = (uint8_t*) ( ((ptrdiff_t)this->buf) + fatsOff);

            secVT tmp = this->findInMachFromBuff(newBuff, this->bufLen-fatsOff, this->arch);

            found.insert( found.end(), tmp.begin(), tmp.end() );
        }

    } 
    else
    {
        // not a FAT file, parse like normal 
        found = this->findInMachFromBuff(this->buf, this->bufLen, this->arch);
    }
#endif

    return found;
}

secVT ExecCodeProvider::getRaw()
{
    secVT s;
    
    // the base address for raw blobs is set to 0x1000 
    // as the VEX engine will complain if it starts at address 0
    lenAddrT    pv = lenAddrT(this->bufLen, 0x1000);
    secPT       p = secPT(this->buf, pv);
    secAndArchT at = secAndArchT(this->arch,p);
    s.push_back(at);
    return s;
}

// convert the windows PE arch field to our TargetArch type
TargetArch ExecCodeProvider::convertPEArch(uint32_t machine_type)
{
    TargetArch result;

    switch(machine_type)
    {
        case IMAGE_FILE_MACHINE_ARM:
            result.ta = ARM;
            break;
        case IMAGE_FILE_MACHINE_I386:
            result.ta = X86;
            break;
        case IMAGE_FILE_MACHINE_AMD64:
            result.ta = AMD64;
            break;
        default:
            result.ta = INVALID;
    }

    return result;
}

// our super executable wrapper that abstracts everything away!
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
    this->dyldCtx = NULL;

    // open and map the executable
    FILE *fp = fopen(p.c_str(), "rb");
    if( fp )
    {
        uint32_t len;
        fseek(fp, 0L, SEEK_END);
        len = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        
        uint8_t	*fileBuf = memMapFile(fp, len, 0);
        assert(fileBuf != NULL);
       
        // save relevant details of the mapped file
        this->buf = fileBuf;
        this->bufLen = len;
        
        fclose(fp);
    }
    else
    {
        std::cout << "[Error] Could not open file " << p << std::endl;
        this->err = true;
        return;
    }

    std::cout << "Executable Type: ";
    
    // Raw blob
    if(raw)
    {
        std::cout << "Raw Blob" << std::endl;
        this->fmt = RawFmt;
        this->arch = t;

        // we're not going to guess the architecture of raw data :|
        if(this->arch.ta == AUTODETECT)
        {
            std::cout << "[Error] You must specify an architecture when using a raw blob" << std::endl; 
            this->err = true;
            return;
        }
    }
    
    // Windows Executable
    else if(memcmp(this->buf, "MZ", 2) == 0)
    {
        std::cout << "Windows Executable" << std::endl;
        this->fmt = PEFmt;
        this->peCtx = ParsePEFromFile(p.c_str());

        // determine executable type
        parsed_pe * p = (parsed_pe*)this->peCtx;
        this->arch = this->convertPEArch(p->peHeader.nt.FileHeader.Machine);
        
        // do other windows things here

    }
    
    // ELF's
    else if(memcmp(this->buf, "\x7f\x45\x4c\x46", 4) == 0)
    {
        std::cout << "Linux ELF" << std::endl;
        this->fmt = ELFFmt;
    }

    // x86 & x64 MachO's
    else if((memcmp(this->buf, "\xce\xfa\xed\xfe", 4) == 0) || 
            (memcmp(this->buf, "\xcf\xfa\xed\xfe", 4) == 0))
    {
        std::cout << "MachO Executable" << std::endl;
        this->fmt = MachOFmt;
       
        mach_header	*mhd = (mach_header *)this->buf;
        //if( bswap_32(m32->magic) == MH_CIGAM )
        
    }
    
    // FAT Executables
    else if(memcmp(this->buf, "\xca\xfe\xba\xbe", 4) == 0)
    {

        std::cout << "Universal (FAT) MachO Executable" << std::endl;
        this->fmt = MachOFmt;

        // check if this is FAT MachO, and if it actually has more than one arch
        fat_header	*n = (fat_header *)this->buf;
        uint32_t numArches = bswap_32(n->nfat_arch);
        if(this->arch.ta == AUTODETECT && numArches > 1)
        {
            std::cout << "[Error] This MachO contains code for multiple architectures" << std::endl;
            std::cout << "[Error] Please specify an architecture to use and try again" << std::endl;
            this->err = true;
            return;
        }

        // select the the user specified architecure for this FAT MachO        
        if(!selectArchForFAT(t))
        {
            std::cout << "[Error] Could not find specified architecture in MachO" << std::endl;
            this->err = true;
            return;
        }
            
/*
#ifndef WIN32
            dyld_decache::ProgramContext *dy = 
                new dyld_decache::ProgramContext(p);
            if( dy && dy->open() ) {
                //read all the files into memory
                dy->read_all_images();
                this->dyldCtx = dy;
            } else {
                this->err = true;
            }
#endif
*/

    }

    // handle unrecognized binaries
    else
    {
        std::cout << "Unrecognized Executable" << std::endl;
        std::cout << "[Error] Could not identify executable format" << std::endl;
        this->err = true;
        return;
    }

    return;
}

std::list<std::string> ExecCodeProvider::filenames(void) {
    std::list<std::string>  found;
    
    switch(this->fmt) {
        case DyldCacheFmt:
        {
#ifndef WIN32
            dyld_decache::ProgramContext *dy = 
                (dyld_decache::ProgramContext *) this->dyldCtx;
            found = dy->list_of_images();
#endif
        }
            break;

        case PEFmt:
        {
            //PE has one 'file name', and that is the file name of the module
            //that we passed 
            //TODO: get real PE filename
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
        case DyldCacheFmt:
        {
            dyld_decache::ProgramContext *dy = 
                (dyld_decache::ProgramContext *) this->dyldCtx;
            std::list<std::string>  fileNames = dy->list_of_images();
            for(std::list<std::string>::iterator it = fileNames.begin();
                it != fileNames.end();
                ++it)
            {
                std::string n = *it;
                std::cout << "looking at filenames: " << n << std::endl;
                //TODO: Fix dyld handling/parsing
                if( this->fName == n ) {
                    
                    //we found the file, now write it out
                    dy->save_image_with_path(n, "foo.bin");
                    //secs = findInMach("foo.bin", this->arch);
                    
                    //erase the temporary file
                    boost::filesystem::remove("foo.bin");
                    break;
                }
            }
        }
            break;

        case PEFmt:
        {
            secs = this->getExecPESections();
        }
            break;

        case MachOFmt:
        {
            secs = this->findInMach();
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
