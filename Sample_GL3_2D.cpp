#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <time.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <FTGL/ftgl.h>
#define sq(x) ((x)*(x))
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

struct FTGLFont {
	FTFont* font;
	GLuint fontMatrixID;
	GLuint fontColorID;
} GL3Font;

GLuint programID,fontProgramID;

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

glm::vec3 getRGBfromHue (int hue)
{
	float intp;
	float fracp = modff(hue/60.0, &intp);
	float x = 1.0 - abs((float)((int)intp%2)+fracp-1.0);

	if (hue < 60)
		return glm::vec3(1,x,0);
	else if (hue < 120)
		return glm::vec3(x,1,0);
	else if (hue < 180)
		return glm::vec3(0,1,x);
	else if (hue < 240)
		return glm::vec3(0,x,1);
	else if (hue < 300)
		return glm::vec3(x,0,1);
	else
		return glm::vec3(1,0,x);
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
float STX,STY;
float triangle_rot_dir = 1;
float rectangle_rot_dir = 1,pipe_rot=-52.0;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
float tx,ty,ti=0,MAXHEIGHT=500;
float s = 1;
bool ballinsky=false;   //whether ball in sky
float PANX = 0;
bool MANPAN=true,firsttime=true;
int BALLCOUNT=0;
struct obstacle;
obstacle *allobstacles;
void translateTargets();
typedef struct color{
	float r,g,b;
	color(float r,float g,float b):
	r(r),g(g),b(b) {}
}color;
VAO* createCircle(float r,color c){
	static GLfloat vbd[7000];
	static GLfloat cbd[7000];
	float px=0,py=0,cx=0,cy=0,i;
	int k=0,v=0,j=0;
	for(i =1;i<=360;i+=1.0){
		v++;
		float tmp[]={0,0,0,r*cos(i*M_PI/180.0f),r*sin(i*M_PI/180.0f),0,r*cos((i-1.0)*M_PI/180.0f),r*sin((i-1.0)*M_PI/180.0f),0};
		for(int j=0;j<9;++j)vbd[k++]=tmp[j];
	}
	for(int i=0;i<3*v;++i){
		cbd[j++]=c.r;
		cbd[j++]=c.g;
		cbd[j++]=c.b;
	}
	return create3DObject(GL_TRIANGLES,3*v,vbd,cbd,GL_FILL);
}
void drawCircle(VAO* circle,float x,float y){
	glm::mat4 MVP;
	glm::mat4 VP = Matrices.projection * Matrices.view;
	glm::mat4 tr = glm::translate(glm::vec3(x,y,0));
	Matrices.model = tr;
	MVP = VP*Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
	draw3DObject(circle);
}
VAO* createRectangle(int w,int h,color c){
	GLfloat vbd[]={
		-w/2.0,-h/2.0,0,
		w/2.0,-h/2.0,0,
		w/2.0,h/2.0,0,

		w/2.0,h/2.0,0,
		-w/2.0,-h/2.0,0,
		-w/2.0,h/2.0,0
	};
	GLfloat cbd[]={
		c.r,c.g,c.b,
		c.r,c.g,c.b,
		c.r,c.g,c.b,

		c.r,c.g,c.b,
		c.r,c.g,c.b,
		c.r,c.g,c.b	};
	return create3DObject(GL_TRIANGLES,6,vbd,cbd,GL_FILL);
}
typedef struct ball{
	float stx,sty;
	float sx,sy,x,y,vel,velx,vely,lu,st;
	float r,k,velx_in,vely_in;
	float rang,rs;
	bool isshoot,collision_obj,collision_ground,falling,power,shootpower;
	VAO *circle;
	GLfloat vbd[7000];
	GLfloat cbd[7000];
	glm::mat4 project;
	glm::mat4 translate;
	void init(){
		s=1;
		PANX=0;
		BALLCOUNT++;
		project = glm::mat4(1.0f);
		translate = glm::mat4(1.0f);
		ballinsky=isshoot=collision_ground=collision_obj=falling=power=false;
		MANPAN=shootpower=true;
		sx=sy=0;
		vel = 400;
		k=1.01;
	} 
	void create(color c){
		project = glm::mat4(1.0f);
		translate = glm::mat4(1.0f);
		collision_ground=collision_obj=falling=power=false;
		shootpower=true;
		sx=sy=0;
		vel = 400;
		k=1.01;
		//k=1+0.01/2.0;  //change k acc. to spring length
		circle = createCircle(r,c);
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
		if(!isshoot){
			project = glm::mat4(1.0f);
			rang = pipe_rot*M_PI/180.0f;
			rs=s;
		}
		glm::mat4 translateBall = translate*glm::translate(glm::vec3(nx,ny*rs,0));
		glm::mat4 rotateBall = glm::rotate((float)(rang),glm::vec3(0,0,1));
		glm::mat4 translateBallagain = glm::translate (glm::vec3(-3.5*0.9*115,-3*0.9*115,0));
		Matrices.model*=(project*translateBallagain*rotateBall*translateBall);
		float *mv = (&Matrices.model[0][0]);
		x =  mv[12];
		y =  mv[13];
		float z =  mv[14];
		float wp =  mv[15];

		x /= wp;
		y /= wp;
		z /= wp;
		printf("%f %f\n",x,y);
		if (onground()&&collision_ground){
			printf("onground\n");
			collision_ground=false;

		 }
		// if(abs(velx-0.0)<=(float)10e-10&&velx<=0){    //ball came to rest (Important buggy not coming to rest on top of an obstacle)
		// 	printf("at rest\n");
		// 	isshoot=ballinsky=false;
		// 	sx=sy=0;
		// 	return;
		// }
		MVP = VP*Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
		//x=nx,y=ny;
//		printf("x:%f y:%f\n",x,y);
		if(x>600){                        //to handle screen panning before drawing ball
			PANX=x-650+100;
		}
		draw3DObject(circle);	
	}
	void shoot(float ang){
		if(firsttime){
			firsttime=false;
			stx =x,sty=y;
			STX=stx,STY=sty;
		}
		float maxh;
		//float ang = -1.f*pipe_rot*M_PI/180.f;
		//ang = 0.5*M_PI - ang;
		//float vel = 500;
		printf("shooted vel:%f\n",vel);
		st = glfwGetTime();
		lu=glfwGetTime();
		isshoot=ballinsky=true;
		//sx=x,sy=y;
		MAXHEIGHT+=abs(sty);
		maxh = (vel*sin(ang))*(vel*sin(ang))/400.f;
		// if(maxh>=MAXHEIGHT){                         //handles going above the window
		// 	vel = sqrt(400.f*MAXHEIGHT)/sin(ang);
		// }
		velx=velx_in=vel*cos(ang),vely=vely_in=vel*sin(ang);
		//printf("in shoot velx:%f vely:%f\n",velx,vely);
	}
	void fire(float s){
		if(x>=1300||x<-650){       //ball out of window
			init();
			return;
		}
		float nx,ny;
		float ct;
		ti = glfwGetTime();
		if(ti-lu>=10e-10){
			ti-=st;
			//printf("sx: %f sy: %f\n",sx,sy);
			if(isshoot&&abs(velx-0.0)<=(float)10e-10&&velx<=0){    //ball came to rest (Important buggy not coming to rest on top of an obstacle)
				printf("at rest in fire\n");
				init();
				// isshoot=ballinsky=false;
				// sx=sy=0;
				return;
			}
			nx = sx+velx_in*ti;
			ny=sy+vely_in*ti-100*ti*ti;
			lu=glfwGetTime();
			vely = vely_in - 200*ti;
			vel = sqrt(velx*velx +vely*vely);
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
	double lasttime;
	VAO *shape;
	void create(){
		lasttime = double(-10e5);
		GLfloat vbd[]={
			-650,-500,0,
			1300,-500,0,
			1300,-100,0,

			1300,-100,0,
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
		float alpha=0.8,ang,beta=0.8;
		if(b.onground()&&b.falling&&!b.collision_ground){
			printf("delta %lf\n",glfwGetTime()-lasttime );
			if(abs(b.velx-0.0)<=(double)10e-18&&b.velx<=0||(glfwGetTime()-lasttime<=0.5)||(b.velx<=0&&abs(b.velx)<=0.1&&b.vely<0&&abs(b.vely)>=10)){    //ball came to rest (if time diff b/w two arrivals <=0.5 init())
				printf("at rest in ground\n");
				b.init();
				printf("new vel %f\n",b.vel);
				s=1;
				return;
			}
			lasttime=glfwGetTime();
			printf("collided ground x:%f y:%f \n",b.x,b.y);
			b.collision_ground=b.falling=true;
			b.sx=b.x-b.stx,b.sy=b.y-b.sty;
			b.vel = sqrt((beta*beta*b.velx*b.velx + alpha*alpha*b.vely*b.vely));  //alpha of collision = 1/600
			if(b.velx>0)ang = atan(-1*alpha*b.vely/b.velx*beta);
			else ang = M_PI/2.0 + atan(abs(b.velx*beta)/abs(alpha*b.vely));
			b.shoot(ang);  //angle is hard-coded for test
		}
	}

}ground;
typedef struct sky{
	VAO *shape;
	void create(){
		GLfloat vbd[]={
			-650,-100,0,
			1300,-100,0,
			1300,500,0,

			1300,500,0,
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
	glm::mat4 translate,translateagain,scale;
	float w,h;   //width and height
	float x,y,r; 
	bool circle;     //whether circle
	bool collision,dir,target,available;
	int numhit;  //no. of times hitted 
	void create(int wi,int he,color c,bool cir,bool tar){
		available=dir=true;
		circle=cir,target=tar;
		collision=false;
		translateagain=scale=glm::mat4(1.0f);
		translate=glm::mat4(1.0f);
		x=y=0;
		numhit=-1;
		
		if(!circle){
			w=wi,h=he;
			//r = sqrt((w/2.0)*(w/2.0) + (h/2.0)*(h/2.0));
			shape = createRectangle(w,h,c);
		}
		else{
			r=wi;
			shape = createCircle(r,c);
		}
	}
	void draw(){
		glm::mat4 MVP;
		glm::mat4 VP = Matrices.projection * Matrices.view;
		Matrices.model = glm::mat4(1.0f);
		 //= glm::translate(glm::vec3(500,-300,0));
		Matrices.model*=(translateagain*translate*scale);
		float *mv = (&Matrices.model[0][0]);
		x =  mv[12];
		y =  mv[13];
		float z =  mv[14];
		float wp =  mv[15];

		x /= wp;
		y /= wp;
		z /= wp;
		MVP = VP*Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
		draw3DObject(shape);
	}
	
	void checkCollision(ball &b){
		float delta=10.0,alpha;
		if(b.x>=x-w/2.0-b.r-delta&&b.x<=x+w/2.0+b.r+delta&&b.y>=y-h/2.0-b.r-delta&&b.y<=y+h/2.0+b.r+delta&&!collision&&b.isshoot){
			if(abs(b.velx-0.0)<=(double)10e-18&&b.velx<=0||(b.velx<=0&&abs(b.velx)<=0.1&&b.vely<0&&abs(b.vely)>=10)){
				return;
			}
			float ang;
			//printf("obscollided x:%f y:%f ang:%f\n",b.x,b.y,atan(b.vely/b.velx));
			collision=true;
			for(int i=0;i<2;++i){
				if(allobstacles[i].x!=x&&allobstacles[i].y!=y){
					allobstacles[i].collision=false;            //set collision with other obstacles as false
			//		printf("entered %f %f\n",x,y);
				}
			}
			b.sx=b.x-b.stx,b.sy=b.y-b.sty;
			if(b.x<=x-w/2.0){                     //left
			//	printf("left velx:%f vely:%f\n",b.velx,b.vely);
				float tmp = atan(b.velx/abs(b.vely));
				ang = M_PI/2.0 + tmp;
				if(b.vely<0)ang*=-1.0;
			}
			else if(b.vely>0){
			//	printf("bottom velx:%f vely:%f\n",b.velx,b.vely);
				ang = -1.0*atan(b.vely/b.velx);
				//ang = -1.0*M_PI/4.0;
			}
			else if(b.vely<0){
			//	printf("top collision\n");
				ang = atan(abs(b.vely)/b.velx);
			}
			b.vel = (b.velx*b.velx + b.vely*b.vely)/600;
			b.shoot(ang);
		}
	}
	void hit(ball b){
		float d = sqrt((b.x-x)*(b.x-x) + (b.y-y)*(b.y-y));
		float s=0.5;
		if(d<=r+b.r&&available){
			if(numhit==-1){
				numhit=BALLCOUNT;
				scale = glm::scale(glm::vec3(s,s,0));
				r*=s;
			}
			else if(numhit!=BALLCOUNT)available=false;
		}	
	}
	void move(float vel){
		float nx,ny,MAXH=200,MINH=-200;
		
		if(!dir){
			ny =y-vel,nx=0;
		}
		if(dir){
			nx=0,ny=y+vel;
		}
		if(ny>=MAXH&&dir){
			dir=0;
	//		printf("godown\n");
		}
		if(ny<=MINH&&!dir){
			dir=1;
	//		printf("goup\n");
		}
	//	printf("ny :%f\n",ny);
		translateagain = glm::translate(glm::vec3(nx,ny,0));
	}
	
}obstacle;
typedef struct power{       //singleton,only one instance needed
	float x,y,r;
	float inx,iny,inti;  //parameters to be set when ball clicked(or power fired)
	bool available;
	int type;       //1-fire 2-speedup
	VAO *circle;
	GLfloat vbd[7000];
	GLfloat cbd[7000];
	glm::mat4 translate;
	void create(float ra,int ty){
		type=ty;
		r = ra;
		available=true;
		circle = createCircle(r,color(0,0,0));
		return;	
	}
	void draw(){
		if(!available)return;
		float ti = glfwGetTime()-inti;
		glm::mat4 MVP;
		glm::mat4 VP = Matrices.projection * Matrices.view;
		Matrices.model = glm::mat4(1.0f);
		translate = glm::translate(glm::vec3(inx,iny-100.0*ti*ti,0));
		Matrices.model*=translate;
		float *mv = (&Matrices.model[0][0]);
		x =  mv[12];
		y =  mv[13];
		float z =  mv[14];
		float wp =  mv[15];

		x /= wp;
		y /= wp;
		z /= wp;
		MVP = VP*Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
		draw3DObject(circle);	
	}
	void hit(obstacle &o){
		float d = sqrt(sq(x-o.x)+sq(y-o.y));
		if(d<=r+o.r){
			available=o.available=false;
		}
	}
}power;
bool checkCollisionCircle(ball b,obstacle o){
	float d = sqrt((b.x-o.x)*(b.x-o.x) + (b.y-o.y)*(b.y-o.y));
	return d<=b.r+o.r;
}
bool checkCollisionRect(ball b,obstacle o){
	if(b.x>=o.x-o.w/2.0-b.r&&b.x<=o.x+o.w/2+b.r&&b.y>=o.y-o.h/2.0-b.r&&b.y<=o.y+o.h/2.0+b.r)
		return true;
	return false;
}
void handleCollisionCircle(ball &b,obstacle &o){
//	if(b.collision_obj)return;
	float phi,theta,alpha;  //phi = angle with x-axis line joining both centres and theta = angle of velocity vector of ball
	float vn,vt,a = 1.0;
	theta = b.vely/b.velx;
	phi = (o.y-b.y)/(o.x-b.x);
	alpha = theta - phi;
	vn = b.vel*cos(alpha),vt=b.vel*sin(alpha);
	float beta = (vt*cos(phi)-a*vn*sin(phi))/(vt*sin(phi)+a*vn*cos(phi));
	beta = atan(beta);
	b.vel = sqrt(vt*vt + a*a*vn*vn);
	b.collision_obj = true;
	b.sx=b.x-b.stx,b.sy=b.y-b.sty;
	b.shoot(M_PI-beta);
}
void handleCollisionRect(ball &b,obstacle &o){
	float ang,alpha=1;
	if(b.x<=o.x-o.w/2-b.r){
		ang = M_PI/2.0 + atan(alpha*b.velx/b.vely);
		b.vel = sqrt(sq(b.vely)+sq(alpha*b.velx));
	}
	else if(b.y>=o.y+o.h/2+b.r){
		//top
	}
	else if(b.x>=o.x+o.w/2+b.r){
		//right side
	}
	else if(b.y<=o.y-o.h/2-o.r){
		ang = -1.0*atan(alpha*b.vely/b.velx);
		b.vel = sqrt(sq(alpha*b.vely)+sq(b.velx));
	}
	b.collision_obj=true;
	b.sx=b.x-b.stx,b.sy=b.y-b.sty;
	b.shoot(ang);
}
ball my;
ball powerball[3];
ground gameground;
sky gamesky;
obstacle test,test2;
power testpow;
float ang,ZOOM=1.0;
float add = 0;
int OBSTACLES=0,LIFES=5;
VAO* life[10];
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_UP:
				ZOOM/=2;
				break;
			case GLFW_KEY_DOWN:
				ZOOM*=2;
				break;
			case GLFW_KEY_P:
				triangle_rot_status = !triangle_rot_status;
				break;
			
			case GLFW_KEY_SPACE:
				ang = -1.f*pipe_rot*M_PI/180.f;  //don't mess with ang
				ang = 0.5*M_PI - ang;
				if(!ballinsky&&LIFES>0){
					s=1;
					my.shoot(ang);           
					my.shootpower=true;
					MANPAN=false;
					LIFES--;  
				}
				break;
			case GLFW_KEY_1:
				if(my.shootpower){
					my.power=true;
					testpow.inx = my.x;
					testpow.iny = my.y;
					testpow.inti=glfwGetTime();
					my.shootpower=false;
				}
				break;
			case GLFW_KEY_2:
				if(my.shootpower){
					my.sx+=150.0*cos(atan(my.vely/my.velx));
					my.sy+=150.0*sin(atan(my.vely/my.velx));
					my.shootpower=false;
				}
				break;
			case GLFW_KEY_3:
				if(my.isshoot&&my.shootpower){
					for(int i=0;i<2;++i){
						printf("myx:%f myy:%f\n",my.x,my.y);
						powerball[i].sx=my.x-STX;
						powerball[i].sy=my.y-STY;
						powerball[i].stx=STX;
						powerball[i].sty=STY;
						powerball[i].vel=300;
						if(i==0)powerball[i].shoot(atan(my.vely/my.velx));
						else powerball[i].shoot(-1*atan(my.vely/my.velx));
					}
					
				}
				my.shootpower=false;
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
//mouse cursorPosition callback
void poscallback(GLFWwindow* window,double xp,double yp){
	double lx,ly;
	ly=1000.0/2.0 + 3.5*115-yp;
	lx=xp-1300.0/2.0+3.5*115;
	pipe_rot=-1.0*atan(lx/ly)*180.0f/M_PI;
}
//mouse scroll-bar callback
double currentYoffset=0;
void scrollcallback(GLFWwindow* window,double xoffset,double yoffset){
	//printf("yoff:%lf\n",yoffset);
	currentYoffset+=yoffset;
	if(currentYoffset>0)ZOOM = ZOOM /(2*currentYoffset);
	else if(currentYoffset==0)ZOOM=1;
	else ZOOM = ZOOM = ZOOM*2*abs(currentYoffset);
	//ZOOM/=yoffset;
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
	glViewport (0, 0, (GLsizei) fbwidth+add, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	// Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	//Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
	Matrices.projection = glm::ortho(-650.0f*ZOOM, 650.0f*ZOOM, -500.0f*ZOOM, 500.0f*ZOOM, 0.1f, 500.0f);
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
void clearcollisions(ball b){    //while colliding with an object clear collisions
	float delta = 10.0;
	for (int i = 0; i < OBSTACLES; ++i)
	{	float x=allobstacles[i].x;
		float y=allobstacles[i].y;
		float w=allobstacles[i].w;
		float h=allobstacles[i].h;
		// if(b.x>=x-w/2.0-b.r&&b.x<=x+w/2.0+b.r&&b.y>=y-h/2.0-b.r&&b.y<=y+h/2.0+b.r==0)
		// 	allobstacles[i].collision=false;
		if(b.x<=x-w/2.0-b.r-delta||b.x>=x+w/2.0+b.r+delta||b.y<=y-h/2.0-b.r-delta||b.y>=y+h/2.0+b.r+delta)
			if(allobstacles[i].collision)allobstacles[i].collision=false;
	}
}
float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
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
	glm::vec3 target (0+PANX, 0, 0);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);

	// Compute Camera matrix (view)
	// Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	Matrices.view = glm::lookAt(glm::vec3(0+PANX,0,3), target, glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

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
	translateTargets();               //to scale and translate targets
	for(int i=0;i<OBSTACLES;++i){
		if(!allobstacles[i].target||(allobstacles[i].target&&allobstacles[i].available))
			allobstacles[i].draw();
	}
	

	// draw3DObject draws the VAO given to it using current MVP matrix
	
	
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateCircle = glm::translate (glm::vec3(-3.5*115,-3*115,0));
	//glm::mat4 rotateCircle = glm::rotate (glm::vec3(0,0,1))
	Matrices.model*=translateCircle;
	MVP = VP*Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
	draw3DObject(circle);
	
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translatePipe = glm::translate (glm::vec3(-3.5*0.9*115,-3*0.9*115,0));
	glm::mat4 rotatePipe = glm::rotate((float)(pipe_rot*M_PI/180.0f),glm::vec3(0,0,1));
	
	Matrices.model*=(translatePipe*rotatePipe);
	MVP = VP*Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
	//printf("%f\n",pipe_rot );
	draw3DObject(pipe);

	Matrices.model = glm::mat4(1.0f);
	glm::mat4 scaleSpring = glm::scale(glm::vec3(1,s,1));
	glm::mat4 translate = glm::translate(glm::vec3(0,s*50.0-50.0,0));
	glm::mat4 translateSpring = glm::translate (glm::vec3(-3.5*0.9*115,-3*0.9*115,0));
	glm::mat4 rotateSpring = glm::rotate((float)(pipe_rot*M_PI/180.0f),glm::vec3(0,0,1));
	Matrices.model*=(translateSpring*rotateSpring*translate*scaleSpring);
	MVP = VP*Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
	draw3DObject(spring);	
	
	for(int i=0;i<LIFES;++i){
		drawCircle(life[i],-620+35*i,480);
	}

	gameground.checkCollision(my);
	for(int j=0;j<2;++j)gameground.checkCollision(powerball[j]);  //check with other(power) balls
	// for(int i=0;i<OBSTACLES;++i){
	// 	if(!allobstacles[i].target)allobstacles[i].checkCollision(my);
	// 	else allobstacles[i].hit(my);
	// }
	// allobstacles[0].checkCollision(my);
	// allobstacles[1].checkCollision(my);
	
	float ang = pipe_rot*M_PI/180.0f;
	if(!my.isshoot&&LIFES>0)my.draw(0,25+10+15,s);
	else my.fire(s);
	for(int i=0;i<2;++i){
		if(powerball[i].isshoot){
			powerball[i].fire(s);
			printf("%d ball fired\n",i+1);
		}
	}
	if(my.power){
		testpow.draw();
		for(int i=0;i<OBSTACLES;++i)testpow.hit(allobstacles[i]);
	}

	for(int i=0;i<OBSTACLES;++i){
		if(!allobstacles[i].target){
			allobstacles[i].checkCollision(my);       //with main ball
			for(int j=0;j<2;++j)allobstacles[i].checkCollision(powerball[j]);  //with power balls
		}
		else {
			allobstacles[i].hit(my);                    //main ball
			for(int j=0;j<2;++j)allobstacles[i].hit(powerball[j]);       //power balls
		}
	}
	//printf("ang: %f\n",ang);
	Matrices.model = glm::mat4(1.0f);
	//glm::mat4 translateBall = glm::translate(glm::vec3(-1.8+2*sin(ang),-2+2*cos(ang),0));
	//Matrices.model*=translateBall;
	MVP = VP*Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);

	// // Render font on screen
	// static int fontScale = 0;
	// float fontScaleValue = 0.75 + 0.25*sinf(fontScale*M_PI/180.0f);
	// glm::vec3 fontColor = getRGBfromHue (fontScale);



	// // Use font Shaders for next part of code
	// glUseProgram(fontProgramID);
	// Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// // Transform the text
	// Matrices.model = glm::mat4(1.0f);
	// glm::mat4 translateText = glm::translate(glm::vec3(-3,2,0));
	// glm::mat4 scaleText = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	// Matrices.model *= (translateText * scaleText);
	// MVP = Matrices.projection * Matrices.view * Matrices.model;
	// // send font's MVP and font color to fond shaders
	// glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	// glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);

	// // Render font
	// GL3Font.font->Render("Round n Round we go !!");
//	drawCircle(powerball,-3.5*115-70.0,-3*115);
	//my.move(-1.8+2*sin(ang),-2+2*cos(ang));
	//printf("%f %f\n",my.x,my.y);
	//draw3DObject(shape);
	// if(checkCollisionRect(my,test)){
	// 	handleCollisionRect(my,test);
	// }
	// if(checkCollisionCircle(my,test2)){
	// 	handleCollisionCircle(my,test2);
	// }
	
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
	GLFWcursor* cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);

	glfwSetCursor(window, cursor);
	glfwSetCursorPosCallback(window,poscallback);
	glfwSetScrollCallback(window,scrollcallback);
	return window;
}
void initlife(){
	for(int i=0;i<5;++i)life[i]=createCircle(15,color(0,0,1));
}

void createFloors(){
	allobstacles[2].create(400,50,color(1,0,0),false,false);          //floor 0
	allobstacles[2].translate=glm::translate(glm::vec3(700,-300,0));

	allobstacles[5].create(150,50,color(1,0,0),false,false);            //floor 1
	allobstacles[5].translate = glm::translate(glm::vec3(950,-150,0));
	allobstacles[7].create(150,50,color(1,0,0),false,false);              //floor 2
	allobstacles[7].translate=glm::translate(glm::vec3(700,-50,0));

	allobstacles[9].create(400,50,color(1,0,0),false,false);               //floor 3
	allobstacles[9].translate=glm::translate(glm::vec3(800,150,0));
}
void createTargets(){
	allobstacles[3].create(50.0,50.0,color(0,1,0),true,true);
	allobstacles[4].create(50.0,50.0,color(0,1,0),true,true);

	allobstacles[6].create(50.0,50.0,color(0,1,0),true,true);

	allobstacles[8].create(50.0,50.0,color(0,1,0),true,true);

	allobstacles[10].create(50.0,50.0,color(0,1,0),true,true);
	allobstacles[11].create(50.0,50.0,color(0,1,0),true,true);
}
void translateTargets(){
	allobstacles[3].translate = glm::translate(glm::vec3(550,-300+25+allobstacles[3].r,0));  //floor 0
	allobstacles[4].translate = glm::translate(glm::vec3(800,-300+25+allobstacles[4].r,0));  //floor 0

	allobstacles[6].translate = glm::translate(glm::vec3(950,-150+25+allobstacles[6].r,0));  //floor 1

	allobstacles[8].translate = glm::translate(glm::vec3(700,-50+25+allobstacles[8].r,0));   //floor 2

	allobstacles[10].translate = glm::translate(glm::vec3(700,150+25+allobstacles[10].r,0));  //floor 3
	allobstacles[11].translate = glm::translate(glm::vec3(950,150+25+allobstacles[11].r,0));  //floor 3



}
void initObjects(){           //improve
	my.x=my.y=0,my.r=0.15*100;
	my.create(color(0,0,1));
	for(int i=0;i<2;++i)powerball[i].r=15;
		for(int i=0;i<2;++i)powerball[i].create(color(0.309,0.047,0.96));
	gameground.create();
	gamesky.create();
	OBSTACLES = 12;
	float r = 50;
	createFloors();
	createTargets();
	//translateTargets();
	allobstacles[0].create(50.0,200.0,color(1,0,0),false,false);      //movable
	allobstacles[0].translate = glm::translate(glm::vec3(-100,0,0));
	allobstacles[1].create(100.0,100.0,color(0,1,0),false,false);        //movable
	allobstacles[1].translate=glm::translate(glm::vec3(100,0,0));
	testpow.create(10.0,3);
	createPipe();
	createSpring();
}
/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	/* Objects should be created before any other gl function and shaders */
	// Create the models
	//	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	circle = createCircle(0.5*100,color(1,0,0));      //canon
	initObjects();
	initlife();
	//createRectangle();
	
	//createShape();
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

	// Initialise FTGL stuff
	// const char* fontfile = "arial.ttf";
	// GL3Font.font = new FTExtrudeFont(fontfile); // 3D extrude style rendering

	// if(GL3Font.font->Error())
	// {
	// 	cout << "Error: Could not load font `" << fontfile << "'" << endl;
	// 	glfwTerminate();
	// 	exit(EXIT_FAILURE);
	// }

	// // Create and compile our GLSL program from the font shaders
	// fontProgramID = LoadShaders( "fontrender.vert", "fontrender.frag" );
	// GLint fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform;
	// fontVertexCoordAttrib = glGetAttribLocation(fontProgramID, "vertexPosition");
	// fontVertexNormalAttrib = glGetAttribLocation(fontProgramID, "vertexNormal");
	// fontVertexOffsetUniform = glGetUniformLocation(fontProgramID, "pen");
	// GL3Font.fontMatrixID = glGetUniformLocation(fontProgramID, "MVP");
	// GL3Font.fontColorID = glGetUniformLocation(fontProgramID, "fontColor");

	// GL3Font.font->ShaderLocations(fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform);
	// GL3Font.font->FaceSize(1);
	// GL3Font.font->Depth(0);
	// GL3Font.font->Outset(0, 0);
	// GL3Font.font->CharMap(ft_encoding_unicode);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 1300;
	int height = 1000;
	float tmp=0;
	allobstacles = new obstacle[20];
	GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

	double last_update_time = glfwGetTime(), current_time;

	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {
		reshapeWindow(window,width,height);        //handle zoom
		// OpenGL Draw commands
		draw();
		clearcollisions(my);
		//printf("%lf %lf \n",xp,yp);
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
			// double xp,yp,lx,ly;
			// glfwGetCursorPos(window,&xp,&yp);
			// ly=height/2.0 + abs(my.y)-yp;
			// lx=xp-width/2.0+abs(my.x);
			// //printf("lx:%lf ly:%lf\n",lx,ly );
			// pipe_rot=M_PI/2.0+atan(ly/lx)*180.0f/M_PI;

			allobstacles[0].move(1.0);
			allobstacles[1].move(2.0);
			if(glfwGetKey(window,GLFW_KEY_A)==GLFW_PRESS){
				if(PANX>0&&MANPAN)PANX-=10.0;
			}
			if(glfwGetKey(window,GLFW_KEY_D)==GLFW_PRESS){
				if(PANX<650&&MANPAN)PANX+=10.0;
			}
			if(glfwGetKey(window,GLFW_KEY_P)==GLFW_PRESS){
				s*=0.99;
				if(!my.isshoot)my.vel*=my.k;
				my.translate = glm::translate(glm::vec3(0,my.rs*50.0-50.0,0));
			}
			if(glfwGetKey(window,GLFW_KEY_M)==GLFW_PRESS){
				if(s<1){
					s/=0.99;
					my.translate = glm::translate(glm::vec3(0,my.rs*50.0-50.0,0));
				}
			}
			last_update_time = current_time;
		}
	}

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
