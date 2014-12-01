clean:
	rm -f 2d 3d

2d:
	g++ -O3 -g -Wall -Wextra -pedantic -std=c++11 -o 2d src/2d/main.cpp src/2d/model.cpp src/2d/view.cpp -lGL -lGLU -lglut -lglui -pthread

3d:
	g++ -O3 -g -Wall -std=c++11 -Wextra -pedantic -L /lib64 -o 3d src/3d/main.cpp src/3d/model.cpp src/3d/view.cpp -lGL -lGLU -lglut -lglui -l pthread

all: clean 2d 3d
