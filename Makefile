#include $(HOME)/source/makefile/makefile.def
CC = /usr/bin/gcc
#CPP = /usr/bin/g++

BINEXEC = tabterm
# General flags needed for compilation
CFLAGS = -Wall -g
#INCFLAGS = -I. -I/usr/local/include/sys
LDLIBS =-lX11 -lXt -lXmu
LDFLAGS =-L/usr/X11R6/lib

# flags for cpp files
#CPPFLAGS =-Wall -O3 -g -msse -mcpu=i686 --fast-math
#INCCPPFLAGS = -I. -I/usr/include/g++
#LDCPPLIBS = -L/usr/lib -L/usr/lib/gcc-lib  -I/usr/local/lib/

#SPECIAL_FLAGS =-DTIMERS
#LDCPPFLAGS =-lm
 
# Compile line
COMPILE_C=$(CC) -c $(CFLAGS) $(INCFLAGS) $< -o $@
#COMPILE_CPP=$(CPP) -c $(SPECIAL_FLAGS) $(CPPFLAGS) $(INCCPPFLAGS) $< -o $@

# Link line
LINK_C=$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS) 
#LINK_CPP=$(CPP) $(LDCPPFLAGS) -o $@ $^ $(LDCPPLIBS) 
 
# make Library line
LIBRARY=$(AR) rc  $@ $^ 

# Optimized Suffix rule
#%.c : %.h
#	touch $@
%.o : %.c
	$(COMPILE_C)
%.opp : %.cpp
	$(COMPILE_CPP)
%.O : %.C
	$(COMPILE_CPP)

nothing: all

clean:
	@rm -f *.[oO] 
	@rm -f ../obj/*.[oO]
	@if [ -e $(BINEXEC) ]; then rm $(BINEXEC); fi

# check file dependencies, regenerate make.dep
depend:
	@./dep

# main functions for different modules

# time this program
time: all
	@time $(BINEXEC)

# just run the program
run: all
	$(BINEXEC)

# main functions for different modules
all: $(BINEXEC)
	@echo -e "[01;32mDone, executable is \`$(BINEXEC)'[m"

## begin dep: DO NOT REMOVE THIS LINE
appres.o: appres.c appres.h Makefile

button.o: button.c button.h Makefile

tabbar.o: tabbar.c tabbar.h tabs.h button.h Makefile

tabs.o: tabs.c tabs.h Makefile

tabterm.o: tabterm.c tabbar.h tabs.h button.h events.inc Makefile

ALL_TARGETS=appres.o button.o tabbar.o tabs.o tabterm.o

## end dep: DO NOT REMOVE THIS LINE

$(BINEXEC): $(ALL_TARGETS)
	$(LINK_C)

