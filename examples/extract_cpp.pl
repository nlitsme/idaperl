use strict;
use warnings;
use IDC;
use IDA;

# sample idaperl script, which tries to create a C++ headerfile
# given the typeinfo information in a gcc compiled binary.

my %tbyea;
my %tbyname;
# process typeinfo with baseclasses
forrefs(LocByName("__ZTVN10__cxxabiv120__si_class_type_infoE"), "DB", sub {
        my $ea=shift;
        my $name= GetString(Dword($ea+4),-1,ASCSTR_C);
        $name =~ s/^\d+//;
        my $basetiea= Dword($ea+8);
        $tbyea{$ea}= $tbyname{$name}= { tiea=>$ea, classname=>$name, basetiea=>$basetiea };
    });
# process typeinfo without baseclasses
forrefs(LocByName("__ZTVN10__cxxabiv117__class_type_infoE"), "DB", sub {
        my $ea=shift;
        my $name= GetString(Dword($ea+4),-1,ASCSTR_C);
        $name =~ s/^\d+//;
        $tbyea{$ea}= $tbyname{$name}= { tiea=>$ea, classname=>$name };
    });

# remember what methods are virtual
my %vmethods;
# for all types, add child links, get vtables.
for my $ti (values %tbyea) {
    if (exists $ti->{basetiea}) {
        $ti->{baseti}= $tbyea{$ti->{basetiea}};
        push @{$ti->{baseti}{children}}, $ti;
    }
    # find vtable for ti.
    forrefs($ti->{tiea}, "DB", sub {
            my $ea=shift;
            if (Dword($ea-4)==0) {
                if (exists $ti->{vtable}) {
                    printf("WARN: vtable already exists: %08lx .. %08lx\n", $ea, $ti->{vtableea});
                }
                $ti->{vtableea}= $ea;
                $ea+=4;
                #printf("processing vtable (%08lx) for %s\n", $ea, $ti->{classname});
                while ((($ea==$ti->{vtableea}+4) || !(GetFlags($ea) & FF_ANYNAME)) && Dword($ea)) {
                    push @{$ti->{vtable}}, my $method= {
                        idx=>($ea-$ti->{vtableea}-4)/4,
                        methodea=>Dword($ea),
                        virtual=>1,
                    };
                    $method->{fullname}= Name($method->{methodea});

                    $vmethods{$method->{fullname}}= 1;

                    my $dem= Demangle($method->{fullname}, GetLongPrm(INF_LONG_DN));
                    if ($dem =~ /^(\w+)::(~?\w+)\((.*)\)$/) {
                        $method->{class}= $1;
                        $method->{name}= $2;
                        $method->{params}= $3;
                        $ti->{usestype}{$_}++ for grep { $_ ne "void" } split /,\s*/, $method->{params};
                        determine_returntype($method);
                    }
                    elsif ($method->{fullname} eq "___cxa_pure_virtual") {
                        $method->{abstract}= 1;
                        $method->{returntype}= "XX";
                    }
                    else {
                        printf("adding vtab %08lx (%s) - %08lx - WARN: unprocessed methodname: %s\n", 
                            $ti->{vtableea}, $ti->{classname}, $ea,
                            $method->{fullname});
                    }
                    $ea+=4;
                }
                #printf("end of vtable: %08lx, f=%08lx\n", $ea, GetFlags($ea));
            }
        });
}
sub determine_returntype
{
    my ($method)= @_;

    if ($method->{name} eq $method->{class}) {
        if ($method->{fullname} =~ "$method->{class}C(\\d)") {
            $method->{constructor}= $1;
        }
        else {
            printf("WARN: constructor without 'C' : %s\n", $method->{fullname});
        }
    }
    elsif ($method->{name} =~ /^~/) {
        if ($method->{fullname} =~ "$method->{class}D(\\d)") {
            $method->{destructor}= $1;
        }
        else {
            printf("WARN: destructor without 'C' : %s\n", $method->{fullname});
        }
    }
    else {
        $method->{returntype}= "XX";
    }
}
# find all non-virtual functions.
forfunctions(sub {
        my $ea=shift;
        my $fullname= Name($ea);
        return if exists $vmethods{$fullname};
        if (Demangle($fullname, GetLongPrm(INF_LONG_DN)) =~ /^(\w+)::(~?\w+)\((.*)\)$/) {
            my ($class, $name, $params)= ($1,$2,$3);
            if (!exists $tbyname{$class}) {
                $tbyname{$class}= {
                    classname=>$class
                };
            }
            my $ti= $tbyname{$class};
            push @{$ti->{methods}}, my $m= {
                methodea=>$ea,
                fullname=>$fullname,
                class=>$class,
                name=>$name,
                params=>$params,
                # not abstract
                # not virtual
            };
            determine_returntype($m);
            $ti->{usestype}{$_}++ for grep { $_ ne "void" } split /,\s*/, $params;
        }
    });
