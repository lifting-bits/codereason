/*
    dyld_decache.cpp ... Extract dylib files from shared cache.
    Copyright (C) 2011  KennyTM~ <kennytm@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
   With reference to DHowett's dyldcache.cc, with the following condition:

    "if you find it useful, do whatever you want with it. just don't forget that
     somebody helped."

   see http://blog.howett.net/?p=75 for detail.
*/

/*
    Part of code is referenced from Apple's dyld project, with the following li-
    cense:
*/

    /* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
        *
        * Copyright (c) 2006-2008 Apple Inc. All rights reserved.
        *
        * @APPLE_LICENSE_HEADER_START@
        *
        * This file contains Original Code and/or Modifications of Original Code
        * as defined in and that are subject to the Apple Public Source License
        * Version 2.0 (the 'License'). You may not use this file except in
        * compliance with the License. Please obtain a copy of the License at
        * http://www.opensource.apple.com/apsl/ and read it before using this
        * file.
        *
        * The Original Code and all software distributed under the License are
        * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
        * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
        * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
        * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
        * Please see the License for the specific language governing rights and
        * limitations under the License.
        *
        * @APPLE_LICENSE_HEADER_END@
        */

//------------------------------------------------------------------------------
// END LEGALESE
//------------------------------------------------------------------------------

// g++ -o dyld_decache -O3 -Wall -Wextra -std=c++98 /usr/local/lib/libboost_filesystem-mt.a /usr/local/lib/libboost_system-mt.a dyld_decache.cpp DataFile.cpp

//#include <unistd.h>
#include <cstdio>
#include <stdint.h>
//#include <getopt.h>
#include "DataFile.h"
#include <string>
#include <vector>
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
#include <utility>
#include <boost/unordered_map.hpp>
#include <boost/foreach.hpp>
#include <boost/cstdint.hpp>

bool streq(const char x[16], const char* y);

long write_uleb128(FILE* f, unsigned u);

boost::filesystem::path remove_all_extensions(const char* the_path);

