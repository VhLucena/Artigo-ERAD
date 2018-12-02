main: src/main.cpp
	g++ -std=c++11 src/main.cpp `pkg-config --cflags --libs glib-2.0` -o bin/main -lpthread -fopenmp

teste: src/teste.cpp
	g++ -std=c++11 src/teste.cpp -o bin/teste -fopenmp

main-mac: src/main.cpp
	clang++ src/main.cpp -o bin/main-mac -Xpreprocessor -fopenmp -lomp 