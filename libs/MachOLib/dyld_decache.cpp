#include "dyld_decache.h"

using namespace dyld_decache;

bool streq(const char x[16], const char* y) {
    return strncmp(x, y, 16) == 0;
}

long write_uleb128(FILE* f, unsigned u) {
    uint8_t buf[16];
    int byte_count = 0;
    while (u) {
        buf[byte_count++] = u | 0x80;
        u >>= 7;
    }
    buf[byte_count-1] &= ~0x80;
    fwrite(buf, byte_count, sizeof(*buf), f);
    return byte_count;
}

boost::filesystem::path remove_all_extensions(const char* the_path) {
    boost::filesystem::path retval (the_path);
    do {
        retval = retval.stem();
    } while (!retval.extension().empty());
    return retval;
}

void MachOFile::retrieve_segments_and_libords(const load_command* cmd) {
    switch (cmd->cmd) {
        default:
            break;
        case LC_SEGMENT: {
            const segment_command* segcmd = static_cast<const segment_command*>(cmd);
            _segments.push_back(segcmd);
            break;
        }
        case LC_LOAD_DYLIB:
        case LC_ID_DYLIB:
        case LC_LOAD_WEAK_DYLIB:
        case LC_REEXPORT_DYLIB:
        case LC_LAZY_LOAD_DYLIB:
        case LC_LOAD_UPWARD_DYLIB: {
            const dylib_command* dlcmd = static_cast<const dylib_command*>(cmd);
            std::string dlname (dlcmd->dylib.name + reinterpret_cast<const char*>(dlcmd));
            _libords.insert(std::make_pair(dlname, _cur_libord));
            ++ _cur_libord;
            break;
        }
        
        case LC_DYLD_INFO:
        case LC_DYLD_INFO_ONLY: {
            if (_image_vmaddr) {
                const dyld_info_command* dicmd = static_cast<const dyld_info_command*>(cmd);
                if (dicmd->export_off)
                    _context->fill_export(dicmd->export_off, dicmd->export_off + dicmd->export_size, _image_vmaddr, _exports);
            }
            break;
        }
    }
}

void DecachingFile::write_segment_content(const segment_command* segcmd) {
    if (!streq(segcmd->segname, "__LINKEDIT")) {
        ExtraStringRepository* repo = this->repo_for_segname(segcmd->segname);

        const char* data_ptr = _context->peek_char_at_vmaddr(segcmd->vmaddr);
        long new_fileoff = ftell(_f);

        fwrite(data_ptr, 1, segcmd->filesize, _f);
        uint32_t filesize = segcmd->filesize;

        if (repo && repo->has_content()) {
            repo->foreach_entry(this, &DecachingFile::write_extrastr);

            // make sure the section is aligned on 8-byte boundary...
            long extra = ftell(_f) % 8;
            if (extra) {
                char padding[8] = {0};
                fwrite(padding, 1, 8-extra, _f);
                repo->increase_size_by(8-extra);
            }
            repo->set_section_fileoff(new_fileoff + filesize);
            filesize += repo->total_size();
        }

        FileoffFixup fixup = {segcmd->fileoff, segcmd->fileoff + filesize, segcmd->fileoff - new_fileoff};
        _fixups.push_back(fixup);
    }
}

