/* vim:set ts=4 sw=4:
 *
 * (C) 2008 Willem Hengeveld  itsme@xs4all.nl
 *
 * IdcPerl - a perl scripting plugin for the Interactive Disassembler by hex-rays.
 * see http://www.xs4all.nl/~itsme/projects/idcperl
 *
 */
#ifndef __IDCHOTKEY_H__
#define __IDCHOTKEY_H__
class hotkey {
private:
    std::string _hotkey;
    std::string _perlthunk;
    std::string _idcthunk;
    uchar *_thunkcode;
public:
    hotkey();
    ~hotkey();

    bool add(const std::string&key, const std::string&func);
    void del();
};

#endif
