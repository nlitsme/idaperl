#!/usr/bin/perl
# vim:set ts=4 sw=4:
#
# (C) 2008 Willem Hengeveld  itsme@xs4all.nl
#
# IdcPerl - a perl scripting plugin for the Interactive Disassembler by hex-rays.
# see http://www.xs4all.nl/~itsme/projects/idcperl
#
use strict;

# script to generate from this:
# perldllprocs.h:PERLDECL I32 (*Perl_call_pv)(pTHX_ const char*, I32);
# these 2 files:
# perldllprocnames.inc:    {"Perl_call_pv", (FARPROC*)&Perl_call_pv},
# redefperlasdll.h:#define Perl_call_pv dll_Perl_call_pv

my @ifdefs;
my $cmtline;
my ($infile, $incfile, $deffile)= @ARGV;

# this file defines or declares function pointers to all perl functions
# defining PERLDEFINE prior to including this file, defines all function pointers
open(IN, "<", "$infile") or die "$infile: $!\n";

# this file contains the contents of the dynamic perl function table
open(INC, ">", "$incfile") or die "$incfile: $!\n";

# this file renames all perl funcs to dll_<name>
# so we can declare them as function pointers, and load perl.dll dynamically
open(DEF, ">", "$deffile") or die "$deffile: $!\n";
print DEF "/* NOTE: this file is generated with gendllcode.pl */\n";
print INC "/* NOTE: this file is generated with gendllcode.pl */\n";
print DEF "#ifndef _REDEFPERLASDLL_H_\n";
print DEF "#define _REDEFPERLASDLL_H_\n";
while (<IN>) {
    if (/#ifndef\s+(?:_\w+_H_|PERLDEFINE)/) {
        # skip
        push @ifdefs, 0;
    }
    elsif (/#ifdef\s+__cplusplus/) {
        # skip
        push @ifdefs, 0;
    }
    elsif (/#if/) {
        push @ifdefs, 1;
        printf INC $_;
        printf DEF $_;
    }
    elsif (/#define/) {
        # skip
    }
    elsif (/[{}]/) {
        # skip
    }
    elsif (/PERLDECL\s+\S+\s*\(\*(\w+)\)/) {
        printf INC "    {\"%s\", (FARPROC*)&%s},\n", $1, $1;
        printf DEF "#define %s dll_%s\n", $1, $1;
    }
    elsif (/#el/) {
        if ($ifdefs[-1]) {
            printf INC $_;
            printf DEF $_;
        }
    }
    elsif (/#end/) {
        if ($ifdefs[-1]) {
            printf INC $_;
            printf DEF $_;
        }
        pop @ifdefs;
    }
    elsif (/#inc/) {
        printf DEF $_;
    }
    elsif (/^ \*\s+\S/) {
        if ($cmtline++<3) {
            printf INC $_;
            printf DEF $_;
        }
    }
    else {
        printf INC $_;
        printf DEF $_;
    }
}
print DEF "#endif\n";
print INC "    {\"\", NULL},\n";
close IN;
close INC;
close DEF;
