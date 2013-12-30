# (C) 2008 Willem Hengeveld  itsme@xs4all.nl
# this module defines all macro functions as defined in idc.idc
sub hasValue {
    my ($F)= @_;
    return (($F & FF_IVL) != 0);
}

sub byteValue {
    my ($F)= @_;
    return ($F & MS_VAL);
}

sub isLoaded {
    my ($ea)= @_;
    return hasValue(GetFlags($ea));
}

sub isCode {
    my ($F)= @_;
    return (($F & MS_CLS) == FF_CODE);
}

sub isData {
    my ($F)= @_;
    return (($F & MS_CLS) == FF_DATA);
}

sub isTail {
    my ($F)= @_;
    return (($F & MS_CLS) == FF_TAIL);
}

sub isUnknown {
    my ($F)= @_;
    return (($F & MS_CLS) == FF_UNK);
}

sub isHead {
    my ($F)= @_;
    return (($F & FF_DATA) != 0);
}

sub isFlow {
    my ($F)= @_;
    return (($F & FF_FLOW) != 0);
}

sub isVar {
    my ($F)= @_;
    return (($F & FF_VAR ) != 0);
}

sub isExtra {
    my ($F)= @_;
    return (($F & FF_LINE) != 0);
}

sub isRef {
    my ($F)= @_;
    return (($F & FF_REF) != 0);
}

sub hasName {
    my ($F)= @_;
    return (($F & FF_NAME) != 0);
}

sub hasUserName {
    my ($F)= @_;
    return (($F & FF_ANYNAME) == FF_NAME);
}

sub isDefArg0 {
    my ($F)= @_;
    return (($F & MS_0TYPE) != FF_0VOID);
}

sub isDefArg1 {
    my ($F)= @_;
    return (($F & MS_1TYPE) != FF_1VOID);
}

sub isDec0 {
    my ($F)= @_;
    return (($F & MS_0TYPE) == FF_0NUMD);
}

sub isDec1 {
    my ($F)= @_;
    return (($F & MS_1TYPE) == FF_1NUMD);
}

sub isHex0 {
    my ($F)= @_;
    return (($F & MS_0TYPE) == FF_0NUMH);
}

sub isHex1 {
    my ($F)= @_;
    return (($F & MS_1TYPE) == FF_1NUMH);
}

sub isOct0 {
    my ($F)= @_;
    return (($F & MS_0TYPE) == FF_0NUMO);
}

sub isOct1 {
    my ($F)= @_;
    return (($F & MS_1TYPE) == FF_1NUMO);
}

sub isBin0 {
    my ($F)= @_;
    return (($F & MS_0TYPE) == FF_0NUMB);
}

sub isBin1 {
    my ($F)= @_;
    return (($F & MS_1TYPE) == FF_1NUMB);
}

sub isOff0 {
    my ($F)= @_;
    return (($F & MS_0TYPE) == FF_0OFF);
}

sub isOff1 {
    my ($F)= @_;
    return (($F & MS_1TYPE) == FF_1OFF);
}

sub isChar0 {
    my ($F)= @_;
    return (($F & MS_0TYPE) == FF_0CHAR);
}

sub isChar1 {
    my ($F)= @_;
    return (($F & MS_1TYPE) == FF_1CHAR);
}

sub isSeg0 {
    my ($F)= @_;
    return (($F & MS_0TYPE) == FF_0SEG);
}

sub isSeg1 {
    my ($F)= @_;
    return (($F & MS_1TYPE) == FF_1SEG);
}

sub isEnum0 {
    my ($F)= @_;
    return (($F & MS_0TYPE) == FF_0ENUM);
}

sub isEnum1 {
    my ($F)= @_;
    return (($F & MS_1TYPE) == FF_1ENUM);
}

sub isFop0 {
    my ($F)= @_;
    return (($F & MS_0TYPE) == FF_0FOP);
}

sub isFop1 {
    my ($F)= @_;
    return (($F & MS_1TYPE) == FF_1FOP);
}

sub isStroff0 {
    my ($F)= @_;
    return (($F & MS_0TYPE) == FF_0STRO);
}

sub isStroff1 {
    my ($F)= @_;
    return (($F & MS_1TYPE) == FF_1STRO);
}

sub isStkvar0 {
    my ($F)= @_;
    return (($F & MS_0TYPE) == FF_0STK);
}

sub isStkvar1 {
    my ($F)= @_;
    return (($F & MS_1TYPE) == FF_1STK);
}

sub isByte {
    my ($F)= @_;
    return (isData($F) && ($F & DT_TYPE) == FF_BYTE);
}

sub isWord {
    my ($F)= @_;
    return (isData($F) && ($F & DT_TYPE) == FF_WORD);
}

sub isDwrd {
    my ($F)= @_;
    return (isData($F) && ($F & DT_TYPE) == FF_DWRD);
}

sub isQwrd {
    my ($F)= @_;
    return (isData($F) && ($F & DT_TYPE) == FF_QWRD);
}

