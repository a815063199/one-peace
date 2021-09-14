.PHONY : all
all : test

OBJ1 = main.o
OBJ3 = coro.o

test : $(OBJ1) $(OBJ3)
	g++ -g $(OBJ1) $(OBJ3) -lpthread -o test

$(OBJ1): %.o: %.cpp
	g++ -c -g $< -o $@ -std=c++17

$(OBJ3): %.o: %.cpp
	g++ -c -g $< -o $@ -std=c++17

.PHONY : clean

clean : clean_o clean_exe

clean_o :
	rm *.o 

clean_exe :
	rm test
