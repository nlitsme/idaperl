/* vim:set ts=4 sw=4 ai si:
 *
 * (C) 2008 Willem Hengeveld  itsme@xs4all.nl
 *
 * IdcPerl - a perl scripting plugin for the Interactive Disassembler by hex-rays.
 * see http://www.xs4all.nl/~itsme/projects/idcperl
 *
 * this file contains perl XSUBS calling the ida api ( as implemented in ida.wll )
 * implementing all functionality as available normally for idc scripts
 *
 */

// define this, so we get the flags (FF_) constants from bytes.hpp
#define BYTES_SOURCE

// ida includes
#include <pro.h>     // for basic types.
#include <ida.hpp>   // for ida constants and types.
#include <netnode.hpp>  // for RootNode
#include <expr.hpp>  // for IDCFuncs

#ifdef _MSC_VER
#pragma warning(disable:4700)
#pragma warning(disable:4100)
#pragma warning(disable:4189)
#endif

// for all functions to implement the idc functionality
#include <auto.hpp>
#include <bytes.hpp>
#include <funcs.hpp>
#include <idp.hpp>
#include <kernwin.hpp>
#include <lines.hpp>
#include <nalt.hpp>
#include <name.hpp>
#include <offset.hpp>
#include <search.hpp>
#include <segment.hpp>
#include <srarea.hpp>
#include <struct.hpp>
#include <ua.hpp>
#include <xref.hpp>
#include <frame.hpp>
#include <ieee.h>

// perl includes
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#undef do_open
#undef do_close

#ifdef DYNAMIC_PERL
#include "redefperlasdll.h"
#include "perldllprocs.h"
#endif

#include "cv_sv2idc.h"

#ifdef TRACE_XS
#define tracexs msg
#else
#define tracexs while(0)
#endif

#ifdef _PERL_THREADS
#include <boost/thread/mutex.hpp>
boost::mutex g_idcmtx;
#endif

#include <map>
#include "idchotkey.h"
#include <boost/shared_ptr.hpp>
typedef boost::shared_ptr<hotkey> hotkey_ptr;
typedef std::map<std::string,hotkey_ptr> hotkey_map;

xrefblk_t lastxr;

extfun_t *find_builtin_idc_func(const char *name)
{
    extfun_t *f= IDCFuncs.f;
    for (int i=0 ; i<IDCFuncs.qnty ; i++, f++)
    {
        if (strcmp(f->name, name)==0)
            return f;
    }
    return NULL;
}


#define MY_CXT_KEY "IDC::_guts"
typedef struct {
    hotkey_map *hotkeys;
} my_cxt_t;

START_MY_CXT

MODULE = IDC        PACKAGE = IDC  PREFIX = idaidc_

PROTOTYPES: DISABLE

BOOT:
{
    MY_CXT_INIT;
    MY_CXT.hotkeys= new hotkey_map;
    tracexs("idc_perl.xs boot, %p, %p\n", &MY_CXT, &lastxr);
}

void idaidc_END()
PREINIT:
    dMY_CXT;
CODE:
    tracexs("idaidc_END\n");
    delete MY_CXT.hotkeys;
    MY_CXT.hotkeys= NULL;

# this creates this construction:
#
# evaluated/compiled idc:
# static idcthunk_XXXX() { perlthunk_XXXX(); }
# asm code:
# perlthunk_XXXX = patched version of idcperlthunk, calling perlfunc

long idaidc_AddHotkey(const char* hotkeyname, const char* idcfunc)
PREINIT:
    dMY_CXT;
CODE:
    tracexs("AddHotkey");

    hotkey_map::iterator oldhk= MY_CXT.hotkeys->find(hotkeyname);
    if (oldhk!=MY_CXT.hotkeys->end()) {
        MY_CXT.hotkeys->erase(oldhk);
    }
    hotkey_ptr hk= hotkey_ptr(new hotkey());

    if (!hk->add(hotkeyname, idcfunc)) {
        RETVAL= -1;
    }
    else {
        MY_CXT.hotkeys->insert(hotkey_map::value_type(hotkeyname,hk));
        RETVAL= 0;
    }
OUTPUT:
    RETVAL

bool idaidc_DelHotkey(const char* hotkeyname)
PREINIT:
    dMY_CXT;
CODE:
    tracexs("DelHotkey");
    // kernwin.hpp: del_idc_hotkey(hotkey)
    hotkey_map::iterator oldhk= MY_CXT.hotkeys->find(hotkeyname);
    if (oldhk!=MY_CXT.hotkeys->end()) {
        MY_CXT.hotkeys->erase(oldhk);
    }
OUTPUT:
    RETVAL


void idaidc_exec_idcfunc(...)
PPCODE:
    if (items < 1)
        croak("Usage: exec_idcfunc(name, args)");
    const char*name= (const char *)SvPV_nolen(ST(0));
    extfun_t *fn= find_builtin_idc_func(name);
    if (fn==NULL)
        croak("unknown idc function: %s", name);
    int fnitems= strlen(fn->args);

    // validate arg count
    if ((fnitems==0 && items!=1)
            || (fn->args[fnitems-1]!=VT_WILD && fnitems!=items-1))
        croak("expected %d args for %s, got %ld\n", fnitems, name, items-1);
    if (fn->args[fnitems-1]==VT_WILD && fnitems>items) {
        croak("expected at least %d args for %s, got %ld\n", fnitems-1, name, items-1);
    }
    tracexs("idcinternal: %s, %ld params\n", name, items-1);
    idc_value_t *argv= new idc_value_t[items-1];
    idc_value_t result;
    for (int i=0 ; i<items-1 ; i++)
    {
        // first deterimine argument type
        if (i<fnitems && fn->args[i]!=VT_WILD)
            argv[i].vtype= fn->args[i];
        else {
            // variable arguments: base on perl SV type
            argv[i].vtype= sv2idctype(ST(i+1));
        }
        // then convert perl SV to idc_value_t
        sv2idcval(ST(i+1), &argv[i]);
    }

    result.num= items;  // nr of arguments
    error_t err=0;
#ifdef _PERL_THREADS
    {
    boost::mutex::scoped_lock lock(g_idcmtx);
#endif
    err= fn->fp(argv, &result);
#ifdef _PERL_THREADS
    }
#endif
    delete[] argv;
    if (err) {
        msg("error in %s: %d\n", name, err);
        XSRETURN_UNDEF;
    }
    ST(0)= newSVidc(&result);
    sv_2mortal(ST(0));
    XSRETURN(1);
    // note: need to research if the 'unreachable code' after XSRETURN has any consequences.
    // the 'PUTBACK'  is now 'unreachable'

