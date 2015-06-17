#include "VEE.h"

extern "C" {
#include <libvex_basictypes.h>
#include <libvex_guest_offsets.h>
#include <libvex_emnote.h>
#include <libvex.h>
#include <libvex_guest_x86.h>
#include <libvex_guest_amd64.h>
#include <libvex_guest_arm.h>
}

using namespace std;

int vee_getmem(lua_State *l);
int vee_getreg(lua_State *l);
int vee_putreg(lua_State *l);
int vee_putmem(lua_State *l);
int vee_getexit(lua_State *l);
int vee_register(lua_State *l);
int vee_hascalls(lua_State *l);
int vee_nextblock(lua_State *l);
int vee_setregclass(lua_State *l);

static const luaL_Reg vee_meths[] = {
    {"getmem", vee_getmem},
    {"putmem", vee_putmem},
    {"getreg", vee_getreg},
    {"putreg", vee_putreg},
    {"getexit", vee_getexit},
    {"hascalls", vee_hascalls},
    {"register", vee_register},
    {"nextblock", vee_nextblock},
    {"setregclass", vee_setregclass},
    {NULL,NULL}
};

int vee_setregclass(lua_State *l) {
    list<regMapT>       regPerms;
    VexExecutionState   *ves = (VexExecutionState *)lua_touserdata(l, 1);
    int                 regWidth;
    RegisterClass       regClass;
    bool                specificValPresent;
    int                 specificRegister;
    uint64_t            value;
    
    //get the contents of the fields by reading from lua
    assert( lua_istable(l, 2) == 1 );

    lua_getfield(l, 2, "CLASS");
    assert( lua_isnil(l, -1) != 1 );
    regClass = (RegisterClass) lua_tointeger(l, -1);
    lua_pop(l, 1);

    lua_getfield(l, 2, "WIDTH");
    assert( lua_isnil(l, -1) != 1 );
    regWidth = lua_tointeger(l, -1);
    lua_pop(l, 1);

    lua_getfield(l, 2, "VALUE");
    assert( lua_isnil(l, -1) != 1 );
    value = lua_tointeger(l, -1);
    lua_pop(l, 1);
  
    lua_getfield(l, 2, "REG");
    if( lua_isnil(l, -1) == 1 ) {
        specificValPresent = false;
    } else {
        specificValPresent = true;
        specificRegister = lua_tointeger(l, -1);
    }
    lua_pop(l, 1);

    //if a specific register is not given, we are doing a range of registers
    if( specificValPresent ) { 
        //only set a specific register
        ConstantValue   v;
        regMapT         blank;

        //arrgh, specify the architecture
        TargetArch  ta = ves->getArch();
        switch(ta.ta) {
            case X86:
                blank.resize(offsetof(VexGuestX86State, padding1));
                break;
            
            case ARM:
                blank.resize(offsetof(VexGuestARMState, padding1));
                break;

            default:
                assert(!"NIY");
        }

        v.valueIsKnown = true;

        regPerms.push_back(blank);        
    } else {
        //generate the permutations of this register
        //IMPORTANT: ONLY consider the registers that are part of this 
        //blocks INPUT set. 
        switch(regClass) {
            case GenericRegister:
            {
                //switch on our architecture
                TargetArch a = ves->getArch();

                switch(a.ta) {
                    case X86:
                    {
                        //we know what the GPRs for this platform are
                        uint32_t    regOffs[] = {
                            OFFSET_x86_EAX, OFFSET_x86_EBX, OFFSET_x86_ECX,
                            OFFSET_x86_EDX, OFFSET_x86_ESI, OFFSET_x86_EDI,
                            OFFSET_x86_EBP };
                        for(uint32_t i = 0;
                            i < sizeof(regOffs)/sizeof(uint32_t);
                            i++)
                        {
                            uint32_t        reg = regOffs[i];
                            ConstantValue   v; 
                            regMapT         blank;

                            blank.resize(offsetof(VexGuestX86State, padding1));

                            v.valueIsKnown = true;
                            v.width = regWidth;
                            switch(v.width) {
                                case 8:
                                    v.U8 = value;
                                    break;
                                case 16:
                                    v.U16 = value;
                                    break;
                                case 32:
                                    v.U32 = value;
                                    break;
                                case 64:
                                    v.U64 = value; 
                                    break;
                            }

                            RegConstant rc = {reg, 32, v};
                            
                            VexExecutionState::setStateS(reg, rc, blank);

                            //add the regMapT to the list 
                            regPerms.push_back(blank);
                        }
                    }
                        break;
                    case AMD64:
                    {
                        //we know what the GPRs for this platform are
                        uint32_t    regOffs[] = {
                            OFFSET_amd64_RAX, OFFSET_amd64_RBX, OFFSET_amd64_RCX,
                            OFFSET_amd64_RDX, OFFSET_amd64_RSI, OFFSET_amd64_RDI,
							OFFSET_amd64_R8,
							OFFSET_amd64_R9,
							OFFSET_amd64_R10,
							OFFSET_amd64_R11,
							OFFSET_amd64_R12,
							OFFSET_amd64_R13,
							OFFSET_amd64_R14,
							OFFSET_amd64_R15,
                            OFFSET_amd64_RBP };
                        for(uint32_t i = 0;
                            i < sizeof(regOffs)/sizeof(uint32_t);
                            i++)
                        {
                            uint32_t        reg = regOffs[i];
                            ConstantValue   v; 
                            regMapT         blank;

                            blank.resize(offsetof(VexGuestAMD64State, pad1));

                            v.valueIsKnown = true;
                            v.width = regWidth;
                            switch(v.width) {
                                case 8:
                                    v.U8 = value;
                                    break;
                                case 16:
                                    v.U16 = value;
                                    break;
                                case 32:
                                    v.U32 = value;
                                    break;
                                case 64:
                                    v.U64 = value; 
                                    break;
                            }

                            RegConstant rc = {reg, 64, v};
                            
                            VexExecutionState::setStateS(reg, rc, blank);

                            //add the regMapT to the list 
                            regPerms.push_back(blank);
                        }
                    }
						break;
                    case ARM:
                    {
                        //we know what the GPRs for this platform are
                        uint32_t    regOffs[] = {
                            OFFSET_arm_R0, OFFSET_arm_R1, OFFSET_arm_R2,
                            OFFSET_arm_R3, OFFSET_arm_R4, OFFSET_arm_R5,
                            OFFSET_arm_R5+4, OFFSET_arm_R7, OFFSET_arm_R7+4,
                            OFFSET_arm_R7+8, OFFSET_arm_R7+12, OFFSET_arm_R7+16,
                            OFFSET_arm_R7+16, OFFSET_arm_R7+20, OFFSET_arm_R13,
                            OFFSET_arm_R14 };
                        for(uint32_t i = 0;
                            i < sizeof(regOffs)/sizeof(uint32_t);
                            i++)
                        {
                            uint32_t        reg = regOffs[i];
                            ConstantValue   v; 
                            regMapT         blank;

                            blank.resize(offsetof(VexGuestARMState, padding1));

                            v.valueIsKnown = true;
                            v.width = regWidth;
                            switch(v.width) {
                                case 8:
                                    v.U8 = value;
                                    break;
                                case 16:
                                    v.U16 = value;
                                    break;
                                case 32:
                                    v.U32 = value;
                                    break;
                                case 64:
                                    v.U64 = value; 
                                    break;
                            }

                            RegConstant rc = {reg, 32, v};
                            
                            VexExecutionState::setStateS(reg, rc, blank);

                            //add the regMapT to the list 
                            regPerms.push_back(blank);
                        }
                    }
                        break;
                    default:
                        assert(!"NIY");
                }
            }
                break;
            case Flags:
                //not currently supported
                break;
            case ProgramCounter:
            {
            }
                break;
            case StackPointer:
            {
                TargetArch a = ves->getArch();
                switch(a.ta) {
                    case X86:
                    {
                        ConstantValue   v;
                        regMapT         blank; 

                        v.valueIsKnown = true;
                        v.width = 32;
                        v.U32 = value;

                        RegConstant rc = {OFFSET_x86_ESP, 32, v};

                        VexExecutionState::setStateS(OFFSET_x86_ESP, rc, blank);

                        regPerms.push_back(blank);
                    }
                        break;
                    default:
                        assert(!"NIY");
                }
            }
                break;
            case InternalState:
                break;
        }
    }
    
    //add our generated set to the state
    ves->addRegPermutations(regPerms);

    return 0;
}

