all: main.cpp
	g++ main.cpp -o main -pthread -std=c++11 `pkg-config --cflags --libs opencv4`

clean:
	$(RM) main
	