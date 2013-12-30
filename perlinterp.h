/* vim:set ts=4 sw=4:
 *
 * (C) 2008 Willem Hengeveld  itsme@xs4all.nl
 *
 * IdcPerl - a perl scripting plugin for the Interactive Disassembler by hex-rays.
 * see http://www.xs4all.nl/~itsme/projects/idcperl
 *
 * 'Perl' is a c++ wrapper around the perl interpreter.
 *
 */
#ifndef _PERLINTERP_H_
#define _PERLINTERP_H_

// perl includes
//#include "EXTERN.h"
//#include "perl.h"
//#include "XSUB.h"

// ida includes
#include <pro.h>     // for basic types.
#include <ida.hpp>   // for ida constants and types.
#include <expr.hpp>  // for IDCFuncs

struct interpreter;
class Perl {
public:
    explicit Perl(struct interpreter *interp=NULL);
    ~Perl();
#ifdef USE_ITHREADS
    Perl* clone();
#endif

    bool initialize();
    void free();
    bool exec(const char*perlcode, char *errbuf, size_t errbufsize);
    bool run(const char *name, int nargs, const idc_value_t args[], idc_value_t *result, char *errbuf, size_t errbufsize);
    void cancel();

private:
    struct interpreter *perl_interp;
//    int _tid;

    //static void xs_init(pTHX);
};
#endif

