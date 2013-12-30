package IDC;
# vim:set ts=4 sw=4:
#
# (C) 2008 Willem Hengeveld  itsme@xs4all.nl
#
# IdcPerl - a perl scripting plugin for the Interactive Disassembler by hex-rays.
# see http://www.xs4all.nl/~itsme/projects/idcperl
#
use strict;
use warnings;

#NOTE: these 2 are includes, not packages
#   'consts' should be included first.
use IDC::consts;
use IDC::macros;

require Exporter;
require DynaLoader;
our @ISA = qw(Exporter DynaLoader);

# export everything
our @EXPORT= keys %IDC::;

our $VERSION = '0.90';
our $AUTOLOAD;
sub AUTOLOAD {
    # remove main:: or IDC:: prefix
    my $idcfunc= substr($AUTOLOAD, rindex($AUTOLOAD, ":")+1);
    return exec_idcfunc($idcfunc, @_);
}
sub SelBegin { return SelStart(); }
bootstrap IDC $VERSION;

1;

