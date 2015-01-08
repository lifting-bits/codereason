#include "getExec.h"
#include "getExec_int.h"

using namespace boost;
#ifndef WIN32
#include <machine.h>
#include <loader.h>
#include <fat.h>
#endif

static inline unsigned short bswap_16(unsigned short x) {
	return (x>>8) | (x<<8);
}

static inline unsigned int bswap_32(unsigned int x) {
	return (bswap_16(x&0xffff)<<16) | (bswap_16(x>>16));
}

static inline unsigned long long bswap_64(unsigned long long x) {
	return (((unsigned long long)bswap_32(x&0xffffffffull))<<32) | (bswap_32(x>>32));
}

/*template<typename T>
T BEToPlat(T input) {
	T r = input;
	//for now, we will assume that the platform is LE
	switch(sizeof(r)) {
		case 8:
			r = r;
			break;
		case 16:
			r = bswap_16(r);
			break;
		case 32:
			r = bswap_32(r);
			break;
		case 64:
			r = bswap_64(r);
			break;
	}

	return r;
}*/

template<int bits>
secVT getExecPESections(PeLib::PeFile   *file) {
    const PeLib::PeHeaderT<bits>    &peh = 
        static_cast<PeLib::PeFileT<bits>&>(*file).peHeader();
    PeLib::word     numSections = peh.calcNumberOfSections();
    PeLib::dword    offRVA = peh.getBaseOfCode();
    PeLib::dword    offVA = peh.rvaToVa(offRVA);
    PeLib::dword    off = peh.vaToOffset(offVA);
    secVT           vec;

    FILE            *f = fopen(file->getFileName().c_str(), "rb");

    assert(f != NULL);
    assert(bits == 32 || bits == 64);
    TargetArch peArch;
    peArch.ta = INVALID;
    switch(bits) {
        case 32:
            peArch.ta = X86;
            break;
        case 64:
            peArch.ta = AMD64;
            break;
    }

    while( numSections > 0 ) {
        PeLib::word     curSecIdx = peh.getSectionWithOffset(off);
        PeLib::dword    secLen = peh.getSizeOfRawData(curSecIdx);
        PeLib::dword    secChars = peh.getCharacteristics(curSecIdx);

        if( secChars & PeLib::PELIB_IMAGE_SCN_MEM_EXECUTE ) {
            unsigned char   *buf = (unsigned char *)malloc(secLen);

            assert(buf != NULL);

            fseek(f, 0, SEEK_SET);
            fseek(f, off, SEEK_SET);

            size_t read = fread(buf, sizeof(unsigned char), secLen, f);

            if( read == secLen ) {
                unsigned long long va = peh.offsetToVa(off);
                lenAddrT  pv = lenAddrT(secLen, va);
                secPT p = secPT(buf, pv);
                secAndArchT n = secAndArchT(peArch, p);
                vec.push_back(n);
            } else{
                //error
                free(buf);
            }
        }

        off += secLen;
        numSections--;
    }

    fclose(f);
    return vec;
}

