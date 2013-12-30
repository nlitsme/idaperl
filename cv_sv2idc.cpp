/* vim:set ts=4 sw=4:
 *
 * (C) 2008 Willem Hengeveld  itsme@xs4all.nl
 *
 * IdcPerl - a perl scripting plugin for the Interactive Disassembler by hex-rays.
 * see http://www.xs4all.nl/~itsme/projects/idcperl
 *
 */
// ida includes
#include <pro.h>     // for basic types.
#include <ida.hpp>   // for ida constants and types.
#include <expr.hpp>  // for IDCFuncs, idc_value_t
#include <idp.hpp>   // for ph

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

#ifdef _MSC_VER
#pragma warning(disable:4700)
#pragma warning(disable:4100)
#pragma warning(disable:4189)
#endif

#include <string>

#include "cv_sv2idc.h"
char sv2idctype(const SV *sv)
{
    if (SvIOK(sv)) return VT_LONG;
    else if (SvNOK(sv)) return VT_FLOAT;
    else if (SvPOK(sv)) return VT_STR;
    else {
        // otherwise, probably an object -> stringify
        return VT_STR;
    }
}
void sv2idcval(SV *sv, idc_value_t *val)
{
    switch(val->vtype)
    {
        // note: idc_value_t calls qfree in it's destructor.
        // problem: if ST is '$1'  this somehow does not work.
        case VT_STR:   val->str= qstrdup(SvPV_nolen_const(sv)); break;
        case VT_LONG:  val->num= SvIV(sv); break;
        case VT_FLOAT: double nv= SvNV(sv);
                       ph.realcvt(&nv, val->e, 3);
                       break;
    }
}
SV* newSVidc(const idc_value_t* val)
{
    switch(val->vtype)
    {
        case VT_STR:   return newSVpv(val->str, 0);
        case VT_LONG:  return newSViv(val->num);
        case VT_FLOAT: double nv;
                       ph.realcvt(&nv, const_cast<ushort*>(val->e), 13);
                       return newSVnv(nv);
    }
    // ... error: invalid vtype
    return NULL;
}

const char*hexstring(sval_t num)
{
    static char hexbuf[16];
    qsnprintf(hexbuf, 16, "0x%08x", num);
    return hexbuf;
}
const char*dblstring(const ushort *e)
{
    double dbl;
    ph.realcvt(&dbl, const_cast<ushort*>(e), 13);
    static char dblbuf[16];
    qsnprintf(dblbuf, 16, "%g", dbl);
    return dblbuf;
}
std::string argstring(int nargs, const idc_value_t args[])
{
    std::string buf;
    for (int i=0 ; i<nargs ; i++)
    {
        if (i)
            buf += ", ";
        switch(args[i].vtype)
        {
            case VT_LONG:   buf+=hexstring(args[i].num); break;
            case VT_STR:    buf+=std::string("\"")+args[i].str+"\""; break;
            case VT_FLOAT:  buf+=dblstring(args[i].e); break;
            case VT_WILD:   buf+="..."; break;
                            // todo: implement these value types
            case VT_OBJ:    buf+="?obj?"; break;
            case VT_FUNC:    buf+="?func?"; break;
            case VT_STR2:    buf+="?str2?"; break;
            case VT_PVOID:    buf+="?pvoid?"; break;
            case VT_INT64:    buf+="?i64?"; break;
            case VT_REF:    buf+="?ref?"; break;
            default:
                            buf+="???";
        }
    }
    return buf;
}

