all: angrybirds

sample3D: AngryBirds.cpp glad.c
	g++ -o angrybirds AngryBirds.cpp glad.c -lGL -lglfw -g

angrybirds: AngryBirds.cpp glad.c
	g++ -o angrybirds AngryBirds.cpp glad.c -lGL -lglfw -lftgl -I/usr/local/include -I/usr/local/include/freetype2 -I/usr/local/include/FTGL -L/usr/local/lib  -ldl -g

clean:
	rm angrybirds
