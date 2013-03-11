# Copyright (c) 2013 KAUST - InfoCloud Group (All Rights Reserved)
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# Authors: Amin Allam  <amin.allam@kaust.edu.sa>,
#          Fuad Jamour <fuad.jamour@kaust.edu.sa>
#
# Current version: 1.0 (initial release)
	

# Build targets (your implementation targets should go in IMPL_O)
TEST_O=test_driver/test.o 
OUR_TEST_O=test_ours/test.o
IMPL_O=ref_impl/core.o
OUR_IMPL_O=lib/core.o lib/linked_list.o lib/query.o lib/trie.o lib/word.o

# Compiler flags
CC  = gcc
CXX = g++
CFLAGS=-O3 -fPIC -Wall -g -I. -I./include
CXXFLAGS=$(CFLAGS)
LDFLAGS=-lpthread

# The programs that will be built
PROGRAMS=testdriver testours

# The name of the library that will be built
LIBRARY=core
OUR_LIBRARY=core_ours

# Build all programs
all: $(PROGRAMS)

lib: $(IMPL_O)
	$(CXX) $(CXXFLAGS) -shared -o lib$(LIBRARY).so $(IMPL_O)
	
lib_ours: $(OUR_IMPL_O)
	$(CXX) $(CXXFLAGS) -shared -o lib$(OUR_LIBRARY).so $(OUR_IMPL_O)	

testdriver: lib $(TEST_O)
	$(CXX) $(CXXFLAGS) -o bin/testdriver $(TEST_O) ./lib$(LIBRARY).so
	
testours: lib_ours $(OUR_TEST_O)
	$(CXX) $(CXXFLAGS) -o bin/testours $(OUR_TEST_O) ./lib$(OUR_LIBRARY).so 	

clean:
	rm -f $(PROGRAMS) bin/* lib$(LIBRARY).so lib$(OUR_LIBRARY).so
	find . -name '*.o' -print | xargs rm -f
