/* vim:set ts=4 sw=4:
 *
 * (C) 2008 Willem Hengeveld  itsme@xs4all.nl
 *
 * IdcPerl - a perl scripting plugin for the Interactive Disassembler by hex-rays.
 * see http://www.xs4all.nl/~itsme/projects/idcperl
 *
 * this file contains the code to init, and call the perl interpreter
 *
 */

// perl includes
#include "extern.h"
#include "perl.h"
#include "xsub.h"
#include "proto.h"
#include "perlapi.h"    // PL_exit_flags
#undef do_open
#undef do_close

#ifdef DYNAMIC_PERL
#include "redefperlasdll.h"
#include "perldllprocs.h"
#endif

#include "perlinterp.h"
#include "perl_magic.h"
#include "iolayer.h"

// work around namespace collisions
#undef unpack_str

#include "kernwin.hpp"  // for msg()
#include "diskio.hpp"   // for idadir()

//#include "cancelthread.h"

#include <string>

#include "cv_sv2idc.h"

#ifdef TRACE_INTERP
#define tracemsg msg
#else
#define tracemsg(...)
#endif

void xs_init(pTHX);

static void perlinit(bool init)
{
    static int refcount= 0;

    msg("perlinit(i=%d, count=%d\n", init, refcount);
    if (init) {
        if (refcount++ == 0) {
            static int argc=1;
            static const char *args[]= { "ida", NULL };
            static char **argv= const_cast<char**>(args);
            PERL_SYS_INIT(&argc,&argv);
        }
    }
    else {
        if (--refcount) {
            PERL_SYS_TERM();
        }
    }
}
Perl::Perl(PerlInterpreter *interp/*=NULL*/) : perl_interp(interp)
//               , _tid(GetCurrentThreadId())
{
    perlinit(true);

    tracemsg("new Perl(%p, host=%08lx)\n", interp, interp?*(long*)interp:0x12345678);
}
Perl::~Perl()
{
    free();

    perlinit(false);
}
#ifdef USE_ITHREADS
Perl* Perl::clone()
{
    tracemsg("Perl::clone, cloning %p, host=%08lx\n", perl_interp, *(long*)perl_interp);
    PERL_SET_CONTEXT(perl_interp);
    return new Perl(perl_clone(perl_interp, CLONEf_CLONE_HOST));
}
#endif
bool Perl::initialize()
{
    // note: these have to be 'char*'  because perl_parse and call_argv don't specify 'const'
    char	*bootargs[] = { "IDC", NULL };
    static char *args[] = { "", "-e", "0" };

    tracemsg("Perl::initialize\n");
    if (perl_interp)
        return true;

    perl_interp = perl_alloc();
    if (perl_interp==NULL)
        return false;
    perl_construct(perl_interp);

    init_idamsg_io(aTHX);

    tracemsg("calling xs_init\n");
    int status= perl_parse(perl_interp, xs_init, sizeof(args)/sizeof(char*), args, 0);
    if (status)
        return false;

    // add perl subdir of the ida application dir to perl search path @INC
    // NOTE: need to do this -after- perl_parse, since PL_incgv is 
    // initialized in perl_parse
    av_push(GvAV(PL_incgv), newSVpv(idadir("perl"),0));

    PL_exit_flags |= PERL_EXIT_DESTRUCT_END;

    tracemsg("calling IDC::bootstrap\n");
    // 'use IDC'
    call_argv("IDC::bootstrap", (long)G_DISCARD, bootargs);

    register_magic(aTHX);

    tracemsg("Perl::initialize done\n");
    return true;
}

void Perl::free()
{
    tracemsg("Perl::free(%p)\n", perl_interp);
    if (perl_interp)
    {
        PERL_SET_CONTEXT(perl_interp);
        //perl_run(perl_interp); ... not needed because of PERL_EXIT_DESTRUCT_END
        perl_destruct(perl_interp);
        perl_free(perl_interp);
        perl_interp = NULL;
    }
}

bool Perl::exec(const char*perlcode, char *errbuf, size_t errbufsize)
{
    PERL_SET_CONTEXT(perl_interp);

    SV *codesv= newSVpv(perlcode, 0);

    bool errenable= true;
    if (errbufsize==0)
        errenable= enablestderr(false);

    sv_setpvn(ERRSV, "", 0);

    I32 count=0;
    try {
    //cancelthread ct(this);
    count= eval_sv(codesv, G_KEEPERR|G_VOID|G_DISCARD);
    //ct.finished();
    }
    catch(...) {
        tracemsg("caught exception\n");  
    }
    if (errbufsize==0)
        enablestderr(errenable);

    // check eval result.
    if (SvTRUE(ERRSV)) {
        tracemsg("IDAPERL exec-ERROR: %s\n", SvPV_nolen(ERRSV));
        if (errbuf)
            qstrncpy(errbuf, SvPV_nolen(ERRSV), errbufsize);
        return false;
    }
    return true;
}

bool Perl::run(const char *name, int nargs, const idc_value_t args[], idc_value_t *result, char *errbuf, size_t errbufsize)
{
    bool bResult= true;
    tracemsg("run: %s(%s) -> %d\n", name, argstring(nargs, args).c_str(), (int)errbufsize);
    PERL_SET_CONTEXT(perl_interp);
    dSP;
    ENTER;
    SAVETMPS;
    PUSHMARK(SP);
    for (int i=0 ; i<nargs ; i++)
    {
	    XPUSHs(sv_2mortal(newSVidc(&args[i])));
    }
	PUTBACK;

    bool errenable= true;
    if (errbufsize==0)
        errenable= enablestderr(false);

    sv_setpvn(ERRSV, "", 0);
    I32 count=0;
    try {
    count= call_pv(name, G_SCALAR|G_KEEPERR|G_EVAL);
    }
    catch(...) {
        tracemsg("caught exception\n");  
    }
    if (errbufsize==0)
        enablestderr(errenable);

    SPAGAIN;

    if (SvTRUE(ERRSV)) {
        tracemsg("IDAPERL run-ERROR: %s\n", SvPV_nolen(ERRSV));
        if (errbuf)
            qstrncpy(errbuf, SvPV_nolen(ERRSV), errbufsize);
        bResult= false;
        (void)POPs;
    }
    else if (!result) {
        // no result required
    }
    else if (count) {
        SV *retsv= POPs;
        result->vtype= sv2idctype(retsv);
        sv2idcval(retsv, result);
    }
    else {
        result->vtype= VT_LONG;
        result->num= 0;
    }

    PUTBACK;

    FREETMPS;
    LEAVE;

    return bResult;
}

void Perl::cancel()
{
    PERL_SET_CONTEXT(perl_interp);

// xsub.h      kill -> PerlProc_kill 
// iperlsys.h  PerlProc_kill -> (*proc)->kill
// perlhost.h  proc.kill = PerlProcKill 
// perlhost.h  PerlProcKill -> win32_kill

    croak("script killed\n");
    //win32_kill(-_tid, 1);
}

#ifndef DYNAMIC_PERL
EXTERN_C void (*boot_DynaLoader)_((pTHX_ CV*));
#endif
EXTERN_C void boot_IDC (pTHX_ CV*);

void xs_init(pTHX)
{
    tracemsg("Perl::xs_init\n");
    // note: in perl 5.8.9 this must be non-const
    const char *file = "idc_perl.xs";

    PERL_SET_CONTEXT(my_perl);

#ifdef __NT__
    // dynaloader is needed to load perlXX.dll under windows.
    newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, file);
#endif
    newXS("IDC::bootstrap", boot_IDC, file);

    tracemsg("xs_init done\n");
}