namespace dyld_decache {

struct dyld_cache_header {
	char		magic[16];
	uint32_t	mappingOffset;
	uint32_t	mappingCount;
	uint32_t	imagesOffset;
	uint32_t	imagesCount;
	uint64_t	dyldBaseAddress;
};

typedef boost::uint64_t		mach_vm_address_t;
typedef boost::uint64_t		mach_vm_offset_t;
typedef boost::uint64_t		mach_vm_size_t;
typedef boost::int32_t vm_prot_t;

struct shared_file_mapping_np {
	mach_vm_address_t	sfm_address;
	mach_vm_size_t		sfm_size;
	mach_vm_offset_t	sfm_file_offset;
	vm_prot_t		sfm_max_prot;
	vm_prot_t		sfm_init_prot;
};

struct dyld_cache_image_info {
	boost::uint64_t	address;
	boost::uint64_t	modTime;
	boost::uint64_t	inode;
	boost::uint32_t	pathFileOffset;
	boost::uint32_t	pad;
};

typedef boost::uint32_t		integer_t;
typedef integer_t	cpu_type_t;
typedef integer_t	cpu_subtype_t;

struct mach_header {
	boost::uint32_t	magic;
	cpu_type_t	cputype;
	cpu_subtype_t	cpusubtype;
	boost::uint32_t	filetype;
	boost::uint32_t	ncmds;
	boost::uint32_t	sizeofcmds;
	boost::uint32_t	flags;
};

struct load_command {
	boost::uint32_t cmd;
	boost::uint32_t cmdsize;
};

#define LC_REQ_DYLD 0x80000000

#define	LC_SEGMENT	0x1
#define	LC_SYMTAB	0x2
#define	LC_SYMSEG	0x3
#define	LC_THREAD	0x4
#define	LC_UNIXTHREAD	0x5
#define	LC_LOADFVMLIB	0x6
#define	LC_IDFVMLIB	0x7
#define	LC_IDENT	0x8
#define LC_FVMFILE	0x9
#define LC_PREPAGE      0xa
#define	LC_DYSYMTAB	0xb
#define	LC_LOAD_DYLIB	0xc
#define	LC_ID_DYLIB	0xd
#define LC_LOAD_DYLINKER 0xe
#define LC_ID_DYLINKER	0xf
#define	LC_PREBOUND_DYLIB 0x10
#define	LC_ROUTINES	0x11
#define	LC_SUB_FRAMEWORK 0x12
#define	LC_SUB_UMBRELLA 0x13
#define	LC_SUB_CLIENT	0x14
#define	LC_SUB_LIBRARY  0x15
#define	LC_TWOLEVEL_HINTS 0x16
#define	LC_PREBIND_CKSUM  0x17
#define	LC_LOAD_WEAK_DYLIB (0x18 | LC_REQ_DYLD)
#define	LC_SEGMENT_64	0x19
#define	LC_ROUTINES_64	0x1a
#define LC_UUID		0x1b
#define LC_RPATH       (0x1c | LC_REQ_DYLD)
#define LC_CODE_SIGNATURE 0x1d
#define LC_SEGMENT_SPLIT_INFO 0x1e
#define LC_REEXPORT_DYLIB (0x1f | LC_REQ_DYLD)
#define	LC_LAZY_LOAD_DYLIB 0x20
#define	LC_ENCRYPTION_INFO 0x21
#define	LC_DYLD_INFO 	0x22
#define	LC_DYLD_INFO_ONLY (0x22|LC_REQ_DYLD)
#define LC_LOAD_UPWARD_DYLIB (0x23|LC_REQ_DYLD)
#define LC_VERSION_MIN_MACOSX 0x24
#define LC_VERSION_MIN_IPHONEOS 0x25
#define LC_FUNCTION_STARTS 0x26
#define LC_DYLD_ENVIRONMENT 0x27

struct segment_command : public load_command {
	char		segname[16];
	boost::uint32_t	vmaddr;
	boost::uint32_t	vmsize;
	boost::uint32_t	fileoff;
	uint32_t	filesize;
	vm_prot_t	maxprot;
	vm_prot_t	initprot;
	uint32_t	nsects;
	uint32_t	flags;
};

struct section {
	char		sectname[16];
	char		segname[16];
	uint32_t	addr;
	uint32_t	size;
	uint32_t	offset;
	uint32_t	align;
	uint32_t	reloff;
	uint32_t	nreloc;
	uint32_t	flags;
	uint32_t	reserved1;
	uint32_t	reserved2;
};

struct symtab_command : public load_command {
	uint32_t	symoff;
	uint32_t	nsyms;
	uint32_t	stroff;
	uint32_t	strsize;
};

struct symseg_command : public load_command {
	uint32_t	offset;
	uint32_t	size;
};

struct dysymtab_command : public load_command {
    uint32_t ilocalsym;
    uint32_t nlocalsym;
    uint32_t iextdefsym;
    uint32_t nextdefsym;
    uint32_t iundefsym;
    uint32_t nundefsym;
    uint32_t tocoff;
    uint32_t ntoc;
    uint32_t modtaboff;
    uint32_t nmodtab;
    uint32_t extrefsymoff;
    uint32_t nextrefsyms;
    uint32_t indirectsymoff;
    uint32_t nindirectsyms;
    uint32_t extreloff;
    uint32_t nextrel;
    uint32_t locreloff;
    uint32_t nlocrel;
};

struct twolevel_hints_command : public load_command {
    uint32_t offset;
    uint32_t nhints;
};

struct segment_command_64 : public load_command {
	char		segname[16];
	uint64_t	vmaddr;
	uint64_t	vmsize;
	uint64_t	fileoff;
	uint64_t	filesize;
	vm_prot_t	maxprot;
	vm_prot_t	initprot;
	uint32_t	nsects;
	uint32_t	flags;
};

struct section_64 {
	char		sectname[16];
	char		segname[16];
	uint64_t	addr;
	uint64_t	size;
	uint32_t	offset;
	uint32_t	align;
	uint32_t	reloff;
	uint32_t	nreloc;
	uint32_t	flags;
	uint32_t	reserved1;
	uint32_t	reserved2;
	uint32_t	reserved3;
};

struct linkedit_data_command : public load_command {
    uint32_t	dataoff;
    uint32_t	datasize;
};

struct encryption_info_command : public load_command {
   uint32_t	cryptoff;
   uint32_t	cryptsize;
   uint32_t	cryptid;
};

struct dyld_info_command : public load_command {
    uint32_t   rebase_off;
    uint32_t   rebase_size;
    uint32_t   bind_off;
    uint32_t   bind_size;
    uint32_t   weak_bind_off;
    uint32_t   weak_bind_size;
    uint32_t   lazy_bind_off;
    uint32_t   lazy_bind_size;
    uint32_t   export_off;
    uint32_t   export_size;
};

struct dylib {
    uint32_t name;
    uint32_t timestamp;
    uint32_t current_version;
    uint32_t compatibility_version;
};

struct dylib_command : public load_command {
    struct dylib dylib;
};

struct nlist {
	int32_t n_strx;
	uint8_t n_type;
	uint8_t n_sect;
	int16_t n_desc;
	uint32_t n_value;
};

struct class_t {
    uint32_t isa;
    uint32_t superclass;
    uint32_t cache;
    uint32_t vtable;
    uint32_t data;
};

struct class_ro_t {
    uint32_t flags;
    uint32_t instanceStart;
    uint32_t instanceSize;
    uint32_t ivarLayout;
    uint32_t name;
    uint32_t baseMethods;
    uint32_t baseProtocols;
    uint32_t ivars;
    uint32_t weakIvarLayout;
    uint32_t baseProperties;
};

struct method_t {
    uint32_t name;
    uint32_t types;
    uint32_t imp;
};

struct protocol_t {
    uint32_t isa;
    uint32_t name;
    uint32_t protocols;
    uint32_t instanceMethods;
    uint32_t classMethods;
    uint32_t optionalInstanceMethods;
    uint32_t optionalClassMethods;
    uint32_t instanceProperties;
};

struct category_t {
    uint32_t name;
    uint32_t cls;
    uint32_t instanceMethods;
    uint32_t classMethods;
    uint32_t protocols;
    uint32_t instanceProperties;
};

#define BIND_OPCODE_MASK					0xF0
#define BIND_IMMEDIATE_MASK					0x0F
#define BIND_OPCODE_DONE					0x00
#define BIND_OPCODE_SET_DYLIB_ORDINAL_IMM			0x10
#define BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB			0x20
#define BIND_OPCODE_SET_DYLIB_SPECIAL_IMM			0x30
#define BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM		0x40
#define BIND_OPCODE_SET_TYPE_IMM				0x50
#define BIND_OPCODE_SET_ADDEND_SLEB				0x60
#define BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB			0x70
#define BIND_OPCODE_ADD_ADDR_ULEB				0x80
#define BIND_OPCODE_DO_BIND					0x90
#define BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB			0xA0
#define BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED			0xB0
#define BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB		0xC0


//------------------------------------------------------------------------------
// END THIRD-PARTY STRUCTURES
//------------------------------------------------------------------------------

class ProgramContext;

// When dyld create the cache file, if it recognize common Objective-C strings
//  and methods across different libraries, they will be coalesced. However,
//  this poses a big trouble when decaching, because the references to the other
//  library will become a dangling pointer. This class is to store these
//  external references, and put them back in an extra section of the decached
//  library.
// ("String" is a misnomer because it can also store non-strings.)
class ExtraStringRepository {
    struct Entry {
        const char* string;
        size_t size;
        uint32_t new_address;
        std::vector<uint32_t> override_addresses;
    };

