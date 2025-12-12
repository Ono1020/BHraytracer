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

-include *.d
