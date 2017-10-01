
OS := $(shell uname)
#$(info OS="$(OS)")

CFLAGS += -std=c99 -fPIC -O3 -Wall -Wextra 
CXXFLAGS += --std=gnu++11 -g -Wall 

INCLUDE=

LIBS :=




%.cc.o: %.cc
	g++ -c $(CXXFLAGS) $< -o $@  

%.cpp.o: %.cpp
	g++ -c $(CXXFLAGS) $< -o $@  

%.c.o: %.c
	gcc -c  $(CFLAGS) $< -o $@


all: build-tetrartree



SRCFILES = \
tetrartree.cpp \
#query-basic-functions.cpp\






# Multiple pattern substition to get the object file right.
OBJFILES := $(patsubst %.cc, %.cc.o, $(SRCFILES))
OBJFILES := $(patsubst %.cpp, %.cpp.o, $(SRCFILES))
OBJFILES := $(patsubst %.c, %.c.o, $(OBJFILES))


build-tetrartree: $(OBJFILES) build-tetrartree.cpp
	g++ $(CXXFLAGS) $(INCLUDE) -o $@  $@.cpp $(OBJFILES) $(LIBS)

pointQuery-with-tetrartree: $(OBJFILES) pointQuery-with-tetrartree.cpp
	g++ $(CXXFLAGS) $(INCLUDE) -o $@  $@.cpp $(OBJFILES) $(LIBS)


#rangeQuery-with-rtree: $(OBJFILES) rangeQuery-with-rtree.cpp
#	g++ $(CXXFLAGS) $(INCLUDE) -o $@  $@.cpp $(OBJFILES) $(LIBS)


	
clean:
	rm -f *.o *.exe build-tetrartree pointQuery-with-tetrartree
	cp /dev/null ./build-index.log
	cp /dev/null ./query.log
	cp /dev/null ./orientation-info.log
	cp /dev/null ./extended-4KB-correct-opt-tetrartree.index
doc:
	doxygen

clean-doc:
	rm -fr doc
	cp /dev/null ./tetrartree.index

clean-all: clean clean-doc
  


.PHONY: clean doc clean-doc clean-all