    boost::unordered_map<const char*, int> _indices;
    std::vector<Entry> _entries;
    size_t _total_size;

    section _template;

public:
    ExtraStringRepository(const char* segname, const char* sectname, uint32_t flags, uint32_t alignment) {
        memset(&_template, 0, sizeof(_template));
        strncpy(_template.segname, segname, 16);
        strncpy(_template.sectname, sectname, 16);
        _template.flags = flags;
        _template.align = alignment;
    }

    // Insert a piece of external data referred from 'override_address' to the
    //  repository.
    void insert(const char* string, size_t size, uint32_t override_address) {
        boost::unordered_map<const char*, int>::const_iterator it = _indices.find(string);
        if (it != _indices.end()) {
            _entries[it->second].override_addresses.push_back(override_address);
        } else {
            Entry entry;
            entry.string = string;
            entry.size = size;
            entry.new_address = this->next_vmaddr();
            entry.override_addresses.push_back(override_address);
            _indices.insert(std::make_pair(string, _entries.size()));
            _entries.push_back(entry);
            _template.size += size;
        }
    }

    void insert(const char* string, uint32_t override_address) {
        this->insert(string, strlen(string) + 1, override_address);
    }

    // Iterate over all external data in this repository.
    template <typename Object>
    void foreach_entry(const Object* self, void (Object::*action)(const char* string, size_t size, uint32_t new_address, const std::vector<uint32_t>& override_addresses) const) const {
        BOOST_FOREACH(const Entry& e, _entries) {
            (self->*action)(e.string, e.size, e.new_address, e.override_addresses);
        }
    }

    void increase_size_by(size_t delta) { _template.size += delta; }
    size_t total_size() const { return _template.size; }
    bool has_content() const { return _template.size != 0; }
    // Get the 'section' structure for the extra section this repository
    //  represents.
    section section_template() const { return _template; }

    void set_section_vmaddr(uint32_t vmaddr) { _template.addr = vmaddr; }
    void set_section_fileoff(uint32_t fileoff) { _template.offset = fileoff; }
    uint32_t next_vmaddr() const { return _template.addr + _template.size; }
};

class ExtraBindRepository {
    struct Entry {
        std::string symname;
        int libord;
        std::vector<std::pair<int, uint32_t> > replace_offsets;
    };
    
    boost::unordered_map<uint32_t, Entry> _entries;
    
public:
    bool contains(uint32_t target_address) const {
        return (_entries.find(target_address) != _entries.end());
    }
    
    template <typename Object>
    void insert(uint32_t target_address, std::pair<int, uint32_t> replace_offset, const Object* self, void (Object::*addr_info_getter)(uint32_t addr, std::string* p_symname, int* p_libord) const) {
        boost::unordered_map<uint32_t, Entry>::iterator it = _entries.find(target_address);
        if (it != _entries.end()) {
            it->second.replace_offsets.push_back(replace_offset);
        } else {
            Entry entry;
            entry.replace_offsets.push_back(replace_offset);
            (self->*addr_info_getter)(target_address, &entry.symname, &entry.libord);
            _entries.insert(std::make_pair(target_address, entry));
        }
    }
    
