main: main.cpp
	g++ -std=c++11 main.cpp `pkg-config --cflags --libs glib-2.0` -o main -lpthread -fopenmp

teste: teste.cpp
	g++ -std=c++11 teste.cpp -o teste -fopenmp
