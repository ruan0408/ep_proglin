simplex: main.o Network.o Arc.o
	g++-4.6 Network.o Arc.o main.o -o simplex

main.o: main.cpp Network.hpp
	g++-4.6 -std=c++0x -c main.cpp 

Network.o: Network.cpp Network.hpp
	g++-4.6 -std=c++0x -c Network.cpp

Arc.o: Arc.cpp Arc.hpp
	g++-4.6 -std=c++0x -c Arc.cpp

clean:
	rm -rf *.o