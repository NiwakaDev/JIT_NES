run:Gui.cpp main.cpp Object.cpp
	g++ -c -o Gui.o Gui.cpp -lSDL2 -std=c++11
	g++ -c -o main.o main.cpp -lSDL2 -std=c++11
	g++ -c -o Object.o Object.cpp -lSDL2 -std=c++11
	g++ ${LD_FLGS} -lSDL2 -std=c++11 -o fami Object.o main.o Gui.o
