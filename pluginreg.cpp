/* vim:set ts=4 sw=4:
 *
 * (C) 2008 Willem Hengeveld  itsme@xs4all.nl
 *
 * IdcPerl - a perl scripting plugin for the Interactive Disassembler by hex-rays.
 * see http://www.xs4all.nl/~itsme/projects/idcperl
 *
 * this file contains the code to register the plugin with ida,
 *
 */
#include <string>
#include <vector>

// ida includes
#include <pro.h>
#include <ida.hpp>
#include <idp.hpp>
#include <bytes.hpp>
#include <loader.hpp>   // for plugin_t
#include <kernwin.hpp>
#include <diskio.hpp>

// perl includes
#include "extern.h"
#include "perl.h"
#include "xsub.h"
#undef do_open
#undef do_close

#ifdef DYNAMIC_PERL
#include "redefperlasdll.h"
#include "perldllprocs.h"

#include "perldll.h"
#endif

#include "perlinterp.h"
#include "pluginreg.h"
#if IDA_SDK_VERSION>=530
#include "langreg.h"
#endif
#if IDA_SDK_VERSION>=540
#include "clireg.h"
#endif

#ifdef _WITH_CANCEL
#include "perlthread.h"
#endif

#ifdef TRACE_PLUGIN
#define tracemsg msg
#else
#define tracemsg(...)
#endif

#ifdef DYNAMIC_PERL
PerlDll dll;
#endif
// base interpreter, contains autorun code.
Perl interp;
Perl *clonedinterp;

bool g_keepstate= false;

#define PERLERR_IDALOG 0
#define PERLERR_POPUP  1
#define PERLERR_WINDOW 2
ushort g_perlerrors= 0;


