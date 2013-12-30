/* vim:set ts=4 sw=4:
 *
 * (C) 2008 Willem Hengeveld  itsme@xs4all.nl
 *
 * IdcPerl - a perl scripting plugin for the Interactive Disassembler by hex-rays.
 * see http://www.xs4all.nl/~itsme/projects/idcperl
 *
 */

#include <string>

// ida includes
#include <pro.h>     // for basic types.
#include <ida.hpp>   // for ida constants and types.
#include <kernwin.hpp>  // for msg
#include <expr.hpp>  // for set_idc_func_ex, execute

// perl includes
#include "extern.h"
#include "perl.h"
#include "xsub.h"
#undef do_open
#undef do_close

#ifdef DYNAMIC_PERL
#include "redefperlasdll.h"
#include "perldllprocs.h"
#endif

#include "idchotkey.h"

#ifdef TRACE_HOTKEY
#define tracemsg msg
#else
#define tracemsg(...) 
#endif

// =============================================================
//    thunk code, for calling perl function with 'AddHotkey'
// =============================================================

typedef error_t (idaapi *idcextensionfunc_t)(idc_value_t *argv,idc_value_t *res);

// this MUST be a char array, later the address to this array is replaced
// with a ptr to the thunked name.
static char dummyperlname[1]={0};
static error_t idaapi idcperlthunk(value_t *argv,value_t *res)
{
    // todo: do i need to set the correct interpreter?
    //     ... i could add a 'dummy global ptr'  and replace it with
    //     the correct interpreter when patching up this chunk of code.
    //     then i would also need to verify that the interpreter is still active.
    char *args[]= { NULL };
    call_argv(dummyperlname, (long)G_DISCARD, args);
    return eOk;
}
static void idcperlthunkend()
{
    // empty function, used to calculate the size of 'idcperlthunk'
}
#define THUNKSIZE ((char*)idcperlthunkend-(char*)idcperlthunk)

bool isvalidaddr(uint32 addr)
{
	return (addr<0x20000000);
}

bool createthunk(uchar *thunkcode, const std::string&func)
{
    uchar* origthunk= reinterpret_cast<uchar*>(&idcperlthunk);
    memcpy(thunkcode, origthunk, THUNKSIZE);
    qstrncpy((char*)thunkcode+THUNKSIZE, func.c_str(), func.size()+1);
    int patchcount=0;
	uint32 origpic_ebx= 0;	// 'ebx' in original thunk code
	uint32 newpic_ebx= 0;		// 'ebx' in copy of thunk code
    for (size_t i=0 ; i<THUNKSIZE-sizeof(uint32) ; i++)
    {
		// check for absolute reference
        if (*(uint32*)(origthunk+i)==(uint32)&dummyperlname) {
            *(uint32*)(thunkcode+i)= (uint32)(thunkcode+THUNKSIZE);
            patchcount++;
        }
		// check for call to 'mov ebx, [esp]
		else if (origthunk[i]==0xe8) {
			uint32 calladdr= *(uint32*)(origthunk+i+1)+(uint32)origthunk+i+5;
			if (isvalidaddr(calladdr) && *(uint32*)calladdr == 0xc3241c8b) {
				origpic_ebx= reinterpret_cast<uint32>(origthunk+i+5);
				newpic_ebx= reinterpret_cast<uint32>(thunkcode+i+5);
			}
		}
		else if (origpic_ebx 
				&& *(uint16*)(origthunk+i) == 0x938d
				&& *(uint32*)(origthunk+i+2)+origpic_ebx == (uint32)&dummyperlname) {
            *(uint32*)(thunkcode+i+2)= (uint32)thunkcode+THUNKSIZE-newpic_ebx;
            patchcount++;
		}
    }
    return patchcount!=0;
}

hotkey::hotkey() : _thunkcode(NULL) { }
hotkey::~hotkey() { del(); }

bool hotkey::add(const std::string&key, const std::string&func)
{
    _thunkcode= new uchar[func.size()+1+THUNKSIZE];
    if (!createthunk(_thunkcode, func)) {
        msg("error creating thunkcode for %s\n", func.c_str());
        return false;
    }
    std::string hexaddr; hexaddr.resize(9);
    qsnprintf(&hexaddr[0], hexaddr.size(), "%p", _thunkcode);
    hexaddr.resize(8);

    std::string perlthunk= std::string("perlthunk_")+hexaddr;
    char idcvoidargs=0;
    if (!set_idc_func_ex(perlthunk.c_str(), (idc_func_t*)_thunkcode, &idcvoidargs, 0)) {
        msg("error setting idc func %s\n", perlthunk.c_str());
        return false;
    }
    _perlthunk= perlthunk;

    std::string idcthunk= std::string("idcthunk_")+hexaddr;
    std::string idcscript= std::string("} static ")+idcthunk+"() { "+perlthunk+"();";
    if (!execute(idcscript.c_str())) {
        msg("error creating %s\n%s\n", idcthunk.c_str(), idcscript.c_str());
        return false;
    }
    _idcthunk= idcthunk;

    int rc= add_idc_hotkey(key.c_str(), idcthunk.c_str());
    if (rc) {
        msg("error %d adding hotkey %s\n", rc, key.c_str());
        return false;
    }
    _hotkey= key;

    tracemsg("created hotkey %p - %s\n", _thunkcode, _hotkey.c_str());
    return true;
}
void hotkey::del()
{
     tracemsg("deleting hotkey %p - %s : %s : %s\n", _thunkcode, _hotkey.c_str(), _idcthunk.c_str(), _perlthunk.c_str());
    // remove hotkey
    if (!_hotkey.empty()) {
        del_idc_hotkey(_hotkey.c_str());
        _hotkey.clear();
    }
    // remove 'idcthunk_%08lx'
    if (!_idcthunk.empty()) {
        set_idc_func_body(_idcthunk.c_str(), 0, NULL, 0);
        _idcthunk.clear();
    }
    // remove 'perlthunk_%08lx'
    if (!_perlthunk.empty()) {
        set_idc_func_ex(_perlthunk.c_str(), NULL, NULL, 0);
        _perlthunk.clear();
    }
    // delete thunkcode
    if (_thunkcode) {
        delete _thunkcode;
        _thunkcode= NULL;
    }
}