    long optimize_and_write(FILE* f) {
        typedef boost::unordered_map<uint32_t, Entry>::value_type V;
        typedef boost::unordered_map<int, std::vector<const Entry*> > M;
        typedef std::pair<int, uint32_t> P;
        
        M entries_by_libord;
        
        BOOST_FOREACH(V& pair, _entries) {
            Entry& entry = pair.second;
            std::sort(entry.replace_offsets.begin(), entry.replace_offsets.end());
            entries_by_libord[entry.libord].push_back(&entry);
        }
        
        fputc(BIND_OPCODE_SET_TYPE_IMM | 1, f);
        
        long size = 1;
        BOOST_FOREACH(const M::value_type& pair, entries_by_libord) {
            int libord = pair.first;
            if (libord < 0x10) {
                unsigned char imm = libord & BIND_IMMEDIATE_MASK;
                unsigned char opcode = libord < 0 ? BIND_OPCODE_SET_DYLIB_SPECIAL_IMM : BIND_OPCODE_SET_DYLIB_ORDINAL_IMM;
                fputc(opcode | imm, f);
                ++ size;
            } else {
                fputc(BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB, f);
                size += 1 + write_uleb128(f, libord);
            }
            
            BOOST_FOREACH(const Entry* entry, pair.second) {
                size_t string_len = entry->symname.size();
                size += string_len + 2;
                fputc(BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM, f);
                fwrite(entry->symname.c_str(), string_len+1, 1, f);
                
                int segnum = -1;
                uint32_t last_offset = 0;
                BOOST_FOREACH(P offset, entry->replace_offsets) {
                    if (offset.first != segnum) {
                        segnum = offset.first;
                        last_offset = offset.second + 4;
                        fputc(BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB | segnum, f);
                        size += 1 + write_uleb128(f, offset.second);
                    } else {
                        uint32_t delta = offset.second - last_offset;
                        unsigned imm_scale = delta % 4 == 0 ? delta / 4 : ~0u;
                        if (imm_scale == 0) {
                            fputc(BIND_OPCODE_DO_BIND, f);
                        } else if (imm_scale < 0x10u) {
                            fputc(BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED | imm_scale, f);
                        } else {
                            fputc(BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB, f);
                            size += write_uleb128(f, delta);
                        }
                        ++ size;
                        last_offset = offset.second + 4;
                    }
                }
                fputc(BIND_OPCODE_DO_BIND, f);
                ++ size;
            }
        }
        
        return size;
    }
};

// A simple structure which only provides services related to VM address.
class MachOFile {
protected:
    const mach_header* _header;
    const ProgramContext* _context;
    std::vector<const segment_command*> _segments;
    uint32_t _image_vmaddr;
    
private:
    boost::unordered_map<std::string, int> _libords;
    int _cur_libord;
    boost::unordered_map<uint32_t, std::string> _exports;

protected:
    template <typename T>
    void foreach_command(void(T::*action)(const load_command* cmd)) {
        const unsigned char* cur_cmd = reinterpret_cast<const unsigned char*>(_header + 1);

        for (uint32_t i = 0; i < _header->ncmds; ++ i) {
            const load_command* cmd = reinterpret_cast<const load_command*>(cur_cmd);
            cur_cmd += cmd->cmdsize;

            (static_cast<T*>(this)->*action)(cmd);
        }
    }

    // Convert VM address to file offset of the decached file _before_ inserting
    //  the extra sections.
    long from_vmaddr(uint32_t vmaddr) const {
        BOOST_FOREACH(const segment_command* segcmd, _segments) {
            if (segcmd->vmaddr <= vmaddr && vmaddr < segcmd->vmaddr + segcmd->vmsize)
                return vmaddr - segcmd->vmaddr + segcmd->fileoff;
        }
        return -1;
    }

private:
    void retrieve_segments_and_libords(const load_command* cmd);

public:
    // Checks if the VM address is included in the decached file _before_
    //  inserting the extra sections.
    bool contains_address(uint32_t vmaddr) const {
        BOOST_FOREACH(const segment_command* segcmd, _segments) {
            if (segcmd->vmaddr <= vmaddr && vmaddr < segcmd->vmaddr + segcmd->vmsize)
                return true;
        }
        return false;
    }
    
    MachOFile(const mach_header* header, const ProgramContext* context, uint32_t image_vmaddr = 0)
        : _header(header), _context(context), _image_vmaddr(image_vmaddr), _cur_libord(0)
    {
        if (header->magic != 0xfeedface)
            return;

        this->foreach_command(&MachOFile::retrieve_segments_and_libords);
    }

    const mach_header* header() const { return _header; }
    
    int libord_with_name(const char* libname) const {
        boost::unordered_map<std::string, int>::const_iterator cit = _libords.find(libname); 
        if (cit == _libords.end())
            return 0;
        else
            return cit->second;
    }
    
    std::string exported_symbol(uint32_t vmaddr) const {
        boost::unordered_map<uint32_t, std::string>::const_iterator cit = _exports.find(vmaddr);
        if (cit != _exports.end())
            return cit->second;
        else
            return "";
    }
};

// This class represents one file going to be decached.
// Decaching is performed in several phases:
//  1. Search for all Objective-C selectors and methods that point outside of
//     this library, and put this into an ExtraStringRepository.
//  2. Write out the __TEXT and __DATA segments, including the data from the
//     ExtraStringRepository.
//  3. Inspect the DYLD_INFO, SYMTAB and DYSYMTAB commands to collect the
//     relevant parts of global __LINKEDIT segment and copy them to the output
//     file.
//  4. Revisit the output file to fix the file offsets. All file offsets were
//     originally pointing to locations in the cache file, but in the decached
//     file these will be no longer meaningful if not fixed.
//  5. Append the extra 'section' header to the corresponding segments, if there
//     are external Objective-C selectors or methods.
//  6. Go through the Objective-C sections and rewire the external references.
class DecachingFile : public MachOFile {
    struct FileoffFixup {
        uint32_t sourceBegin;
        uint32_t sourceEnd;
        int32_t negDelta;
    };

    struct ObjcExtraString {
        const char* string;
        size_t entry_size;
        uint32_t new_address;
        off_t override_offset;
    };

    struct {
        long rebase_off, bind_off, weak_bind_off,
               lazy_bind_off, export_off,            // dyld_info
             symoff, stroff,                         // symtab
             tocoff, modtaboff, extrefsymoff,
               indirectsymoff, extreloff, locreloff, // dysymtab
             dataoff,    // linkedit_data_command (dummy)
               dataoff_cs, dataoff_ssi, dataoff_fs;
        long bind_size;
        int32_t strsize;
    } _new_linkedit_offsets;

private:
    uint32_t _linkedit_offset, _linkedit_size;
    uint32_t _imageinfo_address, _imageinfo_replacement;