int vee_nextblock(lua_State *l) {
    lua_pushnil(l);
    return 1;
}

int vee_hascalls(lua_State *l) {
    bool hasCall=false;

    VexExecutionState *vss = (VexExecutionState *)lua_touserdata(l, 1);
    lua_pushboolean(l, vss->getDidCall());

    return 1;
}

int vee_register(lua_State *l) {
    lua_getglobal(l, "__state"); 
    ScriptState *ss = (ScriptState *) lua_touserdata(l, -1);
    lua_pop(l, 1);
    assert(ss != NULL && ss->postCallbackIdx == 0 && ss->preCallbackIdx == 0);
    ss->postCallbackIdx = luaL_ref(l, LUA_REGISTRYINDEX);
    ss->preCallbackIdx = luaL_ref(l, LUA_REGISTRYINDEX);
    return 0;
}

int vee_getexit(lua_State *l) {
    VexExecutionState *vss;

    vss = (VexExecutionState *) lua_touserdata(l, 1);

    lua_pushinteger(l, vss->getVEEExit());

    return 1;
}

int vee_putreg(lua_State *l) {
    //get offset, width, and value
    int offset;
    int width;
    int v;
    VexExecutionState *vss;

    offset = luaL_checkinteger(l, 2);
    width = luaL_checkinteger(l, 3);
    v = luaL_checkinteger(l, 4);
    vss = (VexExecutionState *) lua_touserdata(l, 1);
	
	vss->setState(offset, width, v);

    return 0;
}

