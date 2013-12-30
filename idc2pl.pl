#!perl -w
# vim:set ts=4 sw=4:
#
# (C) 2008 Willem Hengeveld  itsme@xs4all.nl
#
# IdcPerl - a perl scripting plugin for the Interactive Disassembler by hex-rays.
# see http://www.xs4all.nl/~itsme/projects/idcperl
#
use strict;
use Parse::RecDescent;

# first process preprocessor commands

# replace 'auto ...' with 'my ...'
# change function defs 'static name(...)'  to  'sub name { my (...)=@_; ...}
# remember var+arg names, from the auto and arg list.
# prefix all variables with '$'

# change comment from '//'   to '#'

# todo: grammar of idc.idc is different:
# besides 'static' there is also:
#    'string',  'string or long',  'success', 'long'
#    and arguments have types in arg lists.
#    and idc.idc contains declarations, instead of definitions.

# problems:  empty file fails to match grammar.
#    idc also accepts  static fnname(arg arg arg)  -- i assume this is a bug.
#
my $idcgrammar=<<'__EOF__';
program: function(s?)

function:
  "static" IDENTIFIER "(" arg_list ")"
  "{"
      vardecl(s?)
      statement(s?)
  "}"

arg_list:
    "void"
  | IDENTIFIER(s? /,/)

vardecl: "auto" IDENTIFIER(s? /,/) ";"  

statement: if_statement
         | while_statement
         | for_statement
         | do_statement
         | expression ";"
         | "{" vardecl(s?) statement(s?) "}"
         | "break" ";"
         | "continue" ";"
         | "return" expression(?) ";"
         | ";"

do_statement: "do" statement "while" "(" expression ")" ";"

if_statement: "if" "(" expression ")"
                  statement
             ( "else"
                  statement
             )(?)

while_statement: "while" "(" expression ")"  statement

for_statement:   "for" "(" expression(?) ";" expression(?) ";" expression(?) ")" statement

	expression:
		assignment_expression

	assignment_expression:
		unary_expression ASSIGNMENT_OPERATOR assignment_expression
	|	conditional_expression 


	conditional_expression:
		 logical_OR_expression  ('?' expression ':' conditional_expression)(?)

	logical_OR_expression:
		logical_AND_expression(s /(\|\|)/)

	logical_AND_expression:
		inclusive_OR_expression(s /(&&)/)

	inclusive_OR_expression:
		exclusive_OR_expression(s /(\|)/)

	exclusive_OR_expression:
		AND_expression(s /(\^)/)

	AND_expression:
		equality_expression(s /(&)/)

	equality_expression:
		relational_expression(s /(==|!=)/)

	relational_expression:
		shift_expression(s /(<=|>=|<|>)/)

	shift_expression:
		additive_expression(s /(<<|>>)/)

	additive_expression:
		multiplicative_expression(s /(\+|-)/)

	multiplicative_expression:
		cast_expression(s /(\*|\/|%)/)

	cast_expression:
        ( "char" | "float" | "long" ) "(" expression ")"
		| unary_expression

	unary_expression:
		postfix_expression
	|	'++'  unary_expression
	|	'--'  unary_expression
	|   UNARY_OPERATOR cast_expression
		
		
	postfix_expression:
		primary_expression postfix_expression_token(s?)


	postfix_expression_token:
		  '(' argument_expression_list(?)')'
		| '++' 
		| '--'


	primary_expression:
		IDENTIFIER
	|	constant
	|	STRING(s)
	|	'(' expression ')'
    | "[" expression "," expression "]"

	argument_expression_list:
		assignment_expression(s /(,)/)

	constant:
		CHARACTER_CONSTANT
	|	FLOATING_CONSTANT
	|	INTEGER_CONSTANT


