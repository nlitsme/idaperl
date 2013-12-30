/* vim:set ts=4 sw=4:
 *
 * (C) 2008 Willem Hengeveld  itsme@xs4all.nl
 *
 * IdcPerl - a perl scripting plugin for the Interactive Disassembler by hex-rays.
 * see http://www.xs4all.nl/~itsme/projects/idcperl
 *
 */
#ifndef __CV_SV2IDC_H_
#define __CV_SV2IDC_H_
#include <string>
char sv2idctype(const SV *sv);
void sv2idcval(SV *sv, idc_value_t *val);
SV* newSVidc(const idc_value_t* val);

std::string argstring(int nargs, const idc_value_t args[]);
#endif