//either return a number or Nil
int vee_getreg(lua_State *l) {
    int offset;
    int width;
    VexExecutionState *vss;
    //get offset and width
    offset = luaL_checkinteger(l, 2);
    width = luaL_checkinteger(l, 3);
    vss = (VexExecutionState *)lua_touserdata(l, 1);


	ConstantValue cv = vss->getConstFromState(offset, width);
	if( cv.valueIsKnown ) {
		int k =0;
		switch(cv.width) {
			case 1:
				k = cv.U1;
				break;
			case 8:
				k = cv.U8;
				break;
			case 16:
				k = cv.U16;
				break;
			case 32:
				k = cv.U32;
				break;
			case 64:
				k = cv.U64;
				break;
		}
        //printf("register: 0x%x\n", k);
		lua_pushinteger(l, k);
	} else {
		lua_pushnil(l);
	}

    return 1;
}

int vee_putmem(lua_State *l) {
    //get offset, width, and value
    unsigned int offset;
    unsigned int width;
    unsigned int v;
    VexExecutionState *vss;

    offset = luaL_checkinteger(l, 2);
    width = luaL_checkinteger(l, 3);
    v = luaL_checkinteger(l, 4);
    vss = (VexExecutionState *) lua_touserdata(l, 1);

	vss->setMem(offset, width, v);

    return 0;
}


//either return a number or Nil
int vee_getmem(lua_State *l) {
    // get offset and width
    // TODO: offset should probably be 64bit!
    int offset;
    int width;
    VexExecutionState *vss;
    
    //get offset and width
    offset = luaL_checkinteger(l, 2);
    width = luaL_checkinteger(l, 3);
    vss = (VexExecutionState *)lua_touserdata(l, 1);
	ConstantValue cv = vss->getConstFromMem(offset, width);
    
    if( cv.valueIsKnown ) {
		int k =0;
		switch(cv.width) {
			case 1:
				k = cv.U1;
				break;
			case 8:
				k = cv.U8;
				break;
			case 16:
				k = cv.U16;
				break;
			case 32:
				k = cv.U32;
				break;
		}
		lua_pushinteger(l, k);
	} else {
		lua_pushnil(l);
	}

    return 1;
}

