/* vim:set ts=4 sw=4:
 *
 * (C) 2008 Willem Hengeveld  itsme@xs4all.nl
 *
 * IdcPerl - a perl scripting plugin for the Interactive Disassembler by hex-rays.
 * see http://www.xs4all.nl/~itsme/projects/idcperl
 *
 */
#include <string>

// ida includes
#include <pro.h>     // for basic types.
#include <ida.hpp>   // for ida constants and types.
#include <expr.hpp>  // 
#include <kernwin.hpp> // for msg, cli_t
#ifdef _MSC_VER
#pragma warning(disable:4700)
#pragma warning(disable:4100)
#pragma warning(disable:4189)
#endif



#include "clireg.h"
#include "perlinterp.h"
#include "pluginreg.h"  // todo: move 'cloneinterp' to a 'interpreter mgr'

#ifdef TRACE_CLI
#define tracemsg msg
#else
#define tracemsg(...)
#endif


Perl *cliinterp;

cli_t *registered_cli;

bool idaapi cli_execute(const char *line)
{
    tracemsg("cli: '%s'\n", line);
    std::string errbuf; errbuf.resize(1024);
    if (!cliinterp->exec(line, &errbuf[0], errbuf.size())) {
        msg("%s\n", errbuf.c_str());

        // todo: add support for multiline perl: first only syntax check, return false until syntax is ok.
    }
    return true;
}

void register_cli()
{
    cli_t *cl= new cli_t();

    cliinterp= cloneinterp();

    cl->size= sizeof(cli_t);
    cl->flags= 0;
    cl->sname= "perl";
    cl->lname= "perl";
    cl->hint= "enter a perl expression";
    cl->execute_line= cli_execute;
    cl->complete_line= NULL;
    cl->keydown= NULL;

    install_command_interpreter(cl);
    registered_cli= cl;
}
void deregister_cli()
{
    if (registered_cli) {
        remove_command_interpreter(registered_cli);
        registered_cli= NULL;
    }

    destroyclone(cliinterp);
    cliinterp= NULL;
}