int parseversion(const char *versionstr)
{
    const char *dot= strchr(versionstr, '.');
    if (dot==NULL)
        return 0;
    return strtol(versionstr, 0, 10)*100 + strtol(dot+1, 0, 10);
}
int idaapi findmaxversion(const char *name, void *ud)
{
    const char *dash= strstr(name, "idaperl-");
    if (dash==NULL)
        return 0;
    int version= parseversion(dash+8);
    if (version > *(int*)ud)
        *(int*)ud= version;
    return 0;
}
bool is_newest_version()
{
    int maxversion=0;
    enumerate_system_files(NULL, 0, "plugins", "*", findmaxversion, &maxversion);
    int myversion= parseversion(IDAPERL_VERSION);
    return myversion==maxversion;
}
void read_blob(int blobsupid, std::string& buffer)
{
    netnode n(IDAPERLNODE, 0, true);
    buffer.resize(n.blobsize(blobsupid, stag));
    if (buffer.size()) {
        size_t blobsize= buffer.size();
        n.getblob(&buffer[0], &blobsize, blobsupid, stag);
        tracemsg("readblob(%08x), %s\n", blobsupid, buffer.c_str());
    }
}
void save_blob(int blobsupid, const std::string& buffer)
{
    tracemsg("saveblob(%08x), %s\n", blobsupid, buffer.c_str());
    netnode n(IDAPERLNODE, 0, true);
    n.setblob(buffer.c_str(), buffer.size(), blobsupid, stag);
}
bool edit_blob(int blobsupid, const std::string& title, std::string& buffer, const std::string& initialval)
{
    read_blob(blobsupid, buffer);
    if (buffer.empty()) {
        buffer=initialval;
    }
    buffer.resize(65536);
    char *txt= asktext(buffer.size(), &buffer[0], &buffer[0], "%s", title.c_str());
    if (txt) {
        buffer.resize(strlen(txt));
        save_blob(blobsupid, buffer);
        return true;
    }
    else {
        buffer.clear();
        return false;
    }
}
void exec_perlcode(const std::string&perlcode)
{
#ifdef DYNAMIC_PERL
    if (!dll.isloaded())
        return;
#endif

#ifdef CLONE_PERL
    if (!g_keepstate && clonedinterp) {
        delete clonedinterp;
        clonedinterp= NULL;
    }
    if (clonedinterp==NULL) {
        clonedinterp= interp.clone();
    }
#else
    clonedinterp= &interp;
#endif
#ifdef _WITH_CANCEL
    perlthread pt(clonedinterp, perlcode);
    if (pt.wait(10000)
            && pt.cancelwait()) {
        pt.cancel();
    }
#else
    char errbuf[1024];
    if (!clonedinterp->exec(perlcode.c_str(), errbuf, 1024))
        msg("IDAPERL ERROR: %s\n", errbuf);
#endif

}
void exec_autorun()
{
#ifdef DYNAMIC_PERL
    if (!dll.isloaded())
        return;
#endif
    std::string perlcode;
    read_blob(ISUP_AUTORUN, perlcode);

#ifdef _WITH_CANCEL
    perlthread pt(&interp, perlcode);
    if (pt.wait(10000)
            && pt.cancelwait()) {
        pt.cancel();
    }
#else
    char errbuf[1024];
    if (!interp.exec(perlcode.c_str(), errbuf, 1024))
        msg("IDAPERL ERROR: %s\n", errbuf);
#endif
}
void unload_perl()
{
#ifdef DYNAMIC_PERL
    if (!dll.isloaded())
        return;
#endif
#ifdef CLONE_PERL
    if (clonedinterp) {
        delete clonedinterp;
        clonedinterp= NULL;
    }
#endif
    interp.free();
#ifdef DYNAMIC_PERL
    dll.unload();
#endif
}
bool load_perl()
{
#ifdef DYNAMIC_PERL
    if (!dll.load())
        return false;
#endif
    if (!interp.initialize())
        return false;
    msg("idaperl interpreter reinitialized\n");

    exec_autorun();

    return true;
}
Perl *cloneinterp()
{
#ifdef CLONE_PERL
    return interp.clone();
#else
    return &interp;
#endif
}
void destroyclone(Perl *clone)
{
#ifdef CLONE_PERL
    delete clone;
#endif
}
bool read_file(const std::string& filename, std::string& data)
{
    FILE *f= qfopen(filename.c_str(), "rb");
    if (f==NULL) {
        msg("error opening %s\n", filename.c_str());
        return false;
    }
    qfseek(f, 0, SEEK_END);
    size_t fsize= qftell(f);
    if (fsize<=0 || fsize>=0x10000) {
        msg("invalid size: %08lx\n", fsize);
        qfclose(f);
        return false;
    }
    qfseek(f, 0, SEEK_SET);
    data.resize(fsize);
    size_t res= qfread(f, &data[0], data.size());
    qfclose(f);
    if (res!=data.size()) {
        msg("error reading %s\n", filename.c_str());
        return false;
    }
    return true;
}
////////////////////////////////////////////////////
//  saved script management
struct savedscript {
    std::string tag;
    std::string script;
};
typedef std::vector<savedscript> savedscriptlist_t;
savedscriptlist_t saved_scripts;


bool extract_tag(const std::string& perlcode, std::string& tag)
{
    size_t starttag= perlcode.find("#:");
    if (starttag==perlcode.npos)
        return false;
    if (starttag==0 || perlcode[starttag-1]=='\r' || perlcode[starttag-1]=='\n') {
        size_t endtag= perlcode.find_first_of("\r\n", starttag);
        if (starttag+2<perlcode.size()) {
			tag= perlcode.substr(starttag+2, endtag==perlcode.npos?endtag:endtag-starttag-2);
			tracemsg("found tag: %s\n", tag.c_str());
            return true;
        }
    }
	tracemsg("no tag in script\n");
    return false;
}
void load_saved_scripts()
{
	tracemsg("load_saved_scripts\n");
    for (unsigned i=1 ; i<=MAX_SAVEDSCRIPTS ; i++) {
        std::string buffer;
        read_blob(ISUP_SAVED+0x100*(i-1), buffer);
        std::string tag;
        if (extract_tag(buffer, tag)) {
            saved_scripts.resize(i);
            saved_scripts.back().tag= tag;
            saved_scripts.back().script= buffer;
        }
    }
	tracemsg("found %d scripts\n", (int)saved_scripts.size());
}

