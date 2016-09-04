# definitions
CFLAGS   = -lwiringPi -Wall 
CXXFLAGS = -std=c++11  
DEPS = ../rc-switch/RCSwitch.h AstroCalc4R.h lights433.h
LIBS = -lm 
CC = g++
TS := $(shell /bin/date "+%Y-%m-%d-%H-%M-%S")
prefix=/usr/local

%.o: %.c $(DEPS)
	$(CXX) -g -c -o $@ $< $(CFLAGS) $(LIBS)

all: lights433

lights433: ../433Utils/rc-switch/RCSwitch.o AstroCalc4R.o ini.o INIReader.o lights433.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ $(CFLAGS) $(LIBS)
	git status -s

clean:
	$(RM) *.o lights433

test: ../433Utils/rc-switch/RCSwitch.o test.o 
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ $(CFLAGS) $(LIBS)
	git status -s

install: lights433
	install -m 0755 lights433 $(prefix)/bin
.PHONY: install

run:
	$(prefix)/bin/lights433

# commands for git 
# http://git-scm.com/book/en/v2/Git-Basics-Recording-Changes-to-the-Repository
git-status:
	git status -s

git-stage:
	git add *.c
	git add *.cpp
	git add *.h
	git add Makefile

git-commit:
	git commit -a -m $(TS)

git-list:
	git ls-tree --full-tree -r HEAD
