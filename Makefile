all: sample3D sample2D

sample3D: Sample_GL3_3D.cpp glad.c
	g++ -o sample3D Sample_GL3.cpp glad.c -lGL -lglfw -g

sample2D: Sample_GL3_2D.cpp glad.c
	g++ -o sample2D Sample_GL3_2D.cpp glad.c -lGL -lglfw -ldl -g

clean:
	rm sample2D sample3D
