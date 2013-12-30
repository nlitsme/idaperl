IDAPerl
=======

A perl interface for IDA

IDAPerl, is a plugin for IDA Pro, which adds perl scripting support to ida.

Author: Willem Jan Hengeveld itsme@xs4all.nl
history

* 2008-05-09, version 0.1 - initial release
* 2008-05-12, version 0.2 - simplified interface to IDC, now all IDC functions are implemented, now works on OSX, and probably other platforms too.
* version 0.3 - fixed memory leak.
* 2008-05-14, version 0.4 - redirecting stdout/stderr to ida window, added idaperl to several menu's, added 'autorun', options dialog. optionally using new interpreter for each script. syntax errors are now displayed in the ida window
* 2008-06-09, version 0.5 - scripts are interruptable, first stable version
* 2011-03-24, version 0.6 - ida60, osx support
* 2013-12-13, version 0.7 - ida 6.5 support, release on github


note: I am not actively developping this plugin anymore, since i myself now mostly use idapython for scripting.

Requirements
============

this plugin is known to work with ida5.2-ida6.5, and activesync perl 5.8.8, OSX perl 5.12, other versions have not yet been tested.
on windows: make sure perl58.dll is in your searchpath.

Install
=======

run: make install

how to use
==========

type 'alt-2' to get the idaperl window, and enter for example:

    use strict;
    use warnings;
    use IDC;

    sub test123 { Msg("test123 called\n"); }
    DelHotkey("Shift-I");
    AddHotkey("Shift-I", "test123");

click 'OK' now type 'shift-I' and you will see 'test123 called' be printed in the ida message window
other ida scripting extensions


Other IDA Scripting languages
=============================

* https://code.google.com/p/idapython/
* http://www.wasm.ru/pub/23/files/perl_src.zip
  by redplait, not updated since 2004, for perl5.5, ida4.7
* https://github.com/spoonm/idarub


future plans
============

* provide UI to manage code snippets
  since ida64, something like this is integrated in IDA
* provide UI to manage key bindings
* provide several wrappers ( like the perl '-n' option ), for instance, the wrapper below would allow code to easily operate on the current, or selection

    use IDC;
    for (local $_=(SelBegin()!=BADADDR)?SelBegin():ScreenEA() ; 
                  (SelEnd()!=BADADDR)?$_<SelEnd():$_==ScreenEA() ; 
               $_=(SelEnd()!=BADADDR)?NextHead($_, SelEnd()):BADADDR)
    {
        # CODE
    }

* add support to interact with ida, from a perl script running outside of ida.
* create perl objects for ida items, like functions, segments, structs, etc.
* build library of idaperl modules under idadir\plugins\idaperl
* add method of interrupting a long running script: probably i would need to have the perlinterpreter execute in a different thread, while the ida-ui thread just handles the dialog box.
* add option to convert .idc to .pl
* hook ida notification points


* hook notification points
    kernwin.hpp : ui_notification_t
    dbg.hpp     : dbg_event_code_t
    idp.hpp     : event_code_t 
    idp.hpp     : idp_notify 
    intel.hpp   : event_codes_t 
    graph.hpp   : graph_notification_t 
    dbg.hpp     : dbg_notification_t 

* create perl objects for ida items, like functions, segments, structs, etc.

* build library of idaperl modules under idadir\plugins\idaperl
* add option to convert .idc code to .pl
* find way of moving data from a cloned interpreter, back to the parent.
* implement 'locals' to import registers and local vars during debugging,
      using get_reg_val, get_name_value, get_stkvar
* implement 'for_not_func(sub { MakeFunction(@_); Wait(); })'
* ida5.4: see kernwin.hpp, add cli_t support, install_command_interpreter
* for the cli_t / expression eval, make sure 'use IDC' is always executed.  ( for debugger breakpoints conditions )


adding 'ctrl-c' support
-----------------------

several possibilities:
* run perl code in seperate thread, the main idathread only has a dialogbox with a 'cancel' button.
* check if there is some ida-ui event that can be used to call a perl function, aborting the interpreter
* run perl in the main thread, and the dialog box in a seperate thread.

main problem seems to be interrupting perl.
  'kill' kills the whole process.
  'threads->kill' in a thread kills only that thread.

some ideas:

$SIG{'KILL'}= sub { die "script killed\n"; }

-- idathread:dialog, perlthread:exec
'die "script killed"'  from the idathread -> crash ( recursive exceptions called )



run externally
--------------

it should also be possible to run perl scripts from the commandline,
which then find the correct IDA instance, and interact with it.

probably the best way to identify which ida instance to talk to, is by .idb file, and then find which 
idag.exe has it open.

but if there is only one idag running, just take that instance.

something like this:
 use IDA remote=>'test.idb';


ChangeLog
=========

issues in version 0.1, which were solved in 0.2
-----------------------------------------------

* Message currently does not work, instead use Msg, which takes a single string as parameter.
  to output formatted strings currently you would have to type:

    Msg(sprintf("%08lx: %s\n", 123, 'abc'));

* implement all remaining idc functions
* not every function works, still need to do a lot of testing.
  * tst.ipl will give you an idea of what functions are known to work.
  * idc_perl.xs shows what functions have been implemented.
  also all macro's defined in idc.idc have been implemented/defined here


issues in version 0.2, which were solved in 0.3
-----------------------------------------------

* fix memory leaks

issues in version 0.3, which were solved in 0.4
-----------------------------------------------

* now printing to STDOUT, and STDERR ( using 'print', warn, etc. ) is redirected to the ida message log
* optionally support perl threads, by serializing access to ida.
* added 'run perl file', 'reset perl', 'about', 'options' menu entries
* implemented autorun
* added options dialog
* optionally using new interpreter for each manually entered perl script.
* IDA.pm, containing some iteration helpers.
* bugfix: idc function calls no longer destroy regex results like '$1'
* bugfix: now syntax errors are displayed correctly