    FILE* _f;
    std::vector<FileoffFixup> _fixups;
    std::vector<segment_command> _new_segments;
    ExtraStringRepository _extra_text, _extra_data;
    std::vector<uint32_t> _entsize12_patches, _nullify_patches;
    ExtraBindRepository _extra_bind;

private:
    void open_file(const boost::filesystem::path& filename) {
        //boost::filesystem::create_directories(filename.parent_path());
        const char *st = filename.c_str();
        _f = fopen(st, "wb");
        if (!_f) {
            perror("Error");
            fprintf(stderr, "Error: Cannot write to '%s'.\n", filename.c_str());
        }
    }

    void write_extrastr(const char* string, size_t size, uint32_t, const std::vector<uint32_t>&) const {
        fwrite(string, size, 1, _f);
    }

    void write_segment_content(const segment_command* cmd);

    ExtraStringRepository* repo_for_segname(const char* segname) {
        if (!strcmp(segname, "__DATA"))
            return &_extra_data;
        else if (!strcmp(segname, "__TEXT"))
            return &_extra_text;
        return NULL;
    }

    template<typename T>
    void fix_offset(T& fileoff) const {
        if (fileoff == 0)
            return;

        BOOST_REVERSE_FOREACH(const FileoffFixup& fixup, _fixups) {
            if (fixup.sourceBegin <= fileoff && fileoff < fixup.sourceEnd) {
                fileoff -= fixup.negDelta;
                return;
            }
        }
    }

    void write_real_linkedit(const load_command* cmd);

    void fix_file_offsets(const load_command* cmd) {
        switch (cmd->cmd) {
            default:
                fwrite(cmd, cmd->cmdsize, 1, _f);
                break;

            case LC_SEGMENT: {
                segment_command segcmd = *static_cast<const segment_command*>(cmd);
                if (streq(segcmd.segname, "__LINKEDIT")) {
                    segcmd.vmsize = _linkedit_size;
                    segcmd.fileoff = _linkedit_offset;
                    segcmd.filesize = _linkedit_size;
                    fwrite(&segcmd, sizeof(segcmd), 1, _f);
                } else {
                    const ExtraStringRepository* extra_repo = this->repo_for_segname(segcmd.segname);
                    bool has_extra_sect = extra_repo && extra_repo->has_content();

                    this->fix_offset(segcmd.fileoff);
                    section* sects = new section[segcmd.nsects + has_extra_sect];
                    memcpy(sects, 1 + static_cast<const segment_command*>(cmd), segcmd.nsects * sizeof(*sects));
                    for (uint32_t i = 0; i < segcmd.nsects; ++ i) {
                        this->fix_offset(sects[i].offset);
                        this->fix_offset(sects[i].reloff);
                    }
                    if (has_extra_sect) {
                        uint32_t extra_sect_size = extra_repo->total_size();
                        sects[segcmd.nsects] = extra_repo->section_template();
                        segcmd.cmdsize += sizeof(*sects);
                        segcmd.vmsize += extra_sect_size;
                        segcmd.filesize += extra_sect_size;
                        segcmd.nsects += 1;
                    }
                    fwrite(&segcmd, sizeof(segcmd), 1, _f);
                    fwrite(sects, sizeof(*sects), segcmd.nsects, _f);
                    delete[] sects;
                }
                _new_segments.push_back(segcmd);
                break;
            }

            case LC_SYMTAB: {
                symtab_command symcmd = *static_cast<const symtab_command*>(cmd);
                symcmd.symoff = _new_linkedit_offsets.symoff;
                symcmd.stroff = _new_linkedit_offsets.stroff;
                symcmd.strsize = _new_linkedit_offsets.strsize;
                fwrite(&symcmd, sizeof(symcmd), 1, _f);
                break;
            }

            case LC_DYSYMTAB: {
                dysymtab_command dycmd = *static_cast<const dysymtab_command*>(cmd);
                dycmd.tocoff = _new_linkedit_offsets.tocoff;
                dycmd.modtaboff = _new_linkedit_offsets.modtaboff;
                dycmd.extrefsymoff = _new_linkedit_offsets.extrefsymoff;
                dycmd.indirectsymoff = _new_linkedit_offsets.indirectsymoff;
                dycmd.extreloff = _new_linkedit_offsets.extreloff;
                dycmd.locreloff = _new_linkedit_offsets.locreloff;
                fwrite(&dycmd, sizeof(dycmd), 1, _f);
                break;
            }

            case LC_TWOLEVEL_HINTS: {
                twolevel_hints_command tlcmd = *static_cast<const twolevel_hints_command*>(cmd);
                this->fix_offset(tlcmd.offset);
                fwrite(&tlcmd, sizeof(tlcmd), 1, _f);
                break;
            }

            /*
            case LC_SEGMENT_64: {
                segment_command_64 segcmd = *static_cast<const segment_command_64*>(cmd);
                this->fix_offset(segcmd.fileoff);
                fwrite(&segcmd, sizeof(segcmd), 1, _f);
                section_64* sects = new section_64[segcmd.nsects];
                memcpy(sects, 1 + static_cast<const segment_command_64*>(cmd), segcmd.nsects * sizeof(*sects));
                for (uint32_t i = 0; i < segcmd.nsects; ++ i) {
                    this->fix_offset(sects[i].offset);
                    this->fix_offset(sects[i].reloff);
                }
                fwrite(sects, sizeof(*sects), segcmd.nsects, _f);
                delete[] sects;
                break;
            }
            */

            case LC_CODE_SIGNATURE:
            case LC_SEGMENT_SPLIT_INFO: 
            case LC_FUNCTION_STARTS: {
                linkedit_data_command ldcmd = *static_cast<const linkedit_data_command*>(cmd);
                if (ldcmd.cmd == LC_CODE_SIGNATURE)
                    ldcmd.dataoff = _new_linkedit_offsets.dataoff_cs;
                else if (ldcmd.cmd == LC_SEGMENT_SPLIT_INFO)
                    ldcmd.dataoff = _new_linkedit_offsets.dataoff_ssi;
                else if (ldcmd.cmd == LC_FUNCTION_STARTS)
                    ldcmd.dataoff = _new_linkedit_offsets.dataoff_fs;
                fwrite(&ldcmd, sizeof(ldcmd), 1, _f);
                break;
            }

            case LC_ENCRYPTION_INFO: {
                encryption_info_command eicmd = *static_cast<const encryption_info_command*>(cmd);
                this->fix_offset(eicmd.cryptoff);
                fwrite(&eicmd, sizeof(eicmd), 1, _f);
                break;
            }

            case LC_DYLD_INFO:
            case LC_DYLD_INFO_ONLY: {
                dyld_info_command dicmd = *static_cast<const dyld_info_command*>(cmd);
                dicmd.rebase_off = _new_linkedit_offsets.rebase_off;
                dicmd.bind_off = _new_linkedit_offsets.bind_off;
                dicmd.weak_bind_off = _new_linkedit_offsets.weak_bind_off;
                dicmd.lazy_bind_off = _new_linkedit_offsets.lazy_bind_off;
                dicmd.export_off = _new_linkedit_offsets.export_off;
                dicmd.bind_size = _new_linkedit_offsets.bind_size;
                fwrite(&dicmd, sizeof(dicmd), 1, _f);
                break;
            }
        }
    }

