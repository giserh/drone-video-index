
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


all: calc-quatrilaterals data-extension area-comparison generate-queries



SRCFILES = \
drone.cpp \






# Multiple pattern substition to get the object file right.
OBJFILES := $(patsubst %.cc, %.cc.o, $(SRCFILES))
OBJFILES := $(patsubst %.cpp, %.cpp.o, $(SRCFILES))
OBJFILES := $(patsubst %.c, %.c.o, $(OBJFILES))


calc-quatrilaterals: $(OBJFILES) calc-quatrilaterals.cpp
	g++ $(CXXFLAGS) $(INCLUDE) -o $@  $@.cpp $(OBJFILES) $(LIBS)

data-extension: $(OBJFILES) data-extension.cpp
	g++ $(CXXFLAGS) $(INCLUDE) -o $@  $@.cpp $(OBJFILES) $(LIBS)
	
area-comparison: $(OBJFILES) area-comparison.cpp
	g++ $(CXXFLAGS) $(INCLUDE) -o $@  $@.cpp $(OBJFILES) $(LIBS)
	
generate-queries: $(OBJFILES) generate-queries.cpp
	g++ $(CXXFLAGS) $(INCLUDE) -o $@  $@.cpp $(OBJFILES) $(LIBS)
	
	
clean:
	rm -f *.o *.exe

doc:
	doxygen

clean-doc:
	rm -fr doc


clean-all: clean clean-doc
  


.PHONY: clean doc clean-doc clean-all