void savescript(const std::string& tag, const std::string& perlcode)
{
    for (unsigned i=0 ; i<saved_scripts.size() ; i++)
    {
        if (saved_scripts[i].tag==tag) {
            saved_scripts[i].script= perlcode;
            save_blob(ISUP_SAVED+i*0x100, perlcode);
			tracemsg("saved script # %d\n", i);
            return;
        }
    }
    if (saved_scripts.size()==MAX_SAVEDSCRIPTS) {
        msg("not saving script, you can save up to %d scripts\n", MAX_SAVEDSCRIPTS);
        return;
    }

    saved_scripts.resize(saved_scripts.size()+1);
    saved_scripts.back().tag= tag;
    saved_scripts.back().script= perlcode;
    save_blob(ISUP_SAVED+(saved_scripts.size()-1)*0x100, perlcode);
	tracemsg("now total %d saved scripts\n", (int)saved_scripts.size());
}



////////////////////////////////////////////////////
// ida event handlers

// menu+run handler
bool idaapi run_immediate(void*)
{
    std::string perlcode;
    if (edit_blob(ISUP_MANUAL, "enter perl code", perlcode, 
            "use warnings;\n"
            "use strict;\n"
            "use IDC;\n"))
    {
        exec_perlcode(perlcode);

        std::string tag;
        if (extract_tag(perlcode, tag))
            savescript(tag, perlcode);

        return true;
    }
    return false;
}

// menu handler
bool idaapi pick_script(void*)
{
	if (saved_scripts.empty())
		return false;
    std::string layout= "choose perl script\n";

    layout += "scripts\n";

    for (unsigned i=0 ; i<saved_scripts.size() ; i++)
    {
        layout += "<";

        layout += saved_scripts[i].tag;

		layout += ":R";
        if (i+1==saved_scripts.size())  // the last item
            layout += ">";

        layout += ">\n";
    }

	layout +=
"wrapper\n"
"<none:R>        <e~x~ec:r>\n"
"<heads:R>       <~e~dit:r>\n"
"<addrs:R>       <~d~elete:r>>\n"
"<functions:R>\n"
"<fchunks:R>>\n";

    /*  */  static int scriptid=0;
	/*not static*/ int action=0;
	/*  */  static int wrapper=0;

    int ok= AskUsingForm_c(layout.c_str(), &scriptid, &action, &wrapper);
    if (!ok)
        return false;

    std::string code= saved_scripts[scriptid].script;
	if (action==1) {
		tracemsg("editting before exec\n");
        // todo: add wrapper when editting
		save_blob(ISUP_MANUAL, code);
		return run_immediate(NULL);
	}
	else if (action==2) {
		saved_scripts.erase(saved_scripts.begin()+scriptid);
		for (unsigned i=scriptid ; i<saved_scripts.size() ; i++) {
			save_blob(ISUP_SAVED+i*0x100, saved_scripts[i].script);
		}
		save_blob(ISUP_SAVED+saved_scripts.size()*0x100, "");
	}
    else {
		if (wrapper) {
			std::string firstfn, nextfn, endfn;
			switch(wrapper) {
				case 0: break;
				case 1: nextfn="NextHead($ea,$__end)"; firstfn="ScreenEA()"; endfn="$ea==ScreenEA()"; break;
				case 2: nextfn="NextAddr($ea)"; firstfn="ScreenEA()"; endfn="$ea<ScreenEA()+ItemSize(ScreenEA())"; break;
				case 3: nextfn="NextFunction($ea)";    firstfn="FirstSeg()"; endfn="1"; break;
				case 4: nextfn="NextFuncFchunk(ScreenEA(), $ea)"; firstfn="FirstFuncFchunk(ScreenEA())"; endfn="1"; break;
			}
			code= 
"use IDC;"
"use IDA;"
"my ($__begin, $__end)= (SelBegin(), SelEnd());"
"for (my $ea=($__begin!=BADADDR)?$__begin:"+firstfn+"  ; $ea!=BADADDR && (($__begin!=BADADDR) ? $ea<$__end : "+endfn+") ; $ea="+nextfn+") {\n"
+code
+"}";
;
		}
		exec_perlcode(code);
	}
    return true;
}
#ifdef __NT__
const char *scriptwildcard="\\*.pl";
#else
const char *scriptwildcard="/*.pl";
#endif
std::string g_scriptdir= idadir("perl");


