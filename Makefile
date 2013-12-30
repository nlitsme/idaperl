
ifneq ($(wildcard $(WINDOWS)/system32/krnl32.dll),)
OSTYPE=win32
endif
ifneq ($(wildcard /sbin/bsdlabel),)
OSTYPE=freebsd
endif
ifneq ($(wildcard /System/Library/Extensions/AppleFileSystemDriver.kext),)
OSTYPE=darwin
endif
ifneq ($(wildcard /sbin/modprobe),)
OSTYPE=linux
endif

ifeq ($(OSTYPE),darwin)
MAKEFILE=Makefile.osx
endif
ifeq ($(OSTYPE),win32)
MAKEFILE=Makefile.w32
endif
ifeq ($(OSTYPE),freebsd)
MAKEFILE=Makefile.bsd
endif
ifeq ($(OSTYPE),linux)
MAKEFILE=Makefile.lin
endif

osx:
	$(MAKE) -f $(MAKEFILE)
.PHONY: $(MAKECMDGOALS)
$(MAKECMDGOALS): 
	$(MAKE) -f $(MAKEFILE) $(MAKECMDGOALS)


