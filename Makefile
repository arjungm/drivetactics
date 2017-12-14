all: game


game: game.cpp
	g++ game.cpp -std=c++14 -o game

clean:
	rm game