sub isOwrd {
    my ($F)= @_;
    return (isData($F) && ($F & DT_TYPE) == FF_OWRD);
}

sub isTbyt {
    my ($F)= @_;
    return (isData($F) && ($F & DT_TYPE) == FF_TBYT);
}

sub isFloat {
    my ($F)= @_;
    return (isData($F) && ($F & DT_TYPE) == FF_FLOAT);
}

sub isDouble {
    my ($F)= @_;
    return (isData($F) && ($F & DT_TYPE) == FF_DOUBLE);
}

sub isPackReal {
    my ($F)= @_;
    return (isData($F) && ($F & DT_TYPE) == FF_PACKREAL);
}

sub isASCII {
    my ($F)= @_;
    return (isData($F) && ($F & DT_TYPE) == FF_ASCI);
}

sub isStruct {
    my ($F)= @_;
    return (isData($F) && ($F & DT_TYPE) == FF_STRU);
}

sub isAlign {
    my ($F)= @_;
    return (isData($F) && ($F & DT_TYPE) == FF_ALIGN);
}

sub rotate_dword {
    my ($x, $count)= @_;
    return rotate_left($x, $count, 32, 0);
}

sub rotate_word {
    my ($x, $count)= @_;
    return rotate_left($x, $count, 16, 0);
}

sub rotate_byte {
    my ($x, $count)= @_;
    return rotate_left($x, $count, 8, 0);
}

sub EVAL_FAILURE {
    my ($code)= @_;
    return (IsString($code) && substr($code, 0, 13) == "IDC_FAILURE: ");
}

sub AutoMark {
    my ($ea,$qtype)= @_;
    return AutoMark2($ea,$ea+1,$qtype);
}

sub GetFloat {
    my ($ea)= @_;
    return GetFpNum($ea, 4);
}

sub GetDouble {
    my ($ea)= @_;
    return GetFpNum($ea, 8);
}

sub STRTERM1 {
    my ($strtype)= @_;
    return (($strtype>>8)&0xFF);
}

sub STRTERM2 {
    my ($strtype)= @_;
    return (($strtype>>16)&0xFF);
}

sub SetPrcsr {
    my ($processor)= @_;
    return SetProcessorType($processor, SETPROC_COMPAT);
}

sub SegAlign {
    my ($ea, $alignment)= @_;
    return SetSegmentAttr($ea, SEGATTR_ALIGN, $alignment);
}

sub SegComb {
    my ($ea, $comb)= @_;
    return SetSegmentAttr($ea, SEGATTR_COMB, $comb);
}

sub AddBpt {
    my ($ea)= @_;
    return AddBptEx($ea, 0, BPT_SOFT);
}

sub OpOffset {
    my ($ea,$base)= @_;
    return OpOff($ea,-1,$base);
}

sub OpNum {
    my ($ea)= @_;
    return OpNumber($ea,-1);
}

sub OpChar {
    my ($ea)= @_;
    return OpChr($ea,-1);
}

sub OpSegment {
    my ($ea)= @_;
    return OpSeg($ea,-1);
}

sub OpDec {
    my ($ea)= @_;
    return OpDecimal($ea,-1);
}

sub OpAlt1 {
    my ($ea,$str)= @_;
    return OpAlt($ea,0,$str);
}

sub OpAlt2 {
    my ($ea,$str)= @_;
    return OpAlt($ea,1,$str);
}

sub StringStp {
    my ($x)= @_;
    return SetCharPrm(INF_ASCII_BREAK,$x);
}

sub LowVoids {
    my ($x)= @_;
    return SetLongPrm(INF_LOW_OFF,$x);
}

sub HighVoids {
    my ($x)= @_;
    return SetLongPrm(INF_HIGH_OFF,$x);
}

sub TailDepth {
    my ($x)= @_;
    return SetLongPrm(INF_MAXREF,$x);
}

sub Analysis {
    my ($x)= @_;
    return SetCharPrm(INF_AUTO,$x);
}

sub Tabs {
    my ($x)= @_;
    return SetCharPrm(INF_ENTAB,$x);
}

sub Comments {
    my ($x)= @_;
    return SetCharPrm(INF_CMTFLAG,(($x) ? (SW_ALLCMT|GetCharPrm(INF_CMTFLAG)) : (~SW_ALLCMT&GetCharPrm(INF_CMTFLAG))));
}

sub Voids {
    my ($x)= @_;
    return SetCharPrm(INF_VOIDS,$x);
}

sub XrefShow {
    my ($x)= @_;
    return SetCharPrm(INF_XREFNUM,$x);
}

sub Indent {
    my ($x)= @_;
    return SetCharPrm(INF_INDENT,$x);
}

sub CmtIndent {
    my ($x)= @_;
    return SetCharPrm(INF_COMMENT,$x);
}

sub AutoShow {
    my ($x)= @_;
    return SetCharPrm(INF_SHOWAUTO,$x);
}

sub MinEA {
    return GetLongPrm(INF_MIN_EA);
}

sub MaxEA {
    return GetLongPrm(INF_MAX_EA);
}

