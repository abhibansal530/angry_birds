#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;

	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1,pipe_rot=-52.0;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
float tx,ty,ti=0;
typedef struct ball{
	float stx,sty;
	float sx,sy,x,y,vel,velx,vely,lu,st;
	float r,k,velx_in,vely_in;
	bool isshoot,collision_obj,collision_ground,falling;
	VAO *circle;
	GLfloat vbd[7000];
	GLfloat cbd[7000];
	glm::mat4 project;
	glm::mat4 translate; 
	void create(){
		project = glm::mat4(1.0f);
		translate = glm::mat4(1.0f);
		collision_ground=collision_obj=falling=false;
		sx=sy=0;
		vel = 500;
		k=1;
		int v=0,k=0,j=0;
		float i;
		for(i =0.5;i<=360;i+=0.5){
			v++;
			float tmp[]={0,0,0,r*cos(i*M_PI/180.0f),r*sin(i*M_PI/180.0f),0,r*cos((i-0.5)*M_PI/180.0f),r*sin((i-0.5)*M_PI/180.0f),0};
			for(int j=0;j<9;++j)vbd[k++]=tmp[j];
		}
		for(int i=0;i<3*v;++i){
			cbd[j++]=0;
			cbd[j++]=0;
			cbd[j++]=1;
		}
		circle = create3DObject(GL_TRIANGLES,3*v,vbd,cbd,GL_FILL);
		return;	
	}
	bool onground(){
		//printf("y: %f is:%d\n",y,isshoot);
		return y<=-300&&isshoot;
	}
	void draw(float nx,float ny,float s){
		glm::mat4 MVP;
		glm::mat4 VP = Matrices.projection * Matrices.view;
		Matrices.model = glm::mat4(1.0f);
		glm::mat4 translateBall = translate*glm::translate(glm::vec3(nx,ny*s,0));
		glm::mat4 rotateBall = glm::rotate((float)(pipe_rot*M_PI/180.0f),glm::vec3(0,0,1));
		glm::mat4 translateBallagain = glm::translate (glm::vec3(-3.5*0.9*100,-3*0.9*100,0));
		Matrices.model*=(project*translateBallagain*rotateBall*translateBall);
		float *mv = (&Matrices.model[0][0]);
		x =  mv[12];
		y =  mv[13];
		float z =  mv[14];
		float wp =  mv[15];

		x /= wp;
		y /= wp;
		z /= wp;
		//printf("%f %f\n",x,y);
		if (onground()&&collision_ground){
			printf("onground\n");
			collision_ground=false;

		}
		MVP = VP*Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
		//x=nx,y=ny;
		printf("x:%f y:%f\n",x,y);
		draw3DObject(circle);	
	}
	void shoot(float ang){
		if(!collision_ground&&!collision_obj){
			stx =x,sty=y;
		}
		//float ang = -1.f*pipe_rot*M_PI/180.f;
		//ang = 0.5*M_PI - ang;
		//float vel = 500;
		//printf("shooted\n");
		st = glfwGetTime();
		lu=glfwGetTime();
		isshoot=true;
		//sx=x,sy=y;
		velx=velx_in=vel*cos(ang),vely=vely_in=vel*sin(ang);
		//printf("in shoot velx:%f vely:%f\n",velx,vely);
	}
	void fire(float s){
		float nx,ny;
		float ct;
		ti = glfwGetTime();
		if(ti-lu>=10e-10){
			ti-=st;
			//printf("sx: %f sy: %f\n",sx,sy);
			nx = sx+velx_in*ti;
			ny=sy+vely_in*ti-100*ti*ti;
			lu=glfwGetTime();
			vely = vely_in - 200*ti;
			if(vely<=0)falling=true;
			else falling = false;
			printf("velx:%f vely:%f\n",velx,vely);
		}
		//ti+=0.1;
		project = glm::translate(glm::vec3(nx,ny,0));
		//vely-=0.01*ti;
		draw(0,25+10+15,s);
	}

	void move(float nx,float ny){
		x=nx,y=ny;
	}
	void moveto(float nx,float ny){
		x=nx,y=ny;
	}
} ball;
typedef struct ground
{
	VAO *shape;
	void create(){
		GLfloat vbd[]={
			-650,-500,0,
			650,-500,0,
			650,-100,0,

			650,-100,0,
			-650,-500,0,
			-650,-100,0
		};
		GLfloat cbd[]={
			0.94,0.67,0.4,
			0.94,0.67,0.4,
			0.94,0.67,0.4,

			0.94,0.67,0.4,
			0.94,0.67,0.4,
			0.94,0.67,0.4
		};
		shape = create3DObject(GL_TRIANGLES,6,vbd,cbd,GL_FILL);
	}
	void draw(){
		glm::mat4 MVP;
		glm::mat4 VP = Matrices.projection * Matrices.view;
		Matrices.model = glm::mat4(1.0f);
		MVP = VP*Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
		draw3DObject(shape);
	}
	void checkCollision(ball &b){
		if(b.onground()&&b.falling&&!b.collision_ground){
			printf("collided ground x:%f y:%f \n",b.x,b.y);
			b.collision_ground=b.falling=true;
			b.sx=b.x-b.stx,b.sy=b.y-b.sty;
			b.vel = (b.velx*b.velx + b.vely*b.vely)/600;  //alpha of collision = 1/600
			float ang = atan(-1*b.vely/b.velx);
			b.shoot(ang);  //angle is hard-coded for test
		}
	}

}ground;
typedef struct sky{
	VAO *shape;
	void create(){
		GLfloat vbd[]={
			-650,-100,0,
			650,-100,0,
			650,500,0,

			650,500,0,
			-650,-100,0,
			-650,500,0
		};
		GLfloat cbd[]={
			0,0,0.1,
			0,0,0.1,
			0,0,0.1,

			0,0,0.1,
			0,0,0.1,
			0,0,0.1
		};
		shape = create3DObject(GL_TRIANGLES,6,vbd,cbd,GL_FILL);
	}
	void draw(){
		glm::mat4 MVP;
		glm::mat4 VP = Matrices.projection * Matrices.view;
		Matrices.model = glm::mat4(1.0f);
		MVP = VP*Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
		draw3DObject(shape);
	}
}sky;
typedef struct obstacle
{	VAO* shape;
	void create(){
		GLfloat vbd[]={
			-50,-50,0,
			50,-50,0,
			50,50,0,

			50,50,0,
			-50,-50,0,
			-50,50,0
		};
		GLfloat cbd[]={
			1,0,0,
			1,0,0,
			1,0,0,

			1,0,0,
			1,0,0,
			1,0,0
		};
		shape = create3DObject(GL_TRIANGLES,6,vbd,cbd,GL_FILL);
	}
	void draw(){
		glm::mat4 MVP;
		glm::mat4 VP = Matrices.projection * Matrices.view;
		Matrices.model = glm::mat4(1.0f);
		MVP = VP*Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
		draw3DObject(shape);
	}
	void checkCollision(ball &b){
		if(b.x>=-50-15&&b.x<=50+15&&b.y>=-50-15&&b.y<=50+15&&!b.collision_obj&&b.isshoot){
			float ang;
			printf("obscollided x:%f y:%f \n",b.x,b.y);
			b.collision_obj=true;
			b.sx=b.x-b.stx,b.sy=b.y-b.sty;
			if(b.x<=-50){
				printf("left velx:%f vely:%f\n",b.velx,b.vely);
				ang = M_PI/2.0 + atan(b.velx/b.vely);
			}
			else {
				printf("bottom velx:%f vely:%f\n",b.velx,b.vely);
				//ang = M_PI/2.0-1.0*atan(b.vely/b.velx);
				ang = -1.0*M_PI/4.0;
			}
			b.shoot(ang);
		}
	}
	
}obstacle;
ball my;
ground gameground;
sky gamesky;
obstacle test;
float ang;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_C:
				rectangle_rot_status = !rectangle_rot_status;
				break;
			case GLFW_KEY_P:
				triangle_rot_status = !triangle_rot_status;
				break;
			case GLFW_KEY_X:
				// do something ..
				break;
			case GLFW_KEY_SPACE:
				ang = -1.f*pipe_rot*M_PI/180.f;
				ang = 0.5*M_PI - ang;
				my.shoot(ang);
				break;
			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
			default:
				break;
		}
	}
	else if(action==GLFW_REPEAT){
		switch(key){
			default:
			break;
		}
	}
	
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
			quit(window);
			break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_RELEASE)
				triangle_rot_dir *= -1;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_RELEASE) {
				rectangle_rot_dir *= -1;
			}
			break;
		default:
			break;
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	// Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	//Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
	Matrices.projection = glm::ortho(-650.0f, 650.0f, -500.0f, 500.0f, 0.1f, 500.0f);
}

