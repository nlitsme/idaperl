/* vim:set ts=4 sw=4:
 *
 * (C) 2008 Willem Hengeveld  itsme@xs4all.nl
 *
 * IdcPerl - a perl scripting plugin for the Interactive Disassembler by hex-rays.
 * see http://www.xs4all.nl/~itsme/projects/idcperl
 *
 * this file contains code handling magic 'uvar's
 *     that is a perl scalar with a 'set' and 'get' function.
 *
 * currently that is just '$here'
 */

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

#include "perl_magic.h"

// work around namespace collisions
#undef unpack_str

// ida includes
#include "pro.h"  // for uchar
#include "kernwin.hpp"  // for msg()

#ifdef TRACE_MAGIC
#define tracemsg msg
#else
#define tracemsg(...)
#endif
// do i need to pass pTHX here, for the current interpreter?
//   NO: it is handled implicitly by the sv_setiv macro, which calls 'Perl_get_context()'
struct magichandler {
    virtual ~magichandler() { }
    virtual const char* name() const=0;
    virtual bool readonly() const=0;
    virtual void getter(SV *) const=0;
    virtual void setter(SV *)=0;
};
struct here_handler : magichandler {
    virtual const char* name() const { return "main::here"; }
    virtual bool readonly() const { return false; }
    virtual void getter(SV *sv) const { sv_setiv(sv, get_screen_ea()); }
    virtual void setter(SV *sv) { jumpto(SvIV(sv),0); }
};

struct magichandler* magicitems[]={
    new here_handler(),
};
#define NMAGICITEMS (sizeof(magicitems)/sizeof(*magicitems))
I32 get_magic(pTHX_ IV iv, SV *sv)
{
    PERL_SET_CONTEXT(my_perl);
    if (iv < 0 || unsigned(iv) >= NMAGICITEMS) {
        msg("IDAPERL ERROR: unknown magic id: %ld\n", iv);
        return 0;
    }
    magicitems[iv]->getter(sv);
    return 0;
}
I32 set_magic(pTHX_ IV iv, SV *sv)
{
    PERL_SET_CONTEXT(my_perl);
    if (iv < 0 || unsigned(iv) >= NMAGICITEMS) {
        msg("IDAPERL ERROR: unknown magic id: %ld\n", iv);
        return 0;
    }
    if (!magicitems[iv]->readonly())
        magicitems[iv]->setter(sv);
    return 0;
}

void register_magic(pTHX)
{
    PERL_SET_CONTEXT(my_perl);
    tracemsg("register_magic\n");
    SV *sv;
    for (unsigned i=0 ; i<NMAGICITEMS ; i++) {
        sv = perl_get_sv(magicitems[i]->name(), TRUE);
        struct ufuncs uf;
        uf.uf_val= get_magic;
        uf.uf_set= set_magic;
        uf.uf_index= i;
        sv_magic(sv, NULL, PERL_MAGIC_uvar, (char *)&uf, sizeof(uf));
        if (magicitems[i]->readonly())
            SvREADONLY_on(sv);
    }
}