sub BeginEA {
    return GetLongPrm(INF_BEGIN_EA);
}

sub set_start_cs {
    my ($x)= @_;
    return SetLongPrm(INF_START_CS,$x);
}

sub set_start_ip {
    my ($x)= @_;
    return SetLongPrm(INF_START_IP,$x);
}

sub WriteMap {
    my ($file)= @_;
    return GenerateFile(OFILE_MAP, fopen($file,"w"), 0, BADADDR, GENFLG_MAPSEGS|GENFLG_MAPNAME);
}

sub WriteTxt {
    my ($file,$ea1,$ea2)= @_;
    return GenerateFile(OFILE_ASM,fopen($file,"w"), $ea1, $ea2, 0);
}

sub WriteExe {
    my ($file)= @_;
    return GenerateFile(OFILE_EXE,fopen($file,"wb"), 0, BADADDR, 0);
}

sub AddConst {
    my ($enum_id,$name,$value)= @_;
    return AddConstEx($enum_id,$name,$value,-1);
}

sub AddStruc {
    my ($index,$name)= @_;
    return AddStrucEx($index,$name,0);
}

sub AddUnion {
    my ($index,$name)= @_;
    return AddStrucEx($index,$name,1);
}

sub OpStroff {
    my ($ea,$n,$strid)= @_;
    return OpStroffEx($ea,$n,$strid,0);
}

sub OpEnum {
    my ($ea,$n,$enumid)= @_;
    return OpEnumEx($ea,$n,$enumid,0);
}

sub DelConst {
    my ($id,$v,$mask)= @_;
    return DelConstEx($id,$v,0,$mask);
}

sub GetConst {
    my ($id,$v,$mask)= @_;
    return GetConstEx($id,$v,0,$mask);
}

sub AnalyseArea {
    my ($sEA, $eEA)= @_;
    return AnalyzeArea($sEA,$eEA);
}

sub MakeStruct {
    my ($ea,$name)= @_;
    return MakeStructEx($ea, -1, $name);
}

sub Name {
    my ($ea)= @_;
    return NameEx(BADADDR, $ea);
}

sub GetTrueName {
    my ($ea)= @_;
    return GetTrueNameEx(BADADDR, $ea);
}

sub MakeName {
    my ($ea,$name)= @_;
    return MakeNameEx($ea,$name,SN_CHECK);
}

sub GetFrame {
    my ($ea)= @_;
    return GetFunctionAttr($ea, FUNCATTR_FRAME);
}

sub GetFrameLvarSize {
    my ($ea)= @_;
    return GetFunctionAttr($ea, FUNCATTR_FRSIZE);
}

sub GetFrameRegsSize {
    my ($ea)= @_;
    return GetFunctionAttr($ea, FUNCATTR_FRREGS);
}

sub GetFrameArgsSize {
    my ($ea)= @_;
    return GetFunctionAttr($ea, FUNCATTR_ARGSIZE);
}

sub GetFunctionFlags {
    my ($ea)= @_;
    return GetFunctionAttr($ea, FUNCATTR_FLAGS);
}

sub SetFunctionFlags {
    my ($ea, $flags)= @_;
    return SetFunctionAttr($ea, FUNCATTR_FLAGS, $flags);
}

sub SegStart {
    my ($ea)= @_;
    return GetSegmentAttr($ea, SEGATTR_START);
}

sub SegEnd {
    my ($ea)= @_;
    return GetSegmentAttr($ea, SEGATTR_END);
}

sub SetSegmentType {
    my ($ea, $type)= @_;
    return SetSegmentAttr($ea, SEGATTR_TYPE, $type);
}

sub Comment {
    my ($ea)= @_;
    return CommentEx($ea, 0);
}

sub RptCmt {
    my ($ea)= @_;
    return CommentEx($ea, 1);
}

sub MakeByte {
    my ($ea)= @_;
    return MakeData($ea, FF_BYTE, 1, BADADDR);
}

sub MakeWord {
    my ($ea)= @_;
    return MakeData($ea, FF_WORD, 2, BADADDR);
}

sub MakeDword {
    my ($ea)= @_;
    return MakeData($ea, FF_DWRD, 4, BADADDR);
}

sub MakeQword {
    my ($ea)= @_;
    return MakeData($ea, FF_QWRD, 8, BADADDR);
}

sub MakeOword {
    my ($ea)= @_;
    return MakeData($ea, FF_OWRD, 16, BADADDR);
}

sub MakeFloat {
    my ($ea)= @_;
    return MakeData($ea, FF_FLOAT, 4, BADADDR);
}

sub MakeDouble {
    my ($ea)= @_;
    return MakeData($ea, FF_DOUBLE, 8, BADADDR);
}

sub MakePackReal {
    my ($ea)= @_;
    return MakeData($ea, FF_PACKREAL, 10, BADADDR);
}

sub MakeTbyte {
    my ($ea)= @_;
    return MakeData($ea, FF_TBYT, 10, BADADDR);
}

sub isEnabled {
    my ($ea)= @_;
    return (PrevAddr($ea+1)==$ea);
}

1;
