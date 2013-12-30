/* vim:set ts=4 sw=4:
 *
 * (C) 2008 Willem Hengeveld  itsme@xs4all.nl
 *
 * IdcPerl - a perl scripting plugin for the Interactive Disassembler by hex-rays.
 * see http://www.xs4all.nl/~itsme/projects/idcperl
 *
 */
#include <EXTERN.h>
#include <perl.h>
#include <perlio.h>
#include <perliol.h>
#undef do_open
#undef do_close

/*
 * this file defines a perl 'iolayer' named 'idamsg',
 * which outputs everything in the ida message log.
 * and adds this layer to the stdout, and stderr chain.
 */

#ifdef DYNAMIC_PERL
#include "redefperlasdll.h"
#include "perldllprocs.h"
#endif

#include "iolayer.h"

// work around namespace collisions
#undef unpack_str

// ida includes
#include <pro.h>  // for uchar
#include <kernwin.hpp>  // for msg()

#ifdef TRACE_PERLIO
#define tracemsg msg
#else
#define tracemsg(...)
#endif

static bool g_stderrenabled= true;
bool enablestderr(bool bNewState)
{
    tracemsg("enablestderr\n");
    bool bOldState= g_stderrenabled;
    g_stderrenabled= bNewState;
    return bOldState;
}
SSize_t PerlIOidamsg_write(pTHX_ PerlIO *f, const void *vbuf, Size_t count)
{
    tracemsg("PerlIOidamsg_write\n");
    if (!g_stderrenabled)
        return count;

    SSize_t total= 0;
    Size_t ofs= 0;
    while (ofs<count) {
        Size_t printed= msg("%.*s", (int)(count-ofs), (const char*)vbuf+ofs);
        ofs += printed;
        if (printed==0)
            break;
    }
    return total;
}
static PerlIO_funcs PerlIO_idamsg = {
    sizeof(PerlIO_funcs),        
    "idamsg",                    
    sizeof(_PerlIO),
    PERLIO_K_MULTIARG | PERLIO_K_RAW,
    PerlIOBase_pushed,         
    NULL,                       /* PerlIOBase_popped */
    NULL,                       /* PerlIOidamsg_open */
    NULL,                       /* PerlIOBase_binmode */
    NULL,                       /* no getarg needed */
    NULL,                       /* PerlIOidamsg_fileno */
    NULL,                       /* PerlIOidamsg_dup */
    NULL,                       /* PerlIOidamsg_read */
    NULL,                       /* PerlIOBase_unread */
    PerlIOidamsg_write,         /* PerlIOidamsg_write */
    NULL,                       /* PerlIOidamsg_seek */
    NULL,                       /* PerlIOidamsg_tell */
    NULL,                       /* PerlIOidamsg_close */
    NULL,                       /* PerlIOidamsg_flush */
    NULL,                       /* PerlIOidamsg_noop_fail - fill */
    NULL,                       /* PerlIOidamsg_eof */
    NULL,                       /* PerlIOBase_error */
    NULL,                       /* PerlIOBase_clearerr */
    NULL,                       /* PerlIOBase_setlinebuf */
    NULL,                       /* get_base */
    NULL,                       /* get_bufsiz */
    NULL,                       /* get_ptr */
    NULL,                       /* get_cnt */
    NULL,                       /* set_ptrcnt */
};

void init_idamsg_io(pTHX)
{
    tracemsg("init_idamsg_io1\n");
    PerlIO_define_layer(aTHX_ &PerlIO_idamsg);

//binmode ":idamsg", stdout;
    PerlIO_push(aTHX_ PerlIO_stdout(), &PerlIO_idamsg, "a", NULL);
//binmode ":idamsg", stderr;
    PerlIO_push(aTHX_ PerlIO_stderr(), &PerlIO_idamsg, "a", NULL);
}

