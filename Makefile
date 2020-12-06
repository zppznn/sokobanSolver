all: sokoban.cpp
	g++ sokoban.cpp -o a.out -std=c++11
debug: sokoban.cpp
	g++ sokoban.cpp -o a.out -g -std=c++11
clean:
	rm a.out