VAO *triangle, *rectangle,*shape;
VAO *box,*circle,*pipe,*spring;

// Creates the triangle object used in this sample code
void createTriangle ()
{
	/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

	/* Define vertex array as used in glBegin (GL_TRIANGLES) */
	static const GLfloat vertex_buffer_data [] = {
		0, 1,0, // vertex 0
		-1,-1,0, // vertex 1
		1,-1,0, // vertex 2
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 0
		0,1,0, // color 1
		0,0,1, // color 2
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

// Creates the rectangle object used in this sample code
void createRectangle ()
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		-2,-1,0, // vertex 1
		2,-1,0, // vertex 2
		2, 1,0, // vertex 3

		2, 1,0, // vertex 3
		-2, 1,0, // vertex 4
		-2,-1,0  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		0.647059,0.164706,0.164706, // color 1
		0.647059,0.164706,0.164706, // color 2
		0.647059,0.164706,0.164706, // color 3

		0.647059,0.164706,0.164706, // color 3
		0.647059,0.164706,0.164706, // color 4
		0.647059,0.164706,0.164706  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createBox()
{
	static const GLfloat vertex_buffer_data [] ={
		-1.2,-1,0, // vertex 1
		1.2,-1,0, // vertex 2
		1.2, 1,0, // vertex 3

		1.2, 1,0, // vertex 3
		-1.2, 1,0, // vertex 4
		-1.2,-1,0  // vertex 1
	};
	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 1
		0,0,1, // color 2
		0,1,0, // color 3

		0,1,0, // color 3
		0.3,0.3,0.3, // color 4
		1,0,0  // color 1
	};
	box = create3DObject(GL_TRIANGLES,6,vertex_buffer_data,color_buffer_data,GL_FILL);
}
void createPipe(){
static const GLfloat vertex_buffer_data [] ={
		-20,-50,0, // vertex 1
		20,-50,0, // vertex 2
		20, 50,0, // vertex 3

		20, 50,0, // vertex 3
		-20, -50,0, // vertex 4
		-20,50,0  // vertex 1
	};
	static const GLfloat color_buffer_data [] = {
		0,0,0, // color 1
		0,0,0, // color 2
		0,0,0, // color 3

		0,0,0, // color 3
		0,0,0, // color 4
		0,0,0  // color 1
	};
	pipe = create3DObject(GL_TRIANGLES,6,vertex_buffer_data,color_buffer_data,GL_FILL);	
}
void createSpring(){
static const GLfloat vertex_buffer_data [] ={
		-10,-50,0, // vertex 1
		10,-50,0, // vertex 2
		10, 25,0, // vertex 3

		10, 25,0, // vertex 3
		-10, -50,0, // vertex 4
		-10,25,0,  // vertex 1

		-15,25,0,
		-15,25+10,0,
		15,25,0,

		-15,25+10,0,
		15,25,0,
		15,25+10,0
	};
	static const GLfloat color_buffer_data [] = {
		1,1,1, // color 1
		1,1,1, // color 2
		1,1,1, // color 3

		1,1,1, // color 3
		1,1,1, // color 4
		1,1,1,  // color 1

		1,1,1, // color 1
		1,1,1, // color 2
		1,1,1, // color 3

		1,1,1, // color 3
		1,1,1, // color 4
		1,1,1  // color 1
	};
	spring = create3DObject(GL_TRIANGLES,12,vertex_buffer_data,color_buffer_data,GL_FILL);	
}
void createCircle(float r){
	static GLfloat vbd[7000];
	static GLfloat cbd[7000];
	float px=0,py=0,cx=0,cy=0,i;
	int k=0,v=0,j=0;
	for(i =1;i<=360;i+=1.0){
		//printf("i:%f\n",i);
		v++;
		float tmp[]={0,0,0,r*cos(i*M_PI/180.0f),r*sin(i*M_PI/180.0f),0,r*cos((i-1.0)*M_PI/180.0f),r*sin((i-1.0)*M_PI/180.0f),0};
		for(int j=0;j<9;++j)vbd[k++]=tmp[j];
	}
	/*for(i=r;i>=0;i-=0.01){
		v++;
		cx=i;
		cy=-1.0*sqrt(r*r-i*i);
		if(cy==-0)cy=0;
		printf("cx: %f cy: %f\n",cx,cy);
		float tmp[]={cx,cy,0,px,py,0,0,0,0};
		for(int j=0;j<9;++j)vbd[k++]=tmp[j];
		px=cx,py=cy;
	}*/
	//float tmp[]={r,0,0,px,py,0,0,0,0};
	//for(int i=0;i<9;++i)vbd[k++]=tmp[i];
	//v++;
	for(int i=0;i<3*v;++i){
		cbd[j++]=1;
		cbd[j++]=0;
		cbd[j++]=0;
	}
	circle = create3DObject(GL_TRIANGLES,3*v,vbd,cbd,GL_FILL);
}
void createShape(){
	static GLfloat vbd[7000];
	static GLfloat cbd[7000];
	float px=0,py=0,cx=0,cy=0,i,r=20.0;
	int k=0,v=0,j=0;
	for(i =1;i<=360;i+=1.0){
		//printf("i:%f\n",i);
		v++;
		float tmp[]={r*cos(i*M_PI/180.0f),r*sin(i*M_PI/180.0f),0,r*cos((i-1.0)*M_PI/180.0f),r*sin((i-1.0)*M_PI/180.0f),0};
		for(int j=0;j<6;++j)vbd[k++]=tmp[j];
	}
	for(int i=0;i<2*v;++i){
			cbd[j++]=1;
			cbd[j++]=0;
			cbd[j++]=0;
		}
	shape = create3DObject(GL_POINTS,2*v,vbd,cbd,GL_FILL);
}
float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
float s = 1;
/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
	// clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);

	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (0, 0, 0);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);

	// Compute Camera matrix (view)
	// Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model

	// Load identity to model matrix
	//  Matrices.model = glm::mat4(1.0f);

	/* Render your scene */

	/*  glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
	    glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
	    glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
	    Matrices.model *= triangleTransform; 
	    MVP = VP * Matrices.model; // MVP = p * V * M

	//  Don't change unless you are sure!!
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	draw3DObject(triangle);*/

	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();
	gameground.draw();
	gamesky.draw();
	test.draw();

	// draw3DObject draws the VAO given to it using current MVP matrix
	
	
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateCircle = glm::translate (glm::vec3(-3.5*100,-3*100,0));
	//glm::mat4 rotateCircle = glm::rotate (glm::vec3(0,0,1))
	Matrices.model*=translateCircle;
	MVP = VP*Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
	draw3DObject(circle);
	
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translatePipe = glm::translate (glm::vec3(-3.5*0.9*100,-3*0.9*100,0));
	glm::mat4 rotatePipe = glm::rotate((float)(pipe_rot*M_PI/180.0f),glm::vec3(0,0,1));
	
	Matrices.model*=(translatePipe*rotatePipe);
	MVP = VP*Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
	//printf("%f\n",pipe_rot );
	draw3DObject(pipe);

	Matrices.model = glm::mat4(1.0f);
	glm::mat4 scaleSpring = glm::scale(glm::vec3(1,s,1));
	glm::mat4 translate = glm::translate(glm::vec3(0,s*50.0-50.0,0));
	glm::mat4 translateSpring = glm::translate (glm::vec3(-3.5*0.9*100,-3*0.9*100,0));
	glm::mat4 rotateSpring = glm::rotate((float)(pipe_rot*M_PI/180.0f),glm::vec3(0,0,1));
	Matrices.model*=(translateSpring*rotateSpring*translate*scaleSpring);
	MVP = VP*Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
	draw3DObject(spring);	
	// Increment angles
	//  float increments = 1;
	//pipe_rot+=1;
	//my.draw(0,0.25+0.01+0.15);
	gameground.checkCollision(my);
	test.checkCollision(my);
	float ang = pipe_rot*M_PI/180.0f;
	if(!my.isshoot)my.draw(0,25+10+15,s);
	else my.fire(s);
	
	//printf("ang: %f\n",ang);
	Matrices.model = glm::mat4(1.0f);
	//glm::mat4 translateBall = glm::translate(glm::vec3(-1.8+2*sin(ang),-2+2*cos(ang),0));
	//Matrices.model*=translateBall;
	MVP = VP*Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
	//my.move(-1.8+2*sin(ang),-2+2*cos(ang));
	//printf("%f %f\n",my.x,my.y);
	draw3DObject(shape);
	
	//camera_rotation_angle++; // Simulating camera rotation
	// triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
	// rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	   is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	/* Objects should be created before any other gl function and shaders */
	// Create the models
	//	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createCircle(0.5*100);
	createRectangle();
	my.x=my.y=0,my.r=0.15*100;
	my.create();
	gameground.create();
	gamesky.create();
	test.create();
	//createBox();
	createPipe();
	createSpring();
	createShape();
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 1300;
	int height = 1000;

	GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

	double last_update_time = glfwGetTime(), current_time;

	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {

		// OpenGL Draw commands
		draw();

		printf("fall %d\n",my.falling);
		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();
		if(glfwGetKey(window,GLFW_KEY_J)==GLFW_PRESS)pipe_rot+=1;
		if(glfwGetKey(window,GLFW_KEY_L)==GLFW_PRESS)pipe_rot-=1;
		//if(glfwGetKey(window,GLFW_KEY_P)==GLFW_PRESS)s*=0.9;

		// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
		current_time = glfwGetTime(); // Time in seconds
		if ((current_time - last_update_time) >= 10e-9) { // atleast 0.5s elapsed since last frame
			// do something every 0.5 seconds ..
			if(glfwGetKey(window,GLFW_KEY_P)==GLFW_PRESS){
				s*=0.99;
				my.vel*=my.k;
				my.translate = glm::translate(glm::vec3(0,s*50.0-50.0,0));
			}
			if(glfwGetKey(window,GLFW_KEY_M)==GLFW_PRESS){
				s/=0.99;
				my.translate = glm::translate(glm::vec3(0,s*50.0-50.0,0));
			}
			last_update_time = current_time;
		}
	}

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
