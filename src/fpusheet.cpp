//g++ -o fpusim fpusim.cpp glut32.lib -lopengl32 -lglu32  //use this format to compile

/*
FPU simulation where vertical axis represents the degree of nonlinearity.
*/

#include <iostream>
#include <cstdlib>
#include <time.h>
#include <windows.h>
#include "GL/glut.h"
#include <math.h>
#include <vector>
using namespace std;

#define PARTICLE_NUM 81
#define INIT_VX 0
#define INIT_VY 0
#define INIT_M 1
#define RVAR 0
#define VVAR 0
#define MVAR 0
#define NONLIN .1
#define DISP_MODE 2
#define CUTOFF 1000

GLint winw=750,winh=750;
float TIME_INTERVAL=.001,STEPNUM=500;
GLfloat zoomscale=1,xoffset=0,yoffset=0;
char current_key;
bool paused=false;
int t=0;

float dist(float *r){
    return sqrt(r[0]*r[0]+r[1]*r[1]);
}

class histtype{
    public:
    vector<vector<float> > v;
    void add(float* r){
        vector<float> a;
        a.push_back(r[0]);
        a.push_back(r[1]);
        v.push_back(a);
    }
};

struct particletype{
    GLfloat r[2];
    float v[2];
    float m;
};

class manybody{
    public:
    vector<particletype> p;
    vector<histtype> phist;
    float nonlin,width;
    
    void reset(){
        //srand(1); //this line ensures that reset values are always the same
        int i,j;
        p.clear();
        for(i=0;i<PARTICLE_NUM;i++){
                particletype temp;
                temp.r[0]=(float)i/PARTICLE_NUM;
                //temp.r[1]=1-fabs(i-floor(PARTICLE_NUM/2))/(PARTICLE_NUM/2); //linear pluck
                temp.r[1]=exp(-pow((i-(float)floor(PARTICLE_NUM/2))/(width*PARTICLE_NUM/2.0),2)); //gaussian pluck
                
                temp.v[0]=0;
                temp.v[1]=0;
                temp.m=INIT_M+(((rand()%10000)/10000.0)*MVAR);
                p.push_back(temp);
            }
    }
    manybody(float nl,float w){
        nonlin=nl;
        width=w;
        reset();
    }
    
    void timeevolve(){
        int i,j;
        for(j=0;j<STEPNUM;j++){
        for(i=0;i<p.size();i++){
            float x1,x2;
            if(i==0)x1=0;
            else x1=p[i-1].r[1];
            if(i==p.size()-1)x2=0;
            else x2=p[i+1].r[1];
            
            float a=(x1+x2-(2.0*p[i].r[1]))*(1+(nonlin*(x2-x1)));
            p[i].v[1]+=a*TIME_INTERVAL;
            p[i].r[1]+=p[i].v[1]*TIME_INTERVAL;
        }
        }
        t++;
    }
    
    void disp(){
        int i;
        glColor3f(1,log(27*nonlin),log(2.718*nonlin));
        glBegin(GL_LINE_STRIP);
        for(i=0;i<p.size();i++){
            glVertex2fv(p[i].r);
        }
        glEnd();
    }
};

class mbsheettype{
    vector<manybody> mb;
    
    public:
    void reset(){
        int i;
        for(i=0;i<mb.size();i++)mb[i].reset();
    }
        
    mbsheettype(int num,float nonlin1,float nonlin2,float width1,float width2){
        int i;
        for(i=0;i<num;i++){
            manybody temp(((nonlin2-nonlin1)*((float)i/num))+nonlin1,((width2-width1)*((float)i/num))+width1);
            mb.push_back(temp);
        }
    }
    
    void timeevolve(){
        int i;
        for(i=0;i<mb.size();i++)mb[i].timeevolve();
    }
    
    void disp(){
        int i, j, mbsize=mb.size()-1, pnsize=PARTICLE_NUM-1;
        glBegin(GL_QUADS);
        for(i=0;i<mb.size()-1;i++){
            for(j=0;j<PARTICLE_NUM-1;j++){
                glColor3f((mb[i].p[j].r[1]+1)/2,(mb[i].p[j].r[1]+1)/2,(mb[i].p[j].r[1]+1)/2);
                glVertex2f((float)j/pnsize,(float)i/mbsize);
                
                glColor3f((mb[i].p[j+1].r[1]+1)/2,(mb[i].p[j+1].r[1]+1)/2,(mb[i].p[j+1].r[1]+1)/2);
                glVertex2f((float)(j+1)/pnsize,(float)i/mbsize);
                
                glColor3f((mb[i+1].p[j+1].r[1]+1)/2,(mb[i+1].p[j+1].r[1]+1)/2,(mb[i+1].p[j+1].r[1]+1)/2);
                glVertex2f((float)(j+1)/pnsize,(float)(i+1)/mbsize);
                
                glColor3f((mb[i+1].p[j].r[1]+1)/2,(mb[i+1].p[j].r[1]+1)/2,(mb[i+1].p[j].r[1]+1)/2);
                glVertex2f((float)j/pnsize,(float)(i+1)/mbsize);
                
                //cout<<"("<<mb[i].p[j].r[0]<<","<<(float)i/PARTICLE_NUM<<") ("<<mb[i+1].p[j].r[0]<<","<<(float)i/PARTICLE_NUM<<") ("<<mb[i+1].p[j+1].r[0]<<","<<(float)(i+1)/PARTICLE_NUM<<") ("<<mb[i].p[j+1].r[0]<<","<<(float)(i+1)/PARTICLE_NUM<<")\n";
                //system("pause");
            }
        }
        glEnd();
    }
}mbsheet(20,0,4,.1,.1);

void resize(int x,int y){
    if(x>0)winw=x;
    if(y>0)winh=y;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0,1,0,1);
}

void disp() 
{
    if(!paused){
	   glClear(GL_COLOR_BUFFER_BIT);		     // Clear Screen and Depth Buffer
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    
        glTranslatef(xoffset,yoffset,0);
        glScalef(1,zoomscale,1);
    
	   mbsheet.timeevolve();
	   mbsheet.disp();
	   glutSwapBuffers();
    }
}

void init() 
{
    srand(time(NULL));
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0,0,winw,winh);
	glClearColor(0.0, 0.0, 0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0,1,0,1);
}

void keyfunc(unsigned char key, int x, int y){
    switch(key){
        case '2':STEPNUM*=2; break;
        case '1':STEPNUM/=2; break;
        case '=':zoomscale*=1.4; break;
        case '-':zoomscale/=1.4; break;
        case 'p':paused=!paused;
    }
    cout<<"Stride:"<<STEPNUM<<", Zoom:"<<zoomscale<<"\n";
}
    
void mouse(int but,int state,int x,int y){
    if(but==GLUT_LEFT_BUTTON && state==GLUT_DOWN){
        mbsheet.reset();
    }
}

int main(int argc, char **argv) 
{
	glutInit(&argc, argv);                                      // GLUT initialization
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);  // Display Mode
	glutInitWindowSize(winw,winh);					// set window size
	glutInitWindowPosition(0,0);
	glutCreateWindow("Wave in field");								// create Window
	glutDisplayFunc(disp);									// register Display Function
	glutIdleFunc(disp);						
    glutMouseFunc(mouse);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyfunc);
	init();
	glutMainLoop();												// run GLUT mainloop
	return 0;
}