    // Convert VM address to file offset of the decached file _after_ inserting
    //  the extra sections.
    long from_new_vmaddr(uint32_t vmaddr) const {
        std::vector<segment_command>::const_iterator nit;
        std::vector<const segment_command*>::const_iterator oit;
        
        std::vector<segment_command>::const_iterator end = _new_segments.end(); 
        for (nit = _new_segments.begin(), oit = _segments.begin(); nit != end; ++ nit, ++ oit) {
            if (nit->vmaddr <= vmaddr && vmaddr < nit->vmaddr + nit->vmsize) {
                uint32_t retval = vmaddr - nit->vmaddr + nit->fileoff;
                // This mess is added to solve the __DATA,__bss section issue.
                // This section is zero-filled, causing the segment's vmsize
                //  larger than the filesize. Since the __extradat section is
                //  placed after the __bss section, using just the formula above
                //  will cause the imaginary size comes from that section to be
                //  included as well. The "-=" below attempts to fix it.
                if (vmaddr >= (*oit)->vmaddr + (*oit)->vmsize)
                    retval -= (*oit)->vmsize - (*oit)->filesize;
                return retval;
            }
        }
        
        return -1;
    }
    
    // Get the segment number and offset from that segment given a VM address.
    std::pair<int, uint32_t> segnum_and_offset(uint32_t vmaddr) const {
        int i = 0;
        BOOST_FOREACH(const segment_command* segcmd, _segments) {
            if (segcmd->vmaddr <= vmaddr && vmaddr < segcmd->vmaddr + segcmd->vmsize)
                return std::make_pair(i, vmaddr - segcmd->vmaddr);
            ++ i;
        }
        return std::make_pair(-1, ~0u);
    }

    void prepare_patch_objc_methods(uint32_t method_vmaddr, uint32_t override_vmaddr);
    void prepare_objc_extrastr(const segment_command* segcmd);

    void get_address_info(uint32_t vmaddr, std::string* p_name, int* p_libord) const;
    void add_extlink_to(uint32_t vmaddr, uint32_t override_vmaddr);

    void patch_objc_sects_callback(const char*, size_t, uint32_t new_address, const std::vector<uint32_t>& override_addresses) const {
        BOOST_FOREACH(uint32_t vmaddr, override_addresses) {
            long actual_offset = this->from_new_vmaddr(vmaddr);
            fseek(_f, actual_offset, SEEK_SET);
            fwrite(&new_address, 4, 1, _f);
        }
    }