void DecachingFile::write_real_linkedit(const load_command* cmd) {
    const unsigned char* data_ptr = _context->_f->data();

    // Write all data in [offmem .. offmem+countmem*objsize] to the output file,
    //  and pad to make sure the beginning is aligned with 'objsize' boundary.
    #define TRY_WRITE(offmem, countmem, objsize) \
        if (cmdvar->offmem && cmdvar->countmem) { \
            long curloc = ftell(_f); \
            long extra = curloc % objsize; \
            if (extra != 0) { \
                char padding[objsize] = {0}; \
                fwrite(padding, 1, objsize-extra, _f); \
                curloc += objsize-extra; \
            } \
            _new_linkedit_offsets.offmem = curloc; \
            fwrite(cmdvar->offmem + data_ptr, cmdvar->countmem * objsize, 1, _f); \
        }

    switch (cmd->cmd) {
        default:
            break;

        case LC_DYLD_INFO:
        case LC_DYLD_INFO_ONLY: {
            const dyld_info_command* cmdvar = static_cast<const dyld_info_command*>(cmd);
            TRY_WRITE(rebase_off, rebase_size, 1);
            long curloc = ftell(_f);
            long extra_size = _extra_bind.optimize_and_write(_f);
            TRY_WRITE(bind_off, bind_size, 1);
            _new_linkedit_offsets.bind_off = curloc;
            _new_linkedit_offsets.bind_size += extra_size;
            TRY_WRITE(weak_bind_off, weak_bind_size, 1);
            TRY_WRITE(lazy_bind_off, lazy_bind_size, 1);
            TRY_WRITE(export_off, export_size, 1);
            break;
        }

        case LC_SYMTAB: {
            // The string table is shared by all library, so naively using
            //  TRY_WRITE will create a huge file with lots of unnecessary
            //  strings. Therefore, we have to scan through all symbols and only
            //  take those strings which are used by the symbol.
            const symtab_command* cmdvar = static_cast<const symtab_command*>(cmd);
            if (cmdvar->symoff && cmdvar->nsyms) {
                _new_linkedit_offsets.stroff = ftell(_f);

                nlist* syms = new nlist[cmdvar->nsyms];
                memcpy(syms, _context->_f->peek_data_at<nlist>(cmdvar->symoff), sizeof(*syms) * cmdvar->nsyms);

                int32_t cur_strx = 0;
                for (uint32_t i = 0; i < cmdvar->nsyms; ++ i) {
                    const char* the_string = _context->_f->peek_data_at<char>(syms[i].n_strx + cmdvar->stroff);
                    size_t entry_len = strlen(the_string) + 1;
                    fwrite(the_string, entry_len, 1, _f);
                    syms[i].n_strx = cur_strx;
                    cur_strx += entry_len;
                }
                _new_linkedit_offsets.strsize = cur_strx;

                long curloc = ftell(_f);
                long extra = curloc % sizeof(nlist);
                if (extra != 0) {
                    char padding[sizeof(nlist)] = {0};
                    fwrite(padding, 1, sizeof(nlist)-extra, _f);
                    curloc += sizeof(nlist)-extra;
                }
                _new_linkedit_offsets.symoff = curloc;
                fwrite(syms, cmdvar->nsyms, sizeof(nlist), _f);

                delete[] syms;
            }

            break;
        }

        case LC_DYSYMTAB: {
            const dysymtab_command* cmdvar = static_cast<const dysymtab_command*>(cmd);
            TRY_WRITE(tocoff, ntoc, 8);
            TRY_WRITE(modtaboff, nmodtab, 52);
            TRY_WRITE(extrefsymoff, nextrefsyms, 4);
            TRY_WRITE(indirectsymoff, nindirectsyms, 4);
            TRY_WRITE(extreloff, nextrel, 8);
            TRY_WRITE(locreloff, nlocrel, 8);
            break;
        }

        case LC_CODE_SIGNATURE:
        case LC_SEGMENT_SPLIT_INFO:
        case LC_FUNCTION_STARTS: {
            const linkedit_data_command* cmdvar = static_cast<const linkedit_data_command*>(cmd);
            TRY_WRITE(dataoff, datasize, 1);
            if (cmd->cmd == LC_CODE_SIGNATURE)
                _new_linkedit_offsets.dataoff_cs = _new_linkedit_offsets.dataoff;
            else if (cmd->cmd == LC_SEGMENT_SPLIT_INFO)
                _new_linkedit_offsets.dataoff_ssi = _new_linkedit_offsets.dataoff;
            else if (cmd->cmd == LC_FUNCTION_STARTS)
                _new_linkedit_offsets.dataoff_fs = _new_linkedit_offsets.dataoff;
            break;
        }
    }

    #undef TRY_WRITE
}

void DecachingFile::get_address_info(uint32_t vmaddr, std::string* p_name, int* p_libord) const {
    uint32_t which_image = _context->image_containing_address(vmaddr, p_name);
    const char* image_name = _context->path_of_image(which_image);
    *p_libord = this->libord_with_name(image_name);
}

void DecachingFile::add_extlink_to(uint32_t vmaddr, uint32_t override_vmaddr) {
    if (!vmaddr)
        return;
    if (this->contains_address(vmaddr))
        return;
    _extra_bind.insert(vmaddr, this->segnum_and_offset(override_vmaddr), this, &DecachingFile::get_address_info);
    // get class-dump-z to search for symbols instead of using this invalid
    //  address directly.
    _nullify_patches.push_back(override_vmaddr);
}

void DecachingFile::prepare_patch_objc_methods(uint32_t method_vmaddr, uint32_t override_vmaddr) {
    if (!method_vmaddr)
        return;

    off_t method_offset = _context->from_vmaddr(method_vmaddr);
    _context->_f->seek(method_offset);
    bool wrong_entsize = _context->_f->copy_data<uint32_t>() != sizeof(method_t);
    uint32_t count = _context->_f->copy_data<uint32_t>();

    if (!this->contains_address(method_vmaddr)) {
        method_vmaddr = _extra_data.next_vmaddr();
        size_t size = 8 + sizeof(method_t)*count;
        _extra_data.insert(_context->_f->peek_data_at<char>(method_offset), size, override_vmaddr);
    }

    // add the patch to make sure the method_t's entsize is 12.
    //  (this causes class-dump-3.3.3 to raise an exception)
    if (wrong_entsize)
        _entsize12_patches.push_back(method_vmaddr);

    const method_t* methods = _context->_f->peek_data<method_t>();
    for (uint32_t j = 0; j < count; ++ j) {
        if (!this->contains_address(methods[j].name)) {
            const char* the_string = _context->peek_char_at_vmaddr(methods[j].name);
            _extra_text.insert(the_string, method_vmaddr + 8 + sizeof(method_t)*j);
        }
    }
}