// menu+run handler
bool idaapi run_file(void *)
{
    const char *perlfile= askfile_c(0, (g_scriptdir+scriptwildcard).c_str(), "select perl script");
    if (perlfile==NULL)
        return false;
    std::string perlcode;
    if (!read_file(perlfile, perlcode))
        return false;
    size_t slash= std::string(perlfile).find_last_of("/\\");
    if (slash!=std::string::npos)
        g_scriptdir=std::string(perlfile).substr(0, slash);

    exec_perlcode(perlcode);

    return true;
}

// menu+run handler
bool idaapi reset_interpreter(void*)
{
#ifdef DYNAMIC_PERL
    if (!dll.isloaded())
      return false;
#endif
    unload_perl();
    load_perl();

    return false;
}
std::string gethotkeyname(short hotkey)
{
    std::string keyname;
    if (hotkey&0x8000) { if (!keyname.empty()) keyname+="+"; keyname+="alt"; }

    if (!keyname.empty()) keyname+="-"; 
    int c= (hotkey&0xff);
    if ((c>='0' && c<='9') || (c>='A' && c<='Z'))
        keyname += c;
    else if (c>=0x70 && c<=0x87) {
        char fbuf[8];
        qsnprintf(fbuf, 8, "F%d", c-0x6f);
        keyname += fbuf;
    }
    else {
        char fbuf[8];
        qsnprintf(fbuf, 8, "%02x", c);
        keyname += fbuf;
    }
    return keyname;
}
// menu+run handler
bool idaapi about_idaperl(void *)
{
    static std::string hotkeyname;
    if (hotkeyname.empty())
    {
        for (plugin_info_t *pi= get_plugins() ; pi ; pi=pi->next)
        {
            if (pi->entry==&PLUGIN) {
                msg("%p: %04x %04x %2d F=%x p='%s' on='%s' n='%s'\n",
                    pi, pi->org_hotkey, pi->hotkey, pi->arg, pi->flags, pi->path, pi->org_name, pi->name);
                hotkeyname= gethotkeyname(pi->hotkey);
                break;
            }
        }
    }
    msg("IDAPERL, by Willem Jan Hengeveld, itsme@xs4all.nl\n"
        " %s  : execute immediate perl\n"
        " alt-3  : execute perl file\n"
        " alt-4  : reset perl interpreter\n"
        " alt-5  : pick recent immediate perl\n"
        " alt-6  : help\n", hotkeyname.c_str());
    return false;
}
// ida edit callback
void idaapi autorun_edit(TView *[],int)
{
    std::string perlbuf;
    edit_blob(ISUP_AUTORUN, "enter perl autorun code", perlbuf, "");
}
// menu handler
bool idaapi config_idaperl(void *)
{
#ifdef DYNAMIC_PERL
    if (!dll.isloaded())
      return false;
#endif
    const char*layout=
"IDAPerl Configuration\n"

"<keep perl state:C>>\n"

"<##perl errors##idalog:R>\n"
"<popup:R>\n"
"<seperate window:R>>\n"

"<autorun:B::::>\n";

    ushort checkboxes= (g_keepstate?1:0);
    int ok= AskUsingForm_c(layout, &checkboxes, &g_perlerrors, autorun_edit);
    if (ok)
        g_keepstate= (checkboxes&1)!=0;
    return false;
}

