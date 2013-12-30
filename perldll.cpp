/* vim:set ts=4 sw=4:
 *
 * (C) 2008 Willem Hengeveld  itsme@xs4all.nl
 *
 * IdcPerl - a perl scripting plugin for the Interactive Disassembler by hex-rays.
 * see http://www.xs4all.nl/~itsme/projects/idcperl
 *
 * this module contains code that dynamically imports all functions from DYNAMIC_PERL_DLL
 *
 */

#include <string>

#include "perldll.h"
struct PerlFuncDef {
    char* name;
    FARPROC* ptr;
};
// ida includes
#include "pro.h"     // for basic types.
#include "ida.hpp"   // for ida constants and types.
#include "idp.hpp"   // for interface version
#include "netnode.hpp"  // for RootNode
#include "expr.hpp"  // for IDCFuncs


// perl includes
#include "extern.h"
#include "perl.h"
#include "xsub.h"
#undef do_open
#undef do_close

#include "redefperlasdll.h"

#include <shlwapi.h>   // FilePathExists

#ifdef TRACE_DLL
#define tracemsg msg
#else
#define tracemsg(...)
#endif

// empty def, this is the module defining the function ptrs.
#define PERLDEFINE
#include "perldllprocs.h"

static PerlFuncDef perl_funcname_table[] = {
#include "perldllprocnames.inc"
};

bool PerlDll::load_procs()
{
    for (PerlFuncDef *p=perl_funcname_table ; p->ptr ; p++)
    {
        *p->ptr = GetProcAddress(_h, p->name);
        if (*p->ptr==NULL)
        {
            msg("IDAPERL ERROR loading proc %s\n", p->name);
            return false;
        }
    }
    return true;
}
void PerlDll::unload()
{
    tracemsg("PerlDll::unload\n");
    if (_h) {
        PERL_SYS_TERM();
        FreeLibrary(_h);
        _h = NULL;
    }
}

bool search_regkey(HKEY hKey, const std::string& regpath, const std::string& name, std::string& dllname)
{
    HKEY hKeyAP;
    LONG rc;
    rc= RegOpenKeyEx(hKey, regpath.c_str(), 0, KEY_READ, &hKeyAP);
    if (rc==0) {
        DWORD dllnamesize= MAX_PATH;
        dllname.resize(dllnamesize);
        rc= RegQueryValueEx(hKeyAP, 0, 0, 0, (BYTE*)&dllname[0], &dllnamesize);
        RegCloseKey(hKeyAP);
        if (rc==0) {
            dllname.resize(strlen(dllname.c_str()));
			if (dllname[dllname.size()-1]!='\\' && dllname[dllname.size()-1]!='/')
				dllname += "\\";
            dllname += "bin\\";
            dllname += name;
            if (PathFileExists(dllname.c_str()))
                return true;
        }
    }
    return false;
}
bool search_activestate_regkeys(const std::string& name, std::string& dllname)
{
    HKEY hKey;
    LONG rc;
    rc= RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Activestate\\ActivePerl", 0, KEY_READ, &hKey);
    if (rc)
        return false;
    char apversion[256];
    for (int i=0 ; 1 ; i++) {
        DWORD namelen= 256;
        rc= RegEnumKeyEx(hKey, i, apversion, &namelen, 0, 0, 0, 0);
        if (rc==ERROR_NO_MORE_ITEMS)
            break;
        if (search_regkey(hKey, apversion, name, dllname))
            return true;
    }
    return false;
}
bool search_perl_regkey(const std::string& name, std::string& dllname)
{
    return search_regkey(HKEY_LOCAL_MACHINE, "Software\\Perl", name, dllname);
}
bool search_perl_directories(const std::string& name, std::string& dllname)
{
    // todo - try c:\perl*\bin\<dllname>
    return false;
}
bool searchperldll(const std::string& name, std::string& dllname)
{
    // HKLM\software\activestate\ActivePerl\*
    return search_activestate_regkeys(name, dllname)
    // HKLM\software\perl
            || search_perl_regkey(name, dllname)
    // c:\perl*
            || search_perl_directories(name, dllname);
}
bool PerlDll::load()
{
    tracemsg("PerlDll::load\n");

    std::string dllname;
    if (!searchperldll(DYNAMIC_PERL_DLL, dllname)) {
        // try loading it from the searchpath
        dllname= DYNAMIC_PERL_DLL;
    }
    msg("loading %s\n", dllname.c_str());
    _h= LoadLibrary(dllname.c_str());
    if (_h==NULL || _h==INVALID_HANDLE_VALUE) {
        msg("IDAPERL ERROR: LoadLibrary %s\n", dllname.c_str());
        return false;
    }
    if (!load_procs()) {
        unload();
        return false;
    }

    int argc=0;
    char **argv=NULL;
    char **env=NULL;
    PERL_SYS_INIT3(&argc,&argv,&env);

    tracemsg("PerlDll::load done\n");
    return true;
}

