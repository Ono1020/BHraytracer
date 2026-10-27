PROGRAM = BHraytracer.out
OBJECTS = main.o Vec3.o Ray.o
CXXFLAGS = -Wall -MMD -MP -fopenmp
VPATH = src

all: $(OBJECTS)
	g++ $(CXXFLAGS) -o $(PROGRAM) $(OBJECTS)

.PHONY : clean

clean :
	-rm $(OBJECTS) $(PROGRAM)

run : $(PROGRAM)
	./$(PROGRAM)

.PHONY : test

test :
	g++ -Wall -O2 -Isrc -o test_geodesic.out tests/test_geodesic.cpp src/Ray.cpp src/Vec3.cpp
	./test_geodesic.out

-include *.d