//--------------------------------------------------------------------------
//
//      Initialize.
//
//      IDA will call this function only once.
//      If this function returns PLGUIN_SKIP, IDA will never load it again.
//      If this function returns PLUGIN_OK, IDA will unload the plugin but
//      remember that the plugin agreed to work with the database.
//      The plugin will be loaded again if the user invokes it by
//      pressing the hotkey or selecting it from the menu.
//      After the second load the plugin will stay on memory.
//      If this function returns PLUGIN_KEEP, IDA will keep the plugin
//      in the memory. In this case the initialization function can hook
//      into the processor module and user interface notification points.
//      See the hook_to_notification_point() function.
//
//      In this example we check the input file format and make the decision.
//      You may or may not check any other conditions to decide what you do:
//      whether you agree to work with the database or not.
//
int idaapi init(void)
{
//  if ( inf.filetype == f_ELF ) return PLUGIN_SKIP;

  tracemsg("plugin init\n");
  // prevent accidental activation of multiple versions
//if (!is_newest_version()) {
//    msg("IDCPERL: found multiple versions, only loading latest\n");
//    return PLUGIN_SKIP;
//}
  if (!load_perl())
    return PLUGIN_SKIP;

  add_menu_item("File/IDC Command", "Run P~e~rl Script", "Alt-3", SETMENU_APP, run_file, 0);
  add_menu_item("File/IDC Command", "Pick P~e~rl Script", "Alt-5", SETMENU_APP, pick_script, 0);
  add_menu_item("File/IDC Command", "Reset Perl ~I~nterpreter", "Alt-4", SETMENU_APP, reset_interpreter, 0);
  add_menu_item(
#ifdef _WIN32
          "Help/External help",
#else
          "Help/Check for free update",
#endif
          "IDAPerl Help", "Alt-6", SETMENU_APP, about_idaperl, 0);
  add_menu_item("Options/Font", "IDAPerl Options", "", SETMENU_APP, config_idaperl, 0);

#if IDA_SDK_VERSION>=530
  register_language();
#endif
#if IDA_SDK_VERSION>=540
  register_cli();
#endif

  msg("\n"
      "IDAPERL version %s\n"
      "by Willem Jan Hengeveld, email: itsme@xs4all.nl\n"
      "www: http://www.xs4all.nl/~itsme/\n", IDAPERL_VERSION);

// Please uncomment the following line to see how the notification works
//  hook_to_notification_point(HT_UI, sample_callback, NULL);

// Please uncomment the following line to see how the user-defined prefix works
//  set_user_defined_prefix(prefix_width, get_user_defined_prefix);

  load_saved_scripts();

  return PLUGIN_KEEP;
}

//--------------------------------------------------------------------------
//      Terminate.
//      Usually this callback is empty.
//      The plugin should unhook from the notification lists if
//      hook_to_notification_point() was used.
//
//      IDA will call this function when the user asks to exit.
//      This function won't be called in the case of emergency exits.

void idaapi term(void)
{
#ifdef DYNAMIC_PERL
  if (!dll.isloaded())
    return;
#endif
  tracemsg("plugin term\n");

#if IDA_SDK_VERSION>=530
  deregister_language();
#endif
#if IDA_SDK_VERSION>=540
  deregister_cli();
#endif


  //unhook_from_notification_point(HT_UI, sample_callback);
  //set_user_defined_prefix(0, NULL);
  unload_perl();

  msg("idaperl-%s terminated\n", IDAPERL_VERSION);
}


//--------------------------------------------------------------------------
//
//      The plugin method
//
//      This is the main function of plugin.
//
//      It will be called when the user selects the plugin.
//
//              arg - the input argument, it can be specified in
//                    plugins.cfg file. The default is zero.
//
//
void idaapi run(int arg)
{
#ifdef DYNAMIC_PERL
  if (!dll.isloaded())
    return;
#endif

  tracemsg("plugin run(%d)\n", arg);
  switch(arg)
  {
      case 0: run_immediate(NULL); break;
      case 1: run_file(NULL); break;
      case 2: reset_interpreter(NULL); break;
      case 3: about_idaperl(NULL); break;
  }
}

//--------------------------------------------------------------------------
char comment[] = "runs perl scripts";

char help[] =
    "no help yet\n";


//--------------------------------------------------------------------------
// This is the preferred name of the plugin module in the menu system
// The preferred name may be overriden in plugins.cfg file

char wanted_name[] = "idaperl";


// This is the preferred hotkey for the plugin module
// The preferred hotkey may be overriden in plugins.cfg file
// Note: IDA won't tell you if the hotkey is not correct
//       It will just disable the hotkey.

char wanted_hotkey[] = "Alt-2";


//--------------------------------------------------------------------------
//
//      PLUGIN DESCRIPTION BLOCK
//
//--------------------------------------------------------------------------
plugin_t PLUGIN =
{
  IDP_INTERFACE_VERSION,
  0,                    // plugin flags
  init,                 // initialize

  term,                 // terminate. this pointer may be NULL.

  run,                  // invoke plugin

  comment,              // long comment about the plugin
                        // it could appear in the status line
                        // or as a hint

  help,                 // multiline help about the plugin

  wanted_name,          // the preferred short name of the plugin
  wanted_hotkey         // the preferred hotkey to run the plugin
};
