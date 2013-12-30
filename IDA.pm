package IDA;
# vim:set ts=4 sw=4:
#
# (C) 2008 Willem Hengeveld  itsme@xs4all.nl
#
# IdcPerl - a perl scripting plugin for the Interactive Disassembler by hex-rays.
# see http://www.xs4all.nl/~itsme/projects/idcperl
#

use strict;
use warnings;
use IDC;
require Exporter;
our @ISA= qw(Exporter);
our @EXPORT= qw(forrefs forfunctions);

# todo:
#   sub import { my ($pack, @imports)= @_; ... }
#       -> check 'imports' to handle the various
#       'use IDA <params>'  params.

# ... experiment, trying to put 'ea' in caller's namespace.
#      see Scope::Upper  for how to really do this ( in .xs )
#   no strict 'refs';
#   use vars qw($ea);
#   my $caller=caller;
#   local(*{$caller."::ea"}) = \my $ea;


# enum all functions in selection/file
sub forfunctions (&)
{
    my ($code)= @_;
    my ($ea, $eea);
    if (SelStart()==BADADDR) {
        $ea= GetFunctionAttr(FirstSeg(), FUNCATTR_START);
        $ea=NextFunction(FirstSeg()) if ($ea==BADADDR);
        $eea= BADADDR;
    }
    else {
        $ea= GetFunctionAttr(SelStart(), FUNCATTR_START);
        $ea=NextFunction(SelStart()) if ($ea==BADADDR);
        $eea= SelEnd();
    }

    while (($eea==BADADDR || $ea < $eea) && $ea!=BADADDR)
    {
        eval { &{$code}($ea); };
        last if $@;
        $ea=NextFunction($ea);
    }
}
# enumerate all function chunks in selection/file
sub for_chunks (&)
{
    my ($code)= @_;
    my ($ea, $eea);
    if (SelStart()==BADADDR) {
        $ea= GetFchunkAttr(FirstSeg(), FUNCATTR_START);
        $ea=NextFchunk(FirstSeg()) if ($ea==BADADDR);
        $eea= BADADDR;
    }
    else {
        $ea= GetFchunkAttr(SelStart(), FUNCATTR_START);
        $ea=NextFchunk(SelStart()) if ($ea==BADADDR);
        $eea= SelEnd();
    }

    while (($eea==BADADDR || $ea < $eea) && $ea!=BADADDR)
    {
        eval { &{$code}($ea); };
        last if $@;
        $ea=NextFchunk($ea);
    }
}
# enumerate all chunks belonging to specified function
sub for_fchunks($&)
{
    my ($fea, $code)= @_;
    my $ea= FirstFuncFchunk($fea);
    while ($ea!=BADADDR) {
        eval { &{$code}($ea); };
        last if $@;
        $ea= NextFuncFchunk($fea, $ea);
    }
}

# enumerate all non-funcion blocks of code
sub for_not_func (&)
{
#    my ($code)= @_;
#    my ($ea_f, $ea_c, $eea);
#    if (SelStart()==BADADDR) {
#        $ea_f= GetFchunkAttr(FirstSeg(), FUNCATTR_START);
#        $ea_f=NextFchunk(FirstSeg()) if ($ea_f==BADADDR);
#        $ea_c= FirstSeg();
#        $ea_c= NextCode($ea_c) if (GetFlags($ea_c)&MS_CLS)!=FF_CODE;
#        $eea= BADADDR;
#    }
#    else {
#        $ea_f= GetFchunkAttr(SelStart(), FUNCATTR_START);
#        $ea_f=NextFchunk(SelStart()) if ($ea_f==BADADDR);
#        $ea_c= SelStart();
#        $ea_c= NextCode($ea_c) if (GetFlags($ea_c)&MS_CLS)!=FF_CODE;
#        $eea= SelEnd();
#    }
#
#    while (($eea==BADADDR || ($ea_c < $eea && $ea_f < $eea)) && $ea_c!=BADADDR && $ea_f!=BADADDR)
#    {
#        if ($ea_c<$ea_f) {
#            eval { &{$code}($ea_c, $ea_f); };
#            last if $@;
#        }
#        $ea_f= NextFchunk($ea_f);
#        $ea_c= NextCode(GetFchunkAttr($ea_f, FUNCATTR_END));
#
#    }

    # TODO: iterate over code gaps between functions
    # while ($fcur!=BADADDR) {
    #    fcode = NextCode(endof($fcur))
    #    fnext= NextFchunk()
    #    if (fcode!=fnext) &{$code}($fcode);
    #    $fcur=$fnext
    # }
}

# enum all addresses in selection/file
sub foraddrs (&)
{
    my ($code)= @_;

    my ($ea, $eea);
    if (SelStart()==BADADDR) {
        $ea= FirstSeg();
        $eea= BADADDR;
    }
    else {
        $ea= SelStart();
        $eea= SelEnd();
    }

    while (($eea==BADADDR || $ea < $eea) && $ea!=BADADDR)
    {
        eval { &{$code}($ea); }
        last if $@;
        $ea=NextAddr($ea);
    }
}

# enumerate Heads in the selection/file
sub forheads (&)
{
    my ($code)= @_;
    my ($ea, $eea);
    if (SelStart()==BADADDR) {
        $ea= FirstSeg();
        $eea= BADADDR;
    }
    else {
        $ea= SelStart();
        $eea= SelEnd();
    }

    while (($eea==BADADDR || $ea < $eea) && $ea!=BADADDR)
    {
        eval { &{$code}($ea); }
        last if $@;
        $ea=NextHead($ea);
    }
}
sub here ()
{
    return ScreenEA();
}
# forrefs($ea, "DB", { ... }) 
# is like for (local $_=DfirstB($ea) ; $_!=BADADDR ; $_=DnextB($ea,$_)) { ... }
# IDA::forrefs(ScreenEA(), "DB", sub { printf("%08lx\n", $ea); });
sub forrefs ($$&)
{
    my ($rea, $filter, $code)= @_;
    my %reffuncs= (
        "R"=>{first=>\&IDC::Rfirst, next=>\&IDC::Rnext},
        "RB"=>{first=>\&IDC::RfirstB, next=>\&IDC::RnextB},
        "D"=>{first=>\&IDC::Dfirst, next=>\&IDC::Dnext},
        "DB"=>{first=>\&IDC::DfirstB, next=>\&IDC::DnextB},
    );
    my $first= $reffuncs{$filter}{first};
    my $next= $reffuncs{$filter}{next};

    for (my $ea=&{$first}($rea) ; $ea!=BADADDR ; $ea=&{$next}($rea,$ea)) { &{$code}($ea); }
}

# return list of refs
sub refs($$)
{
    my @l;
    forrefs(@_, sub { push @l, shift; });
    return @l;
}
# enum names in selection/file
sub fornames(&) {
    # todo - call ida api
}
# enum segments in selection/file
sub forsegments(&) 
{
    my ($code)= @_;
    my ($ea, $eea);
    if (SelStart()==BADADDR) {
        $ea= FirstSeg();
        $eea= BADADDR;
    }
    else {
        $ea= SegStart(SelStart());
        $eea= SelEnd();
    }

    while (($eea==BADADDR || $ea < $eea) && $ea!=BADADDR)
    {
        eval { &{$code}($ea); }
        last if $@;
        $ea=NextSeg($ea);
    }
}
1;