static void veeRMap(lua_State *l, const char *name, unsigned long off) {
    lua_pushnumber(l, off);
    lua_setglobal(l, name);
}

void initVeeEnv(lua_State *l) {
    //set up the stuff for the environment / class table stuff
    lua_pushlstring(l, "CLASS", strlen("CLASS"));
    lua_setglobal(l, "CLASS");
    lua_pushlstring(l, "WIDTH", strlen("WIDTH"));
    lua_setglobal(l, "WIDTH");
    lua_pushlstring(l, "VALUE", strlen("VALUE"));
    lua_setglobal(l, "VALUE");
    lua_pushlstring(l, "REG", strlen("REG"));
    lua_setglobal(l, "REG");
    lua_pushnumber(l, GenericRegister);
    lua_setglobal(l, "GenericRegister");
    lua_pushnumber(l, StackPointer);
    lua_setglobal(l, "StackPointer");
    lua_pushnumber(l, ProgramCounter);
    lua_setglobal(l, "ProgramCounter");
    //set up register name mappings
    veeRMap(l, "EAX", OFFSET_x86_EAX);
    veeRMap(l, "EBX", OFFSET_x86_EBX);
    veeRMap(l, "ECX", OFFSET_x86_ECX);
    veeRMap(l, "EDX", OFFSET_x86_EDX);
    veeRMap(l, "ESI", OFFSET_x86_ESI);
    veeRMap(l, "EDI", OFFSET_x86_EDI);
    veeRMap(l, "EBP", OFFSET_x86_EBP);
    veeRMap(l, "ESP", OFFSET_x86_ESP);
    veeRMap(l, "EIP", OFFSET_x86_EIP);
    //define register mappings for ARM
    veeRMap(l, "R0", OFFSET_arm_R0);
    veeRMap(l, "R1", OFFSET_arm_R1);
    veeRMap(l, "R2", OFFSET_arm_R2);
    veeRMap(l, "R3", OFFSET_arm_R3);
    veeRMap(l, "R4", OFFSET_arm_R4);
    veeRMap(l, "R5", OFFSET_arm_R5);
    veeRMap(l, "R6", OFFSET_arm_R5+4);
    veeRMap(l, "R7", OFFSET_arm_R7);
    veeRMap(l, "R8", OFFSET_arm_R7+4);
    veeRMap(l, "R9", OFFSET_arm_R7+8);
    veeRMap(l, "R10", OFFSET_arm_R7+12);
    veeRMap(l, "R11", OFFSET_arm_R7+16);
    veeRMap(l, "R12", OFFSET_arm_R7+20);
    veeRMap(l, "R13", OFFSET_arm_R13);
    veeRMap(l, "R14", OFFSET_arm_R14);
    veeRMap(l, "R15", OFFSET_arm_R15T);
    //register mappings for AMD64
    veeRMap(l, "RAX", OFFSET_amd64_RAX);
    veeRMap(l, "RBX", OFFSET_amd64_RBX);
    veeRMap(l, "RCX", OFFSET_amd64_RCX);
    veeRMap(l, "RDX", OFFSET_amd64_RDX);
    veeRMap(l, "RSI", OFFSET_amd64_RSI);
    veeRMap(l, "RDI", OFFSET_amd64_RDI);
    veeRMap(l, "RSP", OFFSET_amd64_RSP);
    veeRMap(l, "RBP", OFFSET_amd64_RBP);
    veeRMap(l, "R8", OFFSET_amd64_R8);
    veeRMap(l, "R9", OFFSET_amd64_R9);
    veeRMap(l, "R10", OFFSET_amd64_R10);
    veeRMap(l, "R11", OFFSET_amd64_R11);
    veeRMap(l, "R12", OFFSET_amd64_R12);
    veeRMap(l, "R13", OFFSET_amd64_R13);
    veeRMap(l, "R14", OFFSET_amd64_R14);
    veeRMap(l, "R15", OFFSET_amd64_R15);
    veeRMap(l, "RIP", OFFSET_amd64_RIP);
    //set up exit type constants
    lua_pushnumber(l, Fallthrough);
    lua_setglobal(l, "Fallthrough");
    lua_pushnumber(l, Call);
    lua_setglobal(l, "Call");
    lua_pushnumber(l, Return);
    lua_setglobal(l, "Return");
    lua_pushnumber(l, UnknownExit);
    lua_setglobal(l, "UnknownExit");
    return;
}

