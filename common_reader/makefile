.PHONY : all
all: test

OBJ3 = test.o

GCC = gcc
GXX = g++

CFLAGS = -I/usr/local/tars/cpp/include -W -Wall 
CXXFLAGS = -std=c++17

test: $(OBJ3)
	$(GXX)  $(OBJ3) -lpthread -o test

$(OBJ1) $(OBJ2) $(OBJ3): %.o: %.cpp
	$(GXX) -c -g $< -o $@ $(CFLAGS) $(CXXFLAGS)

.PHONY : clean
clean: clean_o clean_exe
clean_o:
	rm *.o
clean_exe: 
	rm test