void DecachingFile::prepare_objc_extrastr(const segment_command* segcmd) {
    if (streq(segcmd->segname, "__DATA")) {
        const section* sects = reinterpret_cast<const section*>(1 + segcmd);
        for (uint32_t i = 0; i < segcmd->nsects; ++ i) {
            const section& sect = sects[i];
            if (streq(sect.sectname, "__objc_selrefs")) {
                const uint32_t* refs = _context->_f->peek_data_at<uint32_t>(sect.offset);
                for (uint32_t j = 0; j < sect.size/4; ++ j) {
                    if (!this->contains_address(refs[j])) {
                        const char* the_string = _context->peek_char_at_vmaddr(refs[j]);
                        _extra_text.insert(the_string, sect.addr + 4*j);
                    }
                }
            } else if (streq(sect.sectname, "__objc_classlist")) {
                const uint32_t* classes = _context->_f->peek_data_at<uint32_t>(sect.offset);
                for (uint32_t j = 0; j < sect.size/4; ++ j) {
                    uint32_t class_vmaddr = classes[j];
                    const class_t* class_obj = reinterpret_cast<const class_t*>(_context->peek_char_at_vmaddr(class_vmaddr));
                    this->add_extlink_to(class_obj->superclass, class_vmaddr + offsetof(class_t, superclass));
                    const class_ro_t* class_data = reinterpret_cast<const class_ro_t*>(_context->peek_char_at_vmaddr(class_obj->data));
                    this->prepare_patch_objc_methods(class_data->baseMethods, class_obj->data + offsetof(class_ro_t, baseMethods));
                    
                    const class_t* metaclass_obj = reinterpret_cast<const class_t*>(_context->peek_char_at_vmaddr(class_obj->isa));
                    this->add_extlink_to(metaclass_obj->isa, class_obj->isa + offsetof(class_t, isa));
                    this->add_extlink_to(metaclass_obj->superclass, class_obj->isa + offsetof(class_t, superclass));
                    const class_ro_t* metaclass_data = reinterpret_cast<const class_ro_t*>(_context->peek_char_at_vmaddr(metaclass_obj->data));
                    this->prepare_patch_objc_methods(metaclass_data->baseMethods, metaclass_obj->data + offsetof(class_ro_t, baseMethods));
                }
            } else if (streq(sect.sectname, "__objc_protolist")) {
                const uint32_t* protos = _context->_f->peek_data_at<uint32_t>(sect.offset);
                for (uint32_t j = 0; j < sect.size/4; ++ j) {
                    uint32_t proto_vmaddr = protos[j];
                    const protocol_t* proto_obj = reinterpret_cast<const protocol_t*>(_context->peek_char_at_vmaddr(proto_vmaddr));
                    this->prepare_patch_objc_methods(proto_obj->instanceMethods, proto_vmaddr + offsetof(protocol_t, instanceMethods));
                    this->prepare_patch_objc_methods(proto_obj->classMethods, proto_vmaddr + offsetof(protocol_t, classMethods));
                    this->prepare_patch_objc_methods(proto_obj->optionalInstanceMethods, proto_vmaddr + offsetof(protocol_t, optionalInstanceMethods));
                    this->prepare_patch_objc_methods(proto_obj->optionalClassMethods, proto_vmaddr + offsetof(protocol_t, optionalClassMethods));
                }
            } else if (streq(sect.sectname, "__objc_catlist")) {
                const uint32_t* cats = _context->_f->peek_data_at<uint32_t>(sect.offset);
                for (uint32_t j = 0; j < sect.size/4; ++ j) {
                    uint32_t cat_vmaddr = cats[j];
                    const category_t* cat_obj = reinterpret_cast<const category_t*>(_context->peek_char_at_vmaddr(cat_vmaddr));
                    this->add_extlink_to(cat_obj->cls, cat_vmaddr + offsetof(category_t, cls));
                    this->prepare_patch_objc_methods(cat_obj->instanceMethods, cat_vmaddr + offsetof(category_t, instanceMethods));
                    this->prepare_patch_objc_methods(cat_obj->classMethods, cat_vmaddr + offsetof(category_t, classMethods));
                }
            } else if (streq(sect.sectname, "__objc_imageinfo")) {
                _imageinfo_address = sect.addr + 4;
                uint32_t original_flag = *reinterpret_cast<const uint32_t*>(_context->peek_char_at_vmaddr(_imageinfo_address));
                _imageinfo_replacement = original_flag & ~8;    // clear the OBJC_IMAGE_OPTIMIZED_BY_DYLD flag. (this chokes class-dump-3.3.3.)
            } else if (streq(sect.sectname, "__objc_classrefs")) {
                const uint32_t* refs = _context->_f->peek_data_at<uint32_t>(sect.offset);
                uint32_t addr = sect.addr;
                for (uint32_t j = 0; j < sect.size/4; ++ j, ++ refs, addr += 4) {
                    this->add_extlink_to(*refs, addr);
                }
            }
        }
    }
}
