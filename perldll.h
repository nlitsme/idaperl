/* vim:set ts=4 sw=4:
 *
 * (C) 2008 Willem Hengeveld  itsme@xs4all.nl
 *
 * IdcPerl - a perl scripting plugin for the Interactive Disassembler by hex-rays.
 * see http://www.xs4all.nl/~itsme/projects/idcperl
 *
 *
 * 'PerlDll' dynamically loads the perlXX.dll
 *
 */
#ifndef _PERLDLL_H_
#define _PERLDLL_H_
#include <windows.h>    // for HMODULE
class PerlDll {
private:
    HMODULE _h;
    bool load_procs();
public:
    PerlDll() : _h(0) { }
    ~PerlDll() { unload(); }
    bool load();
    void unload();
    bool isloaded() const { return _h!=0; }
};
#endif
