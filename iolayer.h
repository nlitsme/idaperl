/* vim:set ts=4 sw=4:
 *
 * (C) 2008 Willem Hengeveld  itsme@xs4all.nl
 *
 * IdcPerl - a perl scripting plugin for the Interactive Disassembler by hex-rays.
 * see http://www.xs4all.nl/~itsme/projects/idcperl
 *
 */
#ifndef __IOLAYER_H__
#define __IOLAYER_H__
void init_idamsg_io(pTHX);
bool enablestderr(bool bNewState);
#endif
