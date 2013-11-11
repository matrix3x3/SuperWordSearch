#
# Makefile for superWordSearch
#
#

targets = SuperWordSearch
objects = superwordsearch.o main.o

all: $(targets)

$(targets) : $(objects)
	g++ -g -o $(targets) $(objects) -lpthread

%.o : %.cpp
	g++ -g -c $<

.PHONY : clean
clean:
	rm -f *.o
	rm -f SuperWordSearch


