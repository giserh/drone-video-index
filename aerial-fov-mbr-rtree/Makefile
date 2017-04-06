
OS := $(shell uname)
#$(info OS="$(OS)")

CFLAGS += -std=c99 -fPIC -O3 -Wall -Wextra 
CXXFLAGS += --std=gnu++11 -g -Wall 

INCLUDE=
#TODO: Indicate above the kmeans dir.

LIBS :=




%.cc.o: %.cc
	g++ -c $(CXXFLAGS) $< -o $@  

%.cpp.o: %.cpp
	g++ -c $(CXXFLAGS) $< -o $@  

%.c.o: %.c
	gcc -c  $(CFLAGS) $< -o $@


all: build-rtree pointQuery-with-rtree



SRCFILES = \
rtree.cpp \
aerialfov.cpp \
#query-basic-functions.cpp \






# Multiple pattern substition to get the object file right.
OBJFILES := $(patsubst %.cc, %.cc.o, $(SRCFILES))
OBJFILES := $(patsubst %.cpp, %.cpp.o, $(SRCFILES))
OBJFILES := $(patsubst %.c, %.c.o, $(OBJFILES))

	
build-rtree: $(OBJFILES) build-rtree.cpp
	g++ $(CXXFLAGS) $(INCLUDE) -o $@  $@.cpp $(OBJFILES) $(LIBS)

pointQuery-with-rtree: $(OBJFILES) pointQuery-with-rtree.cpp
	g++ $(CXXFLAGS) $(INCLUDE) -o $@  $@.cpp $(OBJFILES) $(LIBS)

object-file-txt2binary: aerialfov.cpp object-file-txt2binary.cpp
	g++ $(CXXFLAGS) $(INCLUDE) -o $@  $@.cpp aerialfov.cpp $(LIBS)

#rangeQuery-with-rtree: $(OBJFILES) rangeQuery-with-rtree.cpp
#	g++ $(CXXFLAGS) $(INCLUDE) -o $@  $@.cpp $(OBJFILES) $(LIBS)

	
	
clean:
	rm -f *.o *.exe
	cp /dev/null ./build-index.log
	cp /dev/null ./query.log

doc:
	doxygen

clean-doc:
	rm -fr doc
	cp /dev/null ./rtree.index
	cp /dev/null ./binary-quatrilaterals.dat

clean-all: clean clean-doc
  


.PHONY: clean doc clean-doc clean-all
