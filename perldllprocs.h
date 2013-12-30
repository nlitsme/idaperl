/* vim:set ts=4 sw=4:
 *
 * (C) 2008 Willem Hengeveld  itsme@xs4all.nl
 *
 * IdcPerl - a perl scripting plugin for the Interactive Disassembler by hex-rays.
 * see http://www.xs4all.nl/~itsme/projects/idcperl
 *
 *   this file defines or declares function pointers to all perl functions
 *
 *   defining PERLDEFINE prior to including this file, defines all function pointers
 *
 *  for missing declarations, see C:\local\perl589\lib\CORE\proto.h
 *  and C:\local\perl589\lib\CORE\intrpvar.h
 *   translate PERLVARI(name,type,dfl) or PERLVAR(name,type)
 *          to 
 *  PERLDECL type* (*Perl_#name#_ptr)(pTHX);
 *
 *  translate PERL_CALLCONV type name(args)
 *          to
 *  PERLDECL type (*name)(args);
 *
 */
#ifndef _PERLDLLPROCS_H_
#define _PERLDLLPROCS_H_

#include "patchlevel.h"

#ifndef PERLDEFINE
#define PERLDECL extern
#else
#define PERLDECL
#endif

#ifdef __cplusplus
extern "C" {
#endif

PERLDECL PerlInterpreter* (*perl_alloc)();
PERLDECL void (*perl_construct)(pTHX);
PERLDECL void (*perl_destruct)(pTHX);
PERLDECL void (*perl_free)(pTHX);
PERLDECL int (*perl_run)(pTHX);
PERLDECL int (*perl_parse)(PerlInterpreter*, XSINIT_t, int, char**, char**);
PERLDECL void* (*Perl_get_context)(void);
PERLDECL void (*Perl_croak)(pTHX_ const char*, ...);
PERLDECL void (*Perl_croak_nocontext)(const char*, ...);
PERLDECL I32 (*Perl_dowantarray)(pTHX);
PERLDECL void (*Perl_free_tmps)(pTHX);
PERLDECL HV* (*Perl_gv_stashpv)(pTHX_ const char*, I32);
PERLDECL void (*Perl_markstack_grow)(pTHX);
PERLDECL MAGIC* (*Perl_mg_find)(pTHX_ SV*, int);
PERLDECL int (*Perl_mg_set)(pTHX_ SV*);
PERLDECL CV* (*Perl_newXS)(pTHX_ char*, XSUBADDR_t, char*);
PERLDECL SV* (*Perl_newSV)(pTHX_ STRLEN);
PERLDECL SV* (*Perl_newSViv)(pTHX_ IV);
PERLDECL SV* (*Perl_newSVpv)(pTHX_ const char*, STRLEN);
PERLDECL SV* (*Perl_newSVnv)(pTHX_ NV);
PERLDECL I32 (*Perl_call_argv)(pTHX_ const char*, I32, char**);
PERLDECL I32 (*Perl_call_pv)(pTHX_ const char*, I32);
PERLDECL I32 (*Perl_eval_sv)(pTHX_ SV*, I32);
PERLDECL SV* (*Perl_get_sv)(pTHX_ const char*, I32);
PERLDECL SV* (*Perl_eval_pv)(pTHX_ const char*, I32);
PERLDECL SV* (*Perl_call_method)(pTHX_ const char*, I32);
PERLDECL void (*Perl_pop_scope)(pTHX);
PERLDECL void (*Perl_push_scope)(pTHX);
PERLDECL void (*Perl_save_int)(pTHX_ int*);
PERLDECL SV** (*Perl_stack_grow)(pTHX_ SV**, SV**p, int);
PERLDECL SV** (*Perl_set_context)(void*);
PERLDECL AV* (*Perl_newAV)(pTHX);
PERLDECL AV* (*Perl_get_av)(pTHX_ const char* name, I32 create);
PERLDECL void  (*Perl_av_push)(pTHX_ AV* ar, SV* val);
PERLDECL bool (*Perl_sv_2bool)(pTHX_ SV*);
PERLDECL IV (*Perl_sv_2iv)(pTHX_ SV*);
PERLDECL SV* (*Perl_sv_2mortal)(pTHX_ SV*);
PERLDECL SV* (*Perl_sv_newmortal)(pTHX);
#if (PERL_REVISION == 5) && (PERL_VERSION >= 8)
PERLDECL char* (*Perl_sv_2pv_flags)(pTHX_ SV*, STRLEN*, I32);
PERLDECL UV	(*Perl_sv_2uv_flags)(pTHX_ SV* sv, I32 flags);
PERLDECL IV (*Perl_sv_2iv_flags)(pTHX_ SV* sv, I32 flags);
PERLDECL void*	(*Perl_hv_common)(pTHX_ HV* tb, SV* keysv, const char* key, STRLEN klen, int flags, int action, SV* val, U32 hash);
PERLDECL char* (*Perl_sv_2pv_nolen)(pTHX_ SV*);
#else
PERLDECL char* (*Perl_sv_2pv)(pTHX_ SV*, STRLEN*);
#endif
PERLDECL NV (*Perl_sv_2nv)(pTHX_ SV*);
PERLDECL SV* (*Perl_sv_bless)(pTHX_ SV*, HV*);
#if (PERL_REVISION == 5) && (PERL_VERSION >= 8)
PERLDECL void (*Perl_sv_catpvn_flags)(pTHX_ SV* , const char*, STRLEN, I32);
#else
PERLDECL void (*Perl_sv_catpvn)(pTHX_ SV*, const char*, STRLEN);
#endif
PERLDECL void (*Perl_sv_catpv)(pTHX_ SV*, const char*);
PERLDECL void (*Perl_sv_free)(pTHX_ SV*);
PERLDECL int (*Perl_sv_isa)(pTHX_ SV*, const char*);
PERLDECL void (*Perl_sv_magic)(pTHX_ SV*, SV*, int, const char*, I32);
PERLDECL void (*Perl_sv_setiv)(pTHX_ SV*, IV);
PERLDECL void (*Perl_sv_setpv)(pTHX_ SV*, const char*);
PERLDECL void (*Perl_sv_setpvn)(pTHX_ SV*, const char*, STRLEN);
#if (PERL_REVISION == 5) && (PERL_VERSION >= 8)
PERLDECL void (*Perl_sv_setsv_flags)(pTHX_ SV*, SV*, I32);
#else
PERLDECL void (*Perl_sv_setsv)(pTHX_ SV*, SV*);
#endif
PERLDECL bool (*Perl_sv_upgrade)(pTHX_ SV*, U32);

#if (PERL_REVISION == 5) && (PERL_VERSION <= 8)
PERLDECL SV*** (*Perl_Tstack_sp_ptr)(pTHX);
PERLDECL SV*** (*Perl_Tcurpad_ptr)(pTHX);
PERLDECL OP** (*Perl_Top_ptr)(pTHX);
PERLDECL SV*** (*Perl_Tstack_base_ptr)(pTHX);
PERLDECL SV*** (*Perl_Tstack_max_ptr)(pTHX);
PERLDECL I32* (*Perl_Ttmps_ix_ptr)(pTHX);
PERLDECL I32* (*Perl_Ttmps_floor_ptr)(pTHX);
PERLDECL I32** (*Perl_Tmarkstack_ptr_ptr)(pTHX);
PERLDECL I32** (*Perl_Tmarkstack_max_ptr)(pTHX);
PERLDECL SV** (*Perl_TSv_ptr)(pTHX);
PERLDECL XPV** (*Perl_TXpv_ptr)(pTHX);
#else
PERLDECL SV*** (*Perl_Istack_sp_ptr)(pTHX);
PERLDECL SV*** (*Perl_Icurpad_ptr)(pTHX);
PERLDECL OP** (*Perl_Iop_ptr)(pTHX);
PERLDECL SV*** (*Perl_Istack_base_ptr)(pTHX);
PERLDECL SV*** (*Perl_Istack_max_ptr)(pTHX);
PERLDECL I32* (*Perl_Itmps_ix_ptr)(pTHX);
PERLDECL I32* (*Perl_Itmps_floor_ptr)(pTHX);
PERLDECL I32** (*Perl_Imarkstack_ptr_ptr)(pTHX);
PERLDECL I32** (*Perl_Imarkstack_max_ptr)(pTHX);
PERLDECL SV** (*Perl_ISv_ptr)(pTHX);
PERLDECL XPV** (*Perl_IXpv_ptr)(pTHX);

PERLDECL void*** (*Perl_Imy_cxt_list_ptr)(pTHX);
PERLDECL I32* (*Perl_Iscopestack_ix_ptr)(pTHX);
PERLDECL AV** (*Perl_Iunitcheckav_ptr)(pTHX);
PERLDECL void (*Perl_call_list)(pTHX_ I32 oldscope, AV* av_list);
#ifdef PERL_IMPLICIT_CONTEXT
#ifdef PERL_GLOBAL_STRUCT_PRIVATE
PERLDECL void* (*Perl_my_cxt_init)(pTHX_ const char *my_cxt_key, size_t size);
#else
PERLDECL void* (*Perl_my_cxt_init)(pTHX_ int *index, size_t size);
#endif
#endif

#endif
#if (PERL_REVISION == 5) && (PERL_VERSION <= 8)
PERLDECL STRLEN* (*Perl_Tna_ptr)(pTHX);
#endif
PERLDECL GV** (*Perl_Idefgv_ptr)(pTHX);
PERLDECL GV** (*Perl_Ierrgv_ptr)(pTHX);
PERLDECL GV** (*Perl_Iincgv_ptr)(pTHX);
PERLDECL U8* (*Perl_Iexit_flags_ptr)(pTHX);
PERLDECL SV* (*Perl_Isv_yes_ptr)(pTHX);
PERLDECL SV* (*Perl_Isv_no_ptr)(pTHX);
PERLDECL SV* (*Perl_Isv_undef_ptr)(pTHX);
PERLDECL void (*boot_DynaLoader)_((pTHX_ CV*));

PERLDECL PerlIO* (*PerlIO_push)(pTHX_ PerlIO *f, PERLIO_FUNCS_DECL(*),const char *mode, SV *arg);
PERLDECL PerlIO* (*Perl_PerlIO_stdout)(pTHX);
PERLDECL PerlIO* (*Perl_PerlIO_stderr)(pTHX);
PERLDECL void (*PerlIO_define_layer)(pTHX_ PerlIO_funcs *);
PERLDECL IV (*PerlIOBase_pushed)(pTHX_ PerlIO *f, const char *mode, SV *arg, PerlIO_funcs *tab);
PERLDECL void (*Perl_sv_copypv)(pTHX_ SV* dsv, SV* ssv);
PERLDECL char* (*Perl_sv_2pvbyte_nolen)(pTHX_ SV* sv);
PERLDECL char* (*Perl_sv_2pvbyte)(pTHX_ SV* sv, STRLEN* lp);
PERLDECL PerlInterpreter* (*perl_clone)(pTHX_ UV flags);
PERLDECL SV**	(*Perl_hv_fetch)(pTHX_ HV* tb, const char* key, I32 klen, I32 lval);
PERLDECL UV	(*Perl_sv_2uv)(pTHX_ SV* sv);
PERLDECL HV** (*Perl_Imodglobal_ptr)(pTHX);
PERLDECL void (*Perl_sv_setuv)(pTHX_ SV* sv, UV num);
PERLDECL int (*win32_kill)(int pid, int sig);

PERLDECL void (*Perl_sys_init)(int* argc, char*** argv);
PERLDECL void (*Perl_sys_init3)(int* argc, char*** argv, char*** env);
PERLDECL void (*Perl_sys_term)(void);

#ifdef __cplusplus
}
#endif
#endif
