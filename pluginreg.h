/* vim:set ts=4 sw=4:
 *
 * (C) 2008 Willem Hengeveld  itsme@xs4all.nl
 *
 * IdcPerl - a perl scripting plugin for the Interactive Disassembler by hex-rays.
 * see http://www.xs4all.nl/~itsme/projects/idcperl
 *
 */
#ifndef _PLUGINREG_H_
#define _PLUGINREG_H_

#define IDAPERLNODE "$ idaperl"

// note: you have to choose the supval id's for the blobs
// such that the distance*MAXSPECSIZE >= the maximum desired blob size
//
// a blob consists of the contents of all consequetive nodes with values.
//
// supval where we keep our manually entered script code
// using the same value as ida, in nalt.hpp
//  ... ida also uses RIDX_SMALL_IDC
#define ISUP_MANUAL  0x2000

#define ISUP_AUTORUN 0x2100

#define ISUP_SAVED  0x2200
#define MAX_SAVEDSCRIPTS  0x20

Perl *cloneinterp();
void destroyclone(Perl *);

#endif