###	TERMINALS


	INTEGER_CONSTANT:
		/(?:
         (?:0[xX][\da-fA-F]+) 					# Hexadecimal
		 |(?:0[0-7]*)		  					# Octal or Zero
		 |(?:[1-9]\d*)		  					# Decimal
         )
		 [uUlL]?			  					# Suffix
		 /x
		 
	CHARACTER_CONSTANT:
		/'([^\\'"]							# None of these
		 |\\['\\ntvbrfa'"]  				# or a backslash followed by one of those
		 |\\[0-7]{1,3}|\\x\d+)'			# or an octal or hex constant
		/x

	FLOATING_CONSTANT:
		/(?:\d+|(?=\.\d+))					# No leading digits only if '.moreDigits' follows
		 (?:\.|(?=[eE]))						# There may be no floating point only if an exponent is present
		 \d*									# Zero or more floating digits
		 ([eE][+-]?\d+)?						# expontent
		 [lLfF]?								# Suffix 
		/x

	STRING:
		/"(
            ([^\\"])							# None of these
			|(\\[\\ntvbrfa"])					# or a backslash followed by one of those
			|(\\[0-7]{1,3})
            |(\\x[0-9a-f]+)		                # or an octal or hex 
            |(\\.)                              # any other char can be escaped.
         )*"/x
	
	IDENTIFIER:
		/(?!(auto|break|case|char|const|continue|default|do|double|else|enum|extern|float|for|goto		# LOOKAHEAD FOR KEYWORDS
			|if|int|long|register|return|signed|sizeof|short|static|struct|switch|typedef			# NONE OF THE KEYWORDS
			|union|unsigned|void|volatile|while)[^a-zA-Z_])											# SHOULD FULLY MATCH!
			(([a-zA-Z]\w*)|(_\w+))/x																# Check for valid identifier
  
	ASSIGNMENT_OPERATOR:
		'=' 
	
	UNARY_OPERATOR:
		'+' | '-' | '~' | '!'
__EOF__


my $ppgrammar=<<'__EOF__';
file:
    group(s?)
group:
    control_line
  | if_section
  | text_line
  | /^\s*#.*?\n/

if_section:
    if_group
    elif_group(s?)
    else_group(?)
    endifline

if_group:
    /^\s*#/ "if" expression "\n" group
  | /^\s*#/ "ifdef" identifier "\n" group
  | /^\s*#/ "ifndef" identifier "\n" group
elif_group:
    /^\s*#/ "elif" expression "\n" group
else_group:
    /^\s*#/ "else" "\n" group
endifline:
    /^\s*#\s*endif\s*$/

identifier: /[a-zA-Z_\$][a-zA-Z0-9_\$]*/
number: /\d+(?:\.\d+)?(?:[eE]\d+)?/
expression:
    or_expression(s /\|\|/)
or_expression:
    and_expression(s /&&/)
and_expression:
    identifier
    | number
control_line:
      /^\s*#/ "include" filename /\s*$/
    | /^\s*#/ "define" identifier  /(.*)/
    | /^\s*#/ "define" identifier"(" arglist ")"   /(.*)/
    | /^\s*#/ "undef"
    | /^\s*#/ "line"   /(.*)/
    | /^\s*#/ "error"  /(.*)/
    | /^\s*#/ "pragma" /(.*)/
    | /^\s*#\s*$/
arglist:  identifier(s? /,/)
filename:
    "\"" /(.*)/ "\""
  | "<" /(.*)/ ">"

text_line:
    /.*/
__EOF__

my $cmtgrammar=<<'__EOF__';
<autotree>
file:
    item(s?)
item:
    qstring
    | qqstring
    | linecomment
    | multilinecomment
    | nonstring

qstring:
    /'(?:\.|[^\n\\'])*'/
qqstring:
    /"(?:\.|[^\n\\"])*"/
nonstring:
    /[^"'\/]+/s
linecomment:
    /\/\/.*?$/
multilinecomment:
    /\/\*.*?\*\//s
__EOF__

sub preprocess {

    # first handle backslash+newline
    # then remove comments
    # then evaluate preprocessor directives.
    # 6.4.9  - comments
    while ( $_[0] =~ s/\r//gs ) { }
    while ( $_[0] =~ s/\\\n//gs ) { }
    while ( $_[0] =~ s/^([^"\n]*(?:"(?:\\.|[^"\n\\])*"[^"\n]*)*)\/\*.*?\*\//$1 /mgs ) { }
    while ( $_[0] =~ s/^([^"\n]*(?:"(?:\\.|[^"\n\\])*"[^"\n]*)*)\/\/.*?$/$1 /mgs ) { }
    while ( $_[0] =~ s/^\s*#.*?$//mgs ) { }
    while ( $_[0] =~ s/^\s+\w\s\w\s\w\s.*?$//mgs) { }
}
our $RD_WARN=1;
our $RD_HINT=1;
#our $RD_TRACE=1;
my $idcparser = new Parse::RecDescent ($idcgrammar);
printf("\n*************************************************\n");
local $/;
my $text=<>;
use Dumpvalue;
my $d= new Dumpvalue;
preprocess($text);
printf("l1=%d\n", length($text));
#print "--$text--\n";
my $t= $idcparser->program(\$text);
printf("l2=%d\n", length($text));
$d->dumpValue($t);
print "<<$text>>\n";

#my $cmt= new Parse::RecDescent($cmtgrammar);
#my $pp= new Parse::RecDescent($ppgrammar);

#$d->dumpValue($cmt->qstring("'qweqwe'"));
#$d->dumpValue($cmt->qqstring("\"qweqwe\""));
#$d->dumpValue($cmt->linecomment("// test\n"));
#$d->dumpValue($cmt->multilinecomment("/* test\n ... */"));

#$d->dumpValue($cmt->file("asdadas  /* test\n ... */ // comment\nqweqew qweqe\n'//' ..... /* .. */"));
