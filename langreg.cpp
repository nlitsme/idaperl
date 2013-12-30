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
#include <expr.hpp>  // for extlang_t
#include <kernwin.hpp> // for msg
#ifdef _MSC_VER
#pragma warning(disable:4700)
#pragma warning(disable:4100)
#pragma warning(disable:4189)
#endif



#include "langreg.h"
#include "perlinterp.h"
#include "pluginreg.h"  // todo: move 'cloneinterp' to a 'interpreter mgr'
std::string argstring(int nargs, const idc_value_t args[]);

#ifdef TRACE_LANG
#define tracemsg msg
#else
#define tracemsg(...)
#endif


Perl *dbginterp;

std::string create_sub(const std::string& name, const std::string& expr)
{
    return std::string("sub ")+name+" {"+expr+"}";
}
std::string create_do(const char *filename)
{
    std::string docode= "do '";
    for (const char *p= filename ; *p ; p++)
    {
        if (*p=='/' || *p=='\\')
            docode+= '/';
        else if (*p=='\'') {
            docode+= '\\';
            docode+= '\'';
        }
        else {
            docode+= *p;
        }
    }
    docode += '\'';
    docode += ';';
    return docode;
}

// called with name='_IDC_TMP' when entering a breakpt expression
// or sprintf('_IDC_%X', ea)  when starting a program.
bool idaapi expr_compile(       // Compile an expression
        const char *name,       // in: name of the function which will
                                //     hold the compiled expression
        ea_t current_ea,        // in: current address. if unknown then BADADDR
        const char *expr,       // in: expression to compile
        char *errbuf,           // out: error message if compilation fails
        size_t errbufsize)      // in: size of the error buffer
                                // Returns: success
{
    tracemsg("compile: %08x '%s': '%s' -> %d\n",
            (int)current_ea, name, expr, (int)errbufsize);

    if (errbuf) errbuf[0]=0;
    return dbginterp->exec(create_sub(name, expr).c_str(), errbuf, errbufsize);
}


bool idaapi expr_run(           // Evaluate a previously compiled expression
        const char *name,       // in: function to run
        int nargs,              // in: number of input arguments
        const idc_value_t args[], // in: input arguments
        idc_value_t *result,    // out: function result
        char *errbuf,           // out: error message if evaluation fails
        size_t errbufsize)      // in: size of the error buffer
                                // Returns: success
{
    if (errbuf) errbuf[0]=0;
    return dbginterp->run(name, nargs, args, result, errbuf, errbufsize);
}

// .. called when you 'mouse-over' a disassembly item
// or fromt the '?' manual expr eval
// or from the values entered in an array definition.
bool idaapi expr_calc(      // Compile and evaluate expression
        ea_t current_ea,        // in: current address. if unknown then BADADDR
        const char *expr,       // in: expression to evaluation
        idc_value_t *rv,        // out: expression value
        char *errbuf,           // out: error message if evaluation fails
        size_t errbufsize)      // in: size of the error buffer
                                // Returns: success
{
    tracemsg("calc: %08x '%s' -> %d\n",
            (int)current_ea, expr, (int)errbufsize);
    if (errbuf) errbuf[0]=0;
    if (!dbginterp->exec(create_sub("__idcperl_calc", expr).c_str(), errbuf, errbufsize))
        return false;
    return dbginterp->run("__idcperl_calc", 0, NULL, rv, errbuf, errbufsize);
}


bool idaapi expr_compile_file(  // Compile (load) a file
        const char *file,       // file name
        char *errbuf,           // out: error message if compilation fails
        size_t errbufsize)      // in: size of the error buffer
{
    tracemsg("compile_file: '%s' -> %d\n", file, (int)errbufsize);
    if (errbuf) errbuf[0]=0;

    if (!dbginterp->exec(create_do(file).c_str(), errbuf, errbufsize))
        return false;
    return true;
}

// todo: implement these
// ?? what are they used for by ida?
bool idaapi expr_create_object( // Create an object instance
      const char *name,       // in: object class name
      int nargs,              // in: number of input arguments
      const idc_value_t args[], // in: input arguments
      idc_value_t *result,    // out: created object or exception
      char *errbuf,           // out: error message if evaluation fails
      size_t errbufsize)      // in: size of the error buffer
                              // Returns: success
{
    tracemsg("createobj: %s (%s) -> %p\n", name, argstring(nargs, args).c_str(), result);
    *result= NULL;
    return true;
}
bool idaapi expr_get_attr(      // Returns the attribute value of a given object from the global scope
      const idc_value_t *obj, // in: object (may be NULL)
      const char *attr,       // in: attribute name
      idc_value_t *result) 
                              // Returns: success
{
    tracemsg("getattr: %p.%s -> %p\n", obj, attr, result);
    return true;
}

bool idaapi expr_set_attr(      // Sets the attribute value of a given object in the global scope
      idc_value_t *obj,       // in: object (may be NULL)
      const char *attr,       // in: attribute name
      idc_value_t *value) 
                              // Returns: success
{
    tracemsg("setattr: %p.%s = %s\n", obj, attr, argstring(1, value).c_str());
    return true;
}

bool idaapi expr_call_method(   // Calls a member function
      const idc_value_t *obj,     // in: object instance
      const char *name,           // in: method name to call
      int nargs,                  // in: number of input arguments
      const idc_value_t args[],   // in: input arguments
      idc_value_t *result,        // out: function result or exception
      char *errbuf,               // out: error message if evaluation fails
      size_t errbufsize)          // in: size of the error buffer
{
    tracemsg("callmethod: %p.%s (%s) -> %p\n", obj, name, argstring(nargs, args).c_str(), result);
    return true;
}

bool idaapi expr_run_statements(// Compile and execute a string with statements
                                // (see also: calcexpr() which works with expressions)
    const char *str,            // in: input string to execute
    char *errbuf,               // out: error message
    size_t errbufsize)          // in: size of the error buffer
{
    tracemsg("runstatements '%s', %p:%d\n", str, errbuf, (int)errbufsize);

    if (errbuf) errbuf[0]=0;
    if (!dbginterp->exec(create_sub("__idcperl_stmt", str).c_str(), errbuf, errbufsize))
        return false;
    return dbginterp->run("__idcperl_stmt", 0, NULL, NULL, errbuf, errbufsize);
}



extlang_t g_el;
void register_language()
{
    dbginterp= cloneinterp();

    g_el.size= sizeof(extlang_t);
    g_el.flags= 0;
    g_el.name= "idcperl";
    g_el.compile= expr_compile;
    g_el.run= expr_run;
    g_el.calcexpr= expr_calc;
#if IDA_SDK_VERSION>=540
    g_el.compile_file= expr_compile_file;
    g_el.fileext= "pl";
#if IDA_SDK_VERSION>=570
    g_el.create_object= expr_create_object;
    g_el.get_attr= expr_get_attr;
    g_el.set_attr= expr_set_attr;
    g_el.call_method= expr_call_method;

#if IDA_SDK_VERSION>=620
    g_el.run_statements= expr_run_statements;
#endif
#endif
#endif
    install_extlang(&g_el);
    select_extlang(&g_el);

    tracemsg("perl extlang installed\n");
}
void deregister_language()
{
    delete extlang;
    remove_extlang(&g_el);

    destroyclone(dbginterp);
    dbginterp=NULL;
}