ScriptState *initScript(string scriptPath) {
    ScriptState *ss = (ScriptState *)malloc(sizeof(ScriptState));
    assert(ss != NULL);
    memset(ss, 0, sizeof(ScriptState));
    lua_State *s = luaL_newstate();
    luaL_openlibs(s);
    lua_newtable(s);
    luaL_setfuncs(s, vee_meths, 0);
    lua_pushvalue(s, -1);
    lua_setglobal(s, "vee");
    lua_pushlightuserdata(s, ss);
    lua_setglobal(s, "__state");
    initVeeEnv(s);
    
    ss->S = s;
    int k = luaL_dofile(s, scriptPath.c_str());
    if( k != 0 ) {
        printf("Error occured when calling luaL_dofile() Hint Machine 0x%x\n",k);
        printf("Error: %s", lua_tostring(s,-1));
        lua_close(s);
		s = NULL;
        free(ss);
        ss = NULL;
	}

    return ss;
}

ScriptState *initScriptFromString(string script) {
    ScriptState *ss = (ScriptState *)malloc(sizeof(ScriptState));
    assert(ss != NULL);
    memset(ss, 0, sizeof(ScriptState));
    lua_State *s = luaL_newstate();
    luaL_openlibs(s);
    lua_newtable(s);
    luaL_setfuncs(s, vee_meths, 0);
    lua_pushvalue(s, -1);
    lua_setglobal(s, "vee");
    lua_pushlightuserdata(s, ss);
    lua_setglobal(s, "__state");
    initVeeEnv(s);
    
    ss->S = s;
    int k = luaL_dostring(s, script.c_str());
	if( k != 0 ) {
		lua_close(s);
		s = NULL;
        free(ss);
        ss = NULL;
	}

    return ss;

}

void runPre(ScriptState *ss, VexExecutionStatePtr vss) {
    lua_rawgeti(ss->S, LUA_REGISTRYINDEX, ss->preCallbackIdx);
    lua_pushlightuserdata(ss->S, vss.get());
    lua_call(ss->S, 1, 0);

    return;
}

void runChase(ScriptState *ss, VexExecutionStatePtr vss, bool &res) {
    /*
    lua_getfield(ss->S, LUA_GLOBALSINDEX, "onChase");
    lua_pushlightuserdata(ss->S, vee);
    lua_call(ss->S, 1, 1);

    //get the return value, should be boolean
    int ires = lua_toboolean(ss->S, -1);
    lua_pop(ss->S, 1);
    if( ires == 0 ) {
        res = false;
    } else if( ires == 1 ) {
        res = true;
    }*/

    return;
}

void runPost(ScriptState *ss, VexExecutionStatePtr vss, bool &res) {
    lua_rawgeti(ss->S, LUA_REGISTRYINDEX, ss->postCallbackIdx);
    lua_pushlightuserdata(ss->S, vss.get());
    lua_call(ss->S, 1, 1);

    //get the return value, should be boolean
    int ires = lua_toboolean(ss->S, -1);
    lua_pop(ss->S, 1);
    if( ires == 0 ) {
        res = false;
    } else if( ires == 1 ) {
        res = true;
    }

    return;
}

void finiScript(ScriptState *ss) {
    lua_close(ss->S);
    free(ss);
    return;
}