    void patch_objc_sects() const {
        _extra_text.foreach_entry(this, &DecachingFile::patch_objc_sects_callback);
        _extra_data.foreach_entry(this, &DecachingFile::patch_objc_sects_callback);

        this->patch_objc_sects_callback(NULL, 0, sizeof(method_t), _entsize12_patches);
        this->patch_objc_sects_callback(NULL, 0, 0, _nullify_patches);

        if (_imageinfo_address) {
            long actual_offset = this->from_new_vmaddr(_imageinfo_address);
            fseek(_f, actual_offset, SEEK_SET);
            fwrite(&_imageinfo_replacement, 4, 1, _f);
        }
    }

public:
    DecachingFile(const boost::filesystem::path& filename, const mach_header* header, const ProgramContext* context) :
        MachOFile(header, context), _imageinfo_address(0),
        _extra_text("__TEXT", "__objc_extratxt", 2, 0),
        _extra_data("__DATA", "__objc_extradat", 0, 2)
    {
        if (header->magic != 0xfeedface) {
            fprintf(stderr,
                "Error: Cannot dump '%s'. Only 32-bit little-endian single-file\n"
                "       Mach-O objects are supported.\n", filename.c_str());
            return;
        }
        memset(&_new_linkedit_offsets, 0, sizeof(_new_linkedit_offsets));

        this->open_file(filename);
        if (!_f)
            return;

        // phase 1
        BOOST_FOREACH(const segment_command* segcmd, _segments) {
            ExtraStringRepository* repo = this->repo_for_segname(segcmd->segname);
            if (repo)
                repo->set_section_vmaddr(segcmd->vmaddr + segcmd->vmsize);
        }
        BOOST_FOREACH(const segment_command* segcmd, _segments)
            this->prepare_objc_extrastr(segcmd);

        // phase 2
        BOOST_FOREACH(const segment_command* segcmd, _segments)
            this->write_segment_content(segcmd);

        // phase 3
        _linkedit_offset = static_cast<uint32_t>(ftell(_f));
        this->foreach_command(&DecachingFile::write_real_linkedit);
        _linkedit_size = static_cast<uint32_t>(ftell(_f)) - _linkedit_offset;

        // phase 4 & 5
        fseek(_f, offsetof(mach_header, sizeofcmds), SEEK_SET);
        uint32_t new_sizeofcmds = _header->sizeofcmds + (_extra_text.has_content() + _extra_data.has_content()) * sizeof(section);
        fwrite(&new_sizeofcmds, sizeof(new_sizeofcmds), 1, _f);
        fseek(_f, sizeof(*header), SEEK_SET);
        this->foreach_command(&DecachingFile::fix_file_offsets);

        // phase 6
        this->patch_objc_sects();
    }

    ~DecachingFile() {
        if (_f)
            fclose(_f);
    }

    bool is_open() const { return _f != NULL; }

};

class ProgramContext {
    const char* _folder;
    const char* _filename;
    DataFile* _f;
    bool _printmode;
    std::vector<boost::filesystem::path> _namefilters;
    boost::unordered_map<const mach_header*, boost::filesystem::path> _already_dumped;

    const dyld_cache_header* _header;
    const shared_file_mapping_np* _mapping;
    const dyld_cache_image_info* _images;
    std::vector<MachOFile> _macho_files;

public:
    ProgramContext(std::string path) :
        _folder("libraries"),
        _filename(path.c_str()),
        _f(NULL),
        _printmode(false)
    {
        return;
    }

private:
    bool check_magic() const {
        return !strncmp(_header->magic, "dyld_v1", 7);
    }

    const mach_header* mach_header_of_image(int i) const {
        return _macho_files[i].header();
    }

    off_t from_vmaddr(uint64_t vmaddr) const {
        for (uint32_t i = 0; i < _header->mappingCount; ++ i) {
            if (_mapping[i].sfm_address <= vmaddr && vmaddr < _mapping[i].sfm_address + _mapping[i].sfm_size)
                return vmaddr - _mapping[i].sfm_address + _mapping[i].sfm_file_offset;
        }
        return -1;
    }

    const char* peek_char_at_vmaddr(uint64_t vmaddr) const {
        off_t offset = this->from_vmaddr(vmaddr);
        if (offset >= 0) {
            return _f->peek_data_at<char>(offset);
        } else {
            return NULL;
        }
    }
    
    void process_export_trie_node(off_t start, off_t cur, off_t end, const std::string& prefix, uint32_t bias, boost::unordered_map<uint32_t, std::string>& exports) const {
    	if (cur < end) {
    		_f->seek(cur);
    		unsigned char term_size = static_cast<unsigned char>(_f->read_char());
    		if (term_size != 0) {
    			/*unsigned flags =*/ _f->read_uleb128<unsigned>();
    			unsigned addr = _f->read_uleb128<unsigned>() + bias;
    			exports.insert(std::make_pair(addr, prefix));
    		}
    		_f->seek(cur + term_size + 1);
    		unsigned char child_count = static_cast<unsigned char>(_f->read_char());
    		off_t last_pos;
    		for (unsigned char i = 0; i < child_count; ++ i) {
    			const char* suffix = _f->read_string();
    			unsigned offset = _f->read_uleb128<unsigned>();
    			last_pos = _f->tell();
    			this->process_export_trie_node(start, start + offset, end, prefix + suffix, bias, exports);
    			_f->seek(last_pos);
    		}
    	}
    }
    
public:
    void fill_export(off_t start, off_t end, uint32_t bias, boost::unordered_map<uint32_t, std::string>& exports) const {
        process_export_trie_node(start, start, end, "", bias, exports);
    }

    void close() {
        if (_f) {
            delete _f;
            _f = NULL;
        }
    }

    bool open() {
        _f = new DataFile(_filename);

        _header = _f->peek_data_at<dyld_cache_header>(0);
        if (!this->check_magic()) {
            close();
            return false;
        }

        _mapping = _f->peek_data_at<shared_file_mapping_np>(_header->mappingOffset);
        _images = _f->peek_data_at<dyld_cache_image_info>(_header->imagesOffset);
        return true;
    }
    
