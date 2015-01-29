# Define the compiler
CXX	 = g++

# Define the linker. This must be done since the implicit rule
# for linking .o-files uses CC as the linker.
CC	 = g++

# Define preprocessor and compiler flags
CPPFLAGS  = -I..
CXXFLAGS  = -ggdb -Wall -W -pedantic-errors
CXXFLAGS += -Wmissing-braces -Wparentheses -old-style-cast -std=c++11

# Define source files
SRC	= $(wildcard src/*.cc)

# Targets
PROGS	= client serverFile serverMem

# Phony targets
.PHONY: all clean

all: $(PROGS)
	mv -f src/serverFile bin/serverFile
	mv -f src/client bin/client
	mv -f src/serverMem bin/serverMem

# Targets rely on implicit rules for compiling and linking.
client: src/client.o src/connection.o src/message.o
serverFile: src/serverFile.o src/connection.o src/server.o
serverMem: src/serverMem.o src/connection.o src/server.o

# Standard clean and cleaner
clean:
	rm -rf bin/*
	$(RM) src/*.o
	$(RM) src/*.d

# Generate dependencies in *.d files
src/%.d: src/%.cc
	@set -e; rm -f $@; \
	 $(CXX) -MM $(CPPFLAGS) $< > $@.$$$$; \
	 sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	 rm -f $@.$$$$

# Include dependencies in *.d files
include $(SRC:.cc=.d)