secVT findInPE(std::string path) {
    secVT           found;
    PeLib::PeFile   *pEF = PeLib::openPeFile(path);

    if( pEF != NULL ) {
        if( pEF->readMzHeader() == 0 && pEF->readPeHeader() == 0 ) {
            //we are ready to make the grab
        switch( pEF->getBits() ) {
            case 32:
                return getExecPESections<32>(pEF);
                break;

            case 64:
                return getExecPESections<64>(pEF);
                break;
            }
        }
    }

    return found;
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

secVT findInMachFromBuff(uint8_t *buf, uint32_t len, TargetArch t) {
	secVT found;
    
#ifndef WIN32
	mach_header	*m32 = (mach_header *)buf;
	if( bswap_32(m32->magic) == MH_CIGAM ) {
		//is 32-bit MACH (probably)
		
		//does this mach object have a CPU type that we are interested in?
		bool		isMatch=false;
		uint32_t	cpuType = m32->cputype;
		switch(t.ta) {
			case X86:
				if( cpuType == CPU_TYPE_X86 ) {
					isMatch = true;
				}
				break;
			case ARM:
				if( cpuType == CPU_TYPE_ARM ) {
					isMatch = true;
				}
				break;
            case AMD64:
                if( cpuType == CPU_TYPE_X86_64 ) {
                    isMatch = true;
                }
                break;
            default:
                isMatch = false;
		}
		if( isMatch ) {
			//if we can find anything executable, we'll generate something
			//walk over all the loader commands
			uint8_t	*cur = (uint8_t *) (((ptrdiff_t)buf) + sizeof(mach_header));

			uint32_t	numCmds = m32->ncmds;
			for( int i = 0; i < numCmds; i++ ) {
				load_command	*l = (load_command *)cur;
				uint32_t		cmd_sz = l->cmdsize;

				switch( l->cmd ) {
					case LC_SEGMENT:
					{
						segment_command	*sc = 
                            (segment_command *) (((ptrdiff_t)cur) );
						
						uint32_t	numSects = sc->nsects;
						
						section	*sec = 
                            (section *)(((ptrdiff_t)sc)+sizeof(segment_command)); 
						//iterate over the number of sections
						for( int k = 0; k < numSects; k++ ) {

							if( sc->initprot & VM_PROT_EXECUTE ) {
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
	} else {
		mach_header_64	*m64 = (mach_header_64 *)buf;
		if( m64->magic == MH_MAGIC_64 ) {
            //does this mach object have a CPU type that we are interested in?
            bool		isMatch=false;
            uint32_t	cpuType = m64->cputype;
            switch(t.ta) {
                case X86:
                    if( cpuType == CPU_TYPE_X86 ) {
                        isMatch = true;
                    }
                    break;
                case ARM:
                    if( cpuType == CPU_TYPE_ARM ) {
                        isMatch = true;
                    }
                    break;
                case AMD64:
                    if( cpuType == CPU_TYPE_X86_64 ) {
                        isMatch = true;
                    }
                    break;
                default:
                    isMatch = false;
            }
            if( isMatch ) {
                //if we can find anything executable, we'll generate something
                //walk over all the loader commands
                struct load_command * cur = 
                (struct load_command *) (((ptrdiff_t)buf) + 
                sizeof(mach_header_64));

                for( int i = 0; i < m64->ncmds; i++ ) {
                    switch(cur->cmd) {
                        case LC_SEGMENT_64:
                        {
                            segment_command_64	*sc = 
                                (segment_command_64 *) (((ptrdiff_t)cur) );
                            
                            uint32_t	numSects = sc->nsects;
                            
                            section_64	*sec = 
                    (section_64 *)(((ptrdiff_t)sc)+sizeof(segment_command_64)); 
                            //iterate over the number of sections
                            for( int k = 0; k < numSects; k++ ) {

                                if( sc->initprot & VM_PROT_EXECUTE ) {
                                    //save this one
                                    uint32_t	addr = sec->addr;
                                    uint32_t	bytesOff = sec->offset;
                                    uint32_t	size = sec->size;
                                    uint8_t		*sourceBytes = 
                                (uint8_t *) ( ((ptrdiff_t)buf) + bytesOff);
                                    //copy from sourceBytes 

                                    uint8_t     *dstBytes = 
                                        (uint8_t *) malloc(size);

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

                    cur = (struct load_command *)
                        (((ptrdiff_t)cur) + cur->cmdsize);
                }
            }
        }
	}
#endif

	return found;
}

secVT findInMachMapped(uint8_t *fileBuf, uint32_t len, TargetArch t) {
    secVT   found;
#ifndef WIN32
    //what kind of magic is at the beginning of the file buffer?
    fat_header	*n = (fat_header *)fileBuf;
    if( bswap_32(n->magic) == FAT_MAGIC ) {
        //this is a FAT file
        fat_arch	*fats = 
            (fat_arch*)( ((ptrdiff_t)fileBuf)+sizeof(fat_header) );

        //call findInMachFromBuff on each
        uint32_t	numArches = bswap_32(n->nfat_arch);
        for( int i = 0; i < numArches; i++ ) {
            uint32_t	fatsOff = bswap_32(fats[i].offset);
            uint8_t	*newBuff = (uint8_t*) ( ((ptrdiff_t)fileBuf) + fatsOff);

            secVT tmp = findInMachFromBuff(newBuff, len-fatsOff, t);

            found.insert( found.end(), tmp.begin(), tmp.end() );
        }
    } else {
        found = findInMachFromBuff(fileBuf, len, t);
    }
#endif
    return found;
}

//this file might or might not be a FAT object
secVT findInMach(std::string path, TargetArch t) {
	secVT	found;

	//open and memory map the file
	FILE *f = fopen(path.c_str(), "rb");
	if( f ) {
		uint32_t    len;
		fseek(f, 0L, SEEK_END);
		len = ftell(f);
		fseek(f, 0L, SEEK_SET);
		uint8_t	*fileBuf = memMapFile(f, len, 0);

		assert(fileBuf != NULL );
        
        found = findInMachMapped(fileBuf, len, t);

		fclose(f);
	}

	return found;
}

secVT getRawFromBuff(uint8_t *buf, uint32_t len, TargetArch t) {
    secVT   s;
    uint8_t     *buf2 = (uint8_t *)malloc(len + 8);
    assert(buf2 != NULL);
    memcpy(buf2, buf, len);
    free(buf);
    *(buf2+len) = 0xe9;
    *(buf2+len+1) = 0xec;
    *(buf2+len+2) = 0x5b;
    *(buf2+len+3) = 0x00;
    *(buf2+len+4) = 0xea;
    *(buf2+len+5) = 0xad;
    *(buf2+len+6) = 0xbe;
    *(buf2+len+7) = 0xef;
    lenAddrT    pv = lenAddrT(len, 0x1000);
    secPT       p = secPT(buf2, pv);
    secAndArchT at = secAndArchT(t,p);

    s.push_back(at);

    return s;
}

secVT getRaw(std::string path, TargetArch t) {
    secVT   s;
    //read in the entire file contents as a buffer
    FILE    *f = fopen(path.c_str(), "rb");

    if( f ) {
        uint32_t    len;
        fseek(f, 0L, SEEK_END);
        len = ftell(f);
        fseek(f, 0L, SEEK_SET);
        uint8_t     *buf = memMapFile(f, len, 0);

        if( buf ) {
            s = getRawFromBuff(buf, len, t);
       }

       fclose(f);
    }
    
    return s;
}

secVT getDyld(std::string path, TargetArch t) {
    secVT                           secs;
#ifndef WIN32
    dyld_decache::ProgramContext    pc(path);

    if( pc.open() ) {
        //we opened a shared cache file, yay
        
    }
#endif
    return secs;
}

secVT getExecSections(std::string path, FileFormat f, TargetArch t) {
    secVT   secs;
    switch(f) {
        case PEFmt:
            secs = findInPE(path);
            break;
        case MachOFmt:
            secs = findInMach(path, t);
            break;
        case RawFmt:
            secs = getRaw(path, t);
            break;
        case Invalid:
            break;
        case DyldCacheFmt:
            secs = getDyld(path, t);
            break;
    }

    return secs;
}

ExecCodeProvider::ExecCodeProvider(std::string p, FileFormat f, TargetArch t) {
    this->arch = t;
    this->err = false;
    this->buf = NULL;
    this->bufLen = 0;
    this->fName = p;
    this->peCtx = NULL;
    this->dyldCtx = NULL;

    // open and map the executable
    FILE *fp = fopen(p.c_str(), "rb");
    if( fp ) {
        uint32_t    len;
        fseek(fp, 0L, SEEK_END);
        len = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        uint8_t	*fileBuf = memMapFile(fp, len, 0);
        assert(fileBuf != NULL);
        this->buf = fileBuf;
        this->bufLen = len;
    }

    std::cout << "Executable Type: ";
    
    // Windows Executable
    if(memcmp(this->buf, "MZ\x90\x00", 4) == 0)
    {
        std::cout << "Windows Executable" << std::endl;
        this->fmt = PEFmt;
        this->peCtx = PeLib::openPeFile(p);
    }
    
    // ELF's
    else if(memcmp(this->buf, "\x7f\x45\x4c\x46", 4) == 0)
    {
        std::cout << "Linux ELF" << std::endl;
        this->fmt = ELFFmt;
    }

    // MachO's
    else if((memcmp(this->buf, "\xce\xfa\xed\xfe", 4) == 0) || 
            (memcmp(this->buf, "\xcf\xfa\xed\xfe", 4) == 0))
    {
        std::cout << "MachO Executable" << std::endl;
        this->fmt = MachOFmt;
    }
    
    // DynLib's
    else if(memcmp(this->buf, "\xca\xfe\xba\xbe", 4) == 0)
    {
        std::cout << "Mac Dynlib" << std::endl;
        this->fmt = DyldCacheFmt;

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

    }

    // handle unrecognized binaries
    else
    {
        std::cout << "Unrecognized Binary" << std::endl;
        this->fmt = RawFmt; // or Invalid
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
            PeLib::PeFile   *peF = (PeLib::PeFile *)this->peCtx;
            found.push_back(peF->getFileName());
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

secVT ExecCodeProvider::sections_in_file(std::string file) {
    //how do you shoot a pink elephant?
    //using a pink elephant gun
    //how do you shoot a blue elephant?
    //you twist its trunk until it turns blue, and then you shoot it with the
    //blue elephant gun
    secVT   secs;

    switch(this->fmt) {
        case DyldCacheFmt:
        {
#ifndef WIN32
            dyld_decache::ProgramContext *dy = 
                (dyld_decache::ProgramContext *) this->dyldCtx;
            std::list<std::string>  fileNames = dy->list_of_images();
            for(std::list<std::string>::iterator it = fileNames.begin();
                it != fileNames.end();
                ++it)
            {
                std::string n = *it;

                if( file == n ) {
                    //we found the file, now write it out
                    dy->save_image_with_path(n, "foo.bin");
                    secs = findInMach("foo.bin", this->arch);
                    //erase the temporary file
                    boost::filesystem::remove("foo.bin");
                    break;
                }
            }
#endif
        }
            break;

        case PEFmt:
        {
            PeLib::PeFile   *pEF = (PeLib::PeFile *) this->peCtx; 
            if( pEF != NULL ) {
                if( pEF->readMzHeader() == 0 && pEF->readPeHeader() == 0 ) {
                    switch( pEF->getBits() ) {
                        case 32:
                            secs = getExecPESections<32>(pEF);
                            break;

                        case 64:
                            secs = getExecPESections<64>(pEF);
                            break;
                    }
                }
            }
        }
            break;

        case MachOFmt:
        {
            secs = findInMachMapped(this->buf, this->bufLen, this->arch);
        }
            break;
        
        case RawFmt:
        {
            secs = getRawFromBuff(this->buf, this->bufLen, this->arch);
        }
            break;

        case Invalid:
            break;
    }

    return secs;
}