    uint32_t image_containing_address(uint32_t vmaddr, std::string* symname = NULL) const {
        uint32_t i = 0;
        BOOST_FOREACH(const MachOFile& mo, _macho_files) {
            if (mo.contains_address(vmaddr)) {
                if (symname)
                    *symname = mo.exported_symbol(vmaddr);
                return i;
            }
            ++ i;
        }
        return ~0u;
    }
    
    bool is_print_mode() const { return _printmode; }

    const char* path_of_image(uint32_t i) const {
        return _f->peek_data_at<char>(_images[i].pathFileOffset);
    }

    bool should_skip_image(uint32_t i) const {
        const char* path = this->path_of_image(i);
        if (_namefilters.empty())
            return false;
            
        boost::filesystem::path stem = remove_all_extensions(path);
        BOOST_FOREACH(const boost::filesystem::path& filt, _namefilters) {
            if (stem == filt)
                return false;
        }

        return true;
    }

    // Decache the file of the specified index. If the file is already decached
    //  under a different name, create a symbolic link to it.
    void save_complete_image(uint32_t image_index) {
        boost::filesystem::path filename (_folder);
        const char* path = this->path_of_image(image_index);
        filename /= path;

        const mach_header* header = this->mach_header_of_image(image_index);
        boost::unordered_map<const mach_header*, boost::filesystem::path>::const_iterator cit = _already_dumped.find(header);

        bool already_dumped = (cit != _already_dumped.end());
        printf("%3d/%d: %sing '%s'...\n", image_index, _header->imagesCount, already_dumped ? "Link" : "Dump", path);

        if (already_dumped) {
            boost::system::error_code ec;
            boost::filesystem::path src_path (path);
            boost::filesystem::path target_path (".");
            boost::filesystem::path::iterator it = src_path.begin();
            ++ it;
            ++ it;
            for (; it != src_path.end(); ++ it) {
                target_path /= "..";
            }
            target_path /= cit->second;

            boost::filesystem::remove(filename);
            boost::filesystem::create_directories(filename.parent_path());
            boost::filesystem::create_symlink(target_path, filename, ec);
            if (ec)
                fprintf(stderr, "**** Failed: %s\n", ec.message().c_str());

        } else {
            _already_dumped.insert(std::make_pair(header, path));
            DecachingFile df (filename, header, this);
            if (!df.is_open())
                perror("**** Failed");
        }
    }

    void save_image_with_path(std::string imgName, std::string path) {
        int image_index = -1;
        //find the index that has this name

        for(uint32_t i = 0; i < this->_header->imagesCount; ++i) {
            std::string  s = this->path_of_image(i);
            if( s == imgName ) {
                image_index = i;
                break;
            }
        }

        assert(image_index != -1);

        const mach_header   *hdr = this->mach_header_of_image(image_index);
        DecachingFile       df(path, hdr, this); 

        assert(df.is_open());

        return;
    }

    void read_all_images() {
        this->_macho_files.clear();
        for(uint32_t i = 0; i < this->_header->imagesCount; ++i) {
            uint32_t            vma = this->_images[i].address;
            uint32_t            a = this->from_vmaddr(vma);
            const mach_header   *mh = this->_f->peek_data_at<mach_header>(a);
            
            MachOFile   mof(mh, this, this->_images[i].address);

            this->_macho_files.push_back(mof);
        }
    }

    void save_all_images() {
        _macho_files.clear();
        for (uint32_t i = 0; i < _header->imagesCount; ++ i) {
            const mach_header* mh = _f->peek_data_at<mach_header>(this->from_vmaddr(_images[i].address));
            _macho_files.push_back(MachOFile(mh, this, _images[i].address));
        }
        
        for (uint32_t i = 0; i < _header->imagesCount; ++ i) {
            if (!this->should_skip_image(i)) {
                this->save_complete_image(i);
            }
        }
    }
    
    void print_info() const {
        printf(
            "magic = \"%-.16s\", dyldBaseAddress = 0x%llx\n"
            "\n"
            "Mappings (%d):\n"
            "  ---------address  ------------size  ----------offset  prot\n"
        , _header->magic, _header->dyldBaseAddress, _header->mappingCount);

        for (uint32_t i = 0; i < _header->mappingCount; ++ i) {
            printf("  %16llx  %16llx  %16llx  %x (<= %x)\n",
                _mapping[i].sfm_address, _mapping[i].sfm_size, _mapping[i].sfm_file_offset,
                _mapping[i].sfm_init_prot, _mapping[i].sfm_max_prot
            );
        }

        printf(
            "\n"
            "Images (%d):\n"
            "  ---------address  filename\n"
        , _header->imagesCount);

        for (uint32_t i = 0; i < _header->imagesCount; ++ i) {
            printf("  %16llx  %s\n", _images[i].address, this->path_of_image(i));
        }
    }

    std::list<std::string> list_of_images(void) const {
        std::list<std::string>  found;
        
        for( uint32_t i = 0; i < this->_header->imagesCount; ++i ) {
            found.push_back(std::string(this->path_of_image(i)));
        }

        return found;
    }

    ~ProgramContext() { close(); }

    friend class DecachingFile;
};

}