# resolve virtual functions names in baseclasses, by looking at overriden names
for my $ti (values %tbyea) {
    for my $m (@{$ti->{vtable}}) {
        if ($m->{abstract}) {
            my $derivedm= search_derived($ti, $m->{idx});
            if ($derivedm) {
                $m->{class}= $ti->{classname};
                $m->{name}= $derivedm->{name};
                $m->{params}= $derivedm->{params};
                $ti->{usestype}{$_}++ for grep { $_ ne "void" } split /,\s*/, $m->{params};
            }
            else {
                printf("WARN: no method found for %s::%d\n", $ti->{classname}, $m->{idx});
            }
        }
    }
}
sub search_derived {
    my ($ti, $idx)= @_;
    for my $cti (@{$ti->{children}}) {
        if (!exists $cti->{vtable}[$idx]{abstract}) {
            return $cti->{vtable}[$idx];
        }
        my $d= search_derived($cti, $idx);
        return $d if ($d);
    }
    return undef;
}

# flags: 1 = fwddecl, 2=defined
my %typeprinted;
open FH, ">classdefs.h";
# for classes without baseclass
#    dump as c++
#    for all derived: dump as c++
#
#------------------------------
printf FH ("// total vmethods: %d\n", scalar keys %vmethods);
printf FH ("typedef int XX;\n");
printf FH ("#include <vector>\n");
printf FH ("#include <list>\n");
for my $ti (grep { !exists $_->{baseti} } values %tbyea) {
    print_class($ti);
}

close FH;

sub print_class {
    my ($ti)= @_;
    return if $typeprinted{$ti->{classname}} && ($typeprinted{$ti->{classname}}&2);
    for my $t (keys %{$ti->{usestype}})
    {
        if ($t =~ /^(?:void|int|char|long|unsigned|short)\b/) {
            # nop
        }
        elsif ($t =~/(\w+)(?:\s+const)?(\s*\*+)\s*$/) {
            my ($name, $ptr)= ($1,$2);
            if ($ptr && !$typeprinted{$name}) {
                write_decl($name);
            }
            elsif (!$ptr && !($typeprinted{$name}&1)) {
                if (exists $tbyname{$name}) {
                    my $tn= $tbyname{$name};
                    while (exists $tn->{baseti}) {
                        $tn= $tn->{baseti};
                    }
                    print_class($tn);
                }
                else {
                    write_def($name);
                }
            }
            else {
                printf FH ("// %s : ptr=%d printed=%d\n", $name, ! !$ptr, $typeprinted{$name});
            }
        }
    }
    printf FH ("// ti=%08lx,  vt=%08lx\n", $ti->{tiea}, $ti->{vtableea});
    printf FH ("// virt: %d, other: %d, children: %d\n", 
        exists $ti->{vtable} ? scalar @{$ti->{vtable}}:0, 
        exists $ti->{methods}?scalar @{$ti->{methods}}:0, 
        exists $ti->{children}?scalar @{$ti->{children}}:0);
    if (exists $ti->{usestype}) {
        printf FH ("// uses: %s\n", join(", ", sort keys %{$ti->{usestype}}));
    }
    printf FH ("class %s%s {\n", $ti->{classname}, exists $ti->{baseti}?' : public '.$ti->{baseti}{classname}:"");
    printf FH ("public:\n");
    my %printed;
    for my $m (@{$ti->{vtable}}) {
        my $tag= "$m->{name} ( $m->{params} )";
        if (!exists $printed{$tag}) {
            printf FH ("    virtual %s%s(%s)%s;\n", exists $m->{returntype}?$m->{returntype}.' ':"", $m->{name}, $m->{params}, exists $m->{abstract}?"=0":"");
        }
        $printed{$tag}++;
    }
    printf FH ("\n");
    for my $m (@{$ti->{methods}}) {
        my $tag= "$m->{name} ( $m->{params} )";
        if (!exists $printed{$tag}) {
            printf FH ("    %s%s(%s);\n", exists $m->{returntype}?$m->{returntype}.' ':"", $m->{name}, $m->{params});
        }
        $printed{$tag}++;
    }
    printf FH ("};\n");

    $typeprinted{$ti->{classname}} |= 2;

    for my $c (@{$ti->{children}}) {
        print_class($c);
    }
}
sub write_def {
    my ($name)= @_;
    if ($name =~ /\btag/) {
        printf FH ("struct %s { int unknown; };\n", $name);
    }
    elsif ($name =~ /\b[CE][A-Z]/) {
        printf FH ("class %s { };\n", $name);
    }
    elsif ($name =~ /(?:Type|Code)\b/) {
        # enum
        printf FH ("typedef int %s;\n", $name);
    }
    else {
        printf FH ("typedef int %s;\n", $name);
    }
    $typeprinted{$name} |= 2;
}
sub write_decl {
    my ($name)= @_;
    if ($name =~ /\btag/) {
        printf FH ("struct %s;\n", $name);
    }
    elsif ($name =~ /\b[CE][A-Z]/) {
        printf FH ("class %s;\n", $name);
    }
    elsif ($name =~ /(?:Type|Code)\b/) {
        # enum
        printf FH ("typedef int %s;\n", $name);
        $typeprinted{$name} |= 2;
    }
    else {
        printf FH ("typedef int %s;\n", $name);
        $typeprinted{$name} |= 2;
    }
    $typeprinted{$name} |= 1;
}

