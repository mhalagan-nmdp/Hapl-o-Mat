CXX = g++
INC1 = include
INCDIRS = -I${INC1}
#CXXFLAGS = -W -Wall -g -Wextra -pedantic -std=c++11 ${INCDIRS}
#CXXFLAGS = -Wall -pg -march=native -O2 -std=c++11 ${INCDIRS}
CXXFLAGS = -Wall -march=native -Ofast -std=c++11 ${INCDIRS}

sources = $(wildcard src/*.cc)
OBJ = $(patsubst %.cc, %.o, $(wildcard src/*.cc))

haplomat: $(OBJ)
	$(CXX) ${CXXFLAGS} -o $@ $(OBJ) 

%.d: %.cc %.h
	@ $(CXX) -MM $(CXXFLAGS) $< -o $@

%.o: %.cc %.h
	$(CXX) ${CXXFLAGS} -c $< -o $@

clean:
	$(RM) -f src/*.o *.d
-include $(sources:.cc=.d)

.PHONY: clean all
