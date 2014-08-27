/*  File: main.cpp
    Programmer: Byron Zaragoza
    Course: CSE 40166 - Computer Graphics, Fall 2013
*/

#ifdef __APPLE__			     // if compiling on Mac OS
	#include <GLUT/glut.h>
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else					     // else compiling on Linux OS
	#include <GL/glut.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

#include "parser.h"
#include "textures.h"

using namespace std;

// GLOBAL VARIABLES
	#define PI 3.14159265

	// WINDOW
	int windowWidth = 720, windowHeight = 720;			// Keep track of window width and height.

	// MOUSE
	GLint leftMouseButton = GLUT_UP, rightMouseButton = GLUT_UP;    // Status of the mouse buttons
	int mouseX = 0, mouseY = 0;					// Last known X and Y of the mouse

	// CAMERA
	float cameraTheta, cameraPhi, cameraRadius;			// Camera position in spherical coordinates
	float cameraX, cameraY, cameraZ;				// Camera position in cartesian coordinates
	float viewX, viewY, viewZ;					// Camera direction in cartesian coordinates
	float objX, objY, objZ;						// Object position in world coordinates

	// TEXTURES
	GLuint texHandle;
	int texWidth, texHeight;

	// SCREENSHOT
	unsigned char * pixels;						// Pixel buffer for screenshots.
	int frameno = 0;						// Keep track of framenumber for screenshots.

	// SKYBOX
	int skybox[6];
	int dimension = 1000;

	// INTERPOLATION RATE
	float IRATE = 10.0;

	// MODEL OBJECT
	Object modelStart;
	Object modelEnd;
	Object model;

	// BOOLEANS
	bool USETXT = false,
	     USEGRID = true,
	     USEBOX = false,
	     USELIN = false;

// REGISTER TEXTURE FUNCTION (skeleton by Jeffrey Paone)
bool registerOpenGLTexture(unsigned char *textureData, 
			   unsigned int texWidth, unsigned int texHeight, 
			   GLuint &textureHandle){
	if(textureData == 0) {
		fprintf(stderr,"Cannot register texture; no data specified.");
		return false;
	}

	// Generate texture handle and bind texture
	glGenTextures( 1, &textureHandle );
	glBindTexture( GL_TEXTURE_2D, textureHandle );

	// Set color mode
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	// GL_MODULATE or GL_DECAL
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);	// GL_MODULATE or GL_DECAL	
	
	// Set min/mag filters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  // a MIN Filter MUST BE SET
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
	// Set wrapping options
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// GL_REPEAT or GL_CLAMP
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
	GLenum errCode;
	const GLubyte *errString;
	
	if ((errCode = glGetError()) != GL_NO_ERROR) {
		errString = gluErrorString(errCode);
		fprintf(stderr,"Cannot register texture; OpenGL error: %s.", errString);
		return false;
	} 
	
	// Register image
	glTexImage2D( GL_TEXTURE_2D,		// specifying a 2D texture
		      0,			// level of the texture map - used for mipmapping
		      GL_RGB,			// internal format of image data
		      texWidth,			// texture width
		      texHeight,		// texture height
		      0,			// border width - used for filtering
		      GL_RGB,			// type of texels to use
		      GL_UNSIGNED_BYTE,		// how is the data stored in array
		      textureData		// pointer to array of image data
	);

	return true;
}

// READ .PPM FILE (skeleton by Patrick Flynn)
bool readPPM(string filename, int &imageWidth, int &imageHeight, unsigned char* &imageData) {
    FILE *fp = fopen(filename.c_str(), "r");
    int temp, maxValue;
    fscanf(fp, "P%d", &temp);
    if(temp != 6) {
        fprintf(stderr, "Error: PPM file is not of correct format! (Must be P6, is P%d.)\n", temp);
        fclose(fp);
        return false;
    }
 
    // Get file header
    fscanf(fp, "%d", &imageWidth);
    fscanf(fp, "%d", &imageHeight);
    fscanf(fp, "%d ", &maxValue);
 
    // Allocate buffer
    imageData = new unsigned char[imageWidth*imageHeight*3];
    if(!imageData) {
        fprintf(stderr, "Error: couldn't allocate image memory. Dimensions: %d x %d.\n", imageWidth, imageHeight);
        fclose(fp);
        return false;
    }
 
    fread(imageData,imageWidth*imageHeight*3,sizeof(unsigned char),fp);

    fclose(fp);
    return true;
}

// ROTATE-CAMERA FUNCTION
void updateCamera(float theta, float phi, float radius,
                            float &x, float &y, float &z) {
    x = radius * sinf(theta)*sinf(phi);
    y = radius * -cosf(phi);
    z = radius * -cosf(theta)*sinf(phi);
}

// STRAFE-CAMERA FUNCTION
void strafeCamera( char key ){
    float tempX = objX + viewX - cameraX;
    float tempY = objY + viewY - cameraY;
    float tempZ = objZ + viewZ - cameraZ;

    float x = -tempZ;
    float z = tempX;

    float magnitude = sqrt( (x * x) + (z * z) );

    x /= magnitude;
    z /= magnitude;

    if ( key == 'a'){
	cameraX -= x;
	cameraZ -= z;
	viewX -= x;
	viewZ -= z;
    } else if ( key == 'd'){
	cameraX += x;
	cameraZ += z;
	viewX += x;
	viewZ += z;
    }
}

// RESIZE-WINDOW FUNCTION
void resizeWindow( int w, int h ) {
    windowWidth = w;		             // Record new window width.
    windowHeight = h;			     // Record new window height.
    float aspectRatio = w / (float) h;

    // Allocate buffer to hold pixels for snapshots
    free(pixels);
    pixels = (unsigned char *)malloc(w*h*3*sizeof(unsigned char));
    if (!pixels) {
	perror("malloc");
	abort();
    }

    // Update projection matrix based on window size.
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective(45.0,aspectRatio,0.1,100000);

    // Update viewport and render the whole window
    glViewport( 0, 0, windowWidth, windowHeight );

    // Switch back to model view.
    glMatrixMode( GL_MODELVIEW );
}

// SCREENCAP FUNCTION
void screenCap(){
    // Save window contents if "p" is pressed used.
    glReadPixels( 0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels );
    char name[20];
    sprintf(name, "snapshot%d.ppm", frameno++);
    FILE *fp = fopen( name, "w" );
    if (!fp) {
	perror("fopen");
	abort();
    }
    fprintf(fp, "P6\n%d %d\n255\n", windowWidth, windowHeight);
    fwrite( pixels, windowWidth*windowHeight*3, sizeof(unsigned char), fp);
    fclose(fp);
}

// INITIALIZE SCENE FUNCTION
void initScene() {
    glEnable(GL_DEPTH_TEST);

    // Point light.
    float lightCol[4] = { 0.5, 0.5, 0.5, 1 };
    float ambientCol[4] = { 2.0, 2.0, 2.0, 1 };
    float lPosition[4] = { -0.75*dimension, dimension, 0.75*dimension, 1 };

    glLightfv( GL_LIGHT0, GL_DIFFUSE,  lightCol);
    glLightfv( GL_LIGHT0, GL_AMBIENT,  ambientCol);
    glLightfv( GL_LIGHT0, GL_POSITION, lPosition);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Enable smooth surfacing.
    glShadeModel(GL_SMOOTH);
}

// INITIALIZE TEXTURE FUNCTION
void initTexture( string fileName ) {
    unsigned char *texData;

    if( readPPM( fileName, texWidth, texHeight, texData ) )
	cout << "Successfully read in texture " << fileName << "." << endl;
	
    if( registerOpenGLTexture( texData, texWidth, texHeight, texHandle ) )
	cout << "Successfully registered texture with handle." << endl;
}

// INITIALIZE SKYBOX FUNCTION
void initSkybox(){
    skybox[0] = loadTexBMP("front.bmp");
    skybox[1] = loadTexBMP("right.bmp");
    skybox[2] = loadTexBMP("left.bmp");
    skybox[3] = loadTexBMP("back.bmp");
    skybox[4] = loadTexBMP("top.bmp");
    skybox[5] = loadTexBMP("bottom.bmp");
}

// GRID FUNCTION
void toggleGrid() {
    if ( USEGRID ){
	    glDisable(GL_TEXTURE_2D);
	    glDisable(GL_LIGHTING);

	    glColor3f(1,1,1);
	    glBegin(GL_LINES); {
		//draw the lines along the Z axis
		for(int i = 0; i < 11; i++) {
		    glVertex3f((i-5), 0, -5);
		    glVertex3f((i-5), 0, 5);
		}
		//draw the lines along the X axis
		for(int i = 0; i < 11; i++) {
		    glVertex3f(-5, 0, (i-5));
		    glVertex3f(5, 0, (i-5));
		}
	    }; glEnd();

	    glEnable(GL_LIGHTING);
    }
}

// TEXTURE FUNCTION
void toggleTex(){
    if ( USETXT ) {
	glEnable( GL_TEXTURE_2D );
	glBindTexture(  GL_TEXTURE_2D, texHandle );
    } else {
	glDisable( GL_TEXTURE_2D );
    }
}

// DRAW SKYBOX FUNCTION
void drawSkybox(){
    if ( USEBOX ){
	    glEnable( GL_TEXTURE_2D );
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	    glPushMatrix();{
		glTranslatef(cameraX+objX, cameraY+objY, cameraZ+objZ);

		glBindTexture(GL_TEXTURE_2D,skybox[0]);
		glBegin(GL_QUADS);{
			glTexCoord2f(0.1,0); glVertex3f( -dimension, -dimension, -dimension );
			glTexCoord2f(1,0); glVertex3f(  dimension, -dimension, -dimension );
			glTexCoord2f(1,0.99); glVertex3f(  dimension,  dimension, -dimension );
			glTexCoord2f(0.1,0.99); glVertex3f( -dimension,  dimension, -dimension );
		}; glEnd();

		glBindTexture(GL_TEXTURE_2D,skybox[2]);
		glBegin(GL_QUADS);{
			glTexCoord2f(0.1,0); glVertex3f(  dimension, -dimension, -dimension );
			glTexCoord2f(1,0); glVertex3f(  dimension, -dimension,  dimension );
			glTexCoord2f(1,0.99); glVertex3f(  dimension,  dimension,  dimension );
			glTexCoord2f(0.1,0.99); glVertex3f(  dimension,  dimension, -dimension );
		}; glEnd();

		glBindTexture(GL_TEXTURE_2D,skybox[3]);
		glBegin(GL_QUADS);{
			glTexCoord2f(0.1,0); glVertex3f(  dimension, -dimension,  dimension );
			glTexCoord2f(1,0); glVertex3f( -dimension, -dimension,  dimension );
			glTexCoord2f(1,0.99); glVertex3f( -dimension,  dimension,  dimension );
			glTexCoord2f(0.1,0.99); glVertex3f(  dimension,  dimension,  dimension );
		}; glEnd();

		glBindTexture(GL_TEXTURE_2D,skybox[1]);
		glBegin(GL_QUADS);{
			glTexCoord2f(0.1,0); glVertex3f( -dimension, -dimension,  dimension );
			glTexCoord2f(1,0); glVertex3f( -dimension, -dimension, -dimension );
			glTexCoord2f(1,0.99); glVertex3f( -dimension,  dimension, -dimension );
			glTexCoord2f(0.1,0.99); glVertex3f( -dimension,  dimension,  dimension );
		}; glEnd();

		glBindTexture(GL_TEXTURE_2D,skybox[4]);
		glBegin(GL_QUADS);{
			glTexCoord2f(0.1,0); glVertex3f( -dimension,  dimension, -dimension );
			glTexCoord2f(1,0); glVertex3f(  dimension,  dimension, -dimension );
			glTexCoord2f(1,0.99); glVertex3f(  dimension,  dimension,  dimension );
			glTexCoord2f(0.1,0.99); glVertex3f( -dimension,  dimension,  dimension );
		}; glEnd();

		glBindTexture(GL_TEXTURE_2D,skybox[5]);
		glBegin(GL_QUADS);{
			glTexCoord2f(0.1,0); glVertex3f(  dimension, -dimension, -dimension );
			glTexCoord2f(1,0); glVertex3f( -dimension, -dimension, -dimension );
			glTexCoord2f(1,1); glVertex3f( -dimension, -dimension,  dimension );
			glTexCoord2f(0.1,1); glVertex3f(  dimension, -dimension,  dimension );
		}; glEnd();
	    }; glPopMatrix();
    }
}

// DRAW OBJECT FUNCTION
void drawObject( Object obj ){
    vector<float> normal, texture, vertex;
    int nIndex, tIndex, vIndex;

    for ( int i = 0; i < obj.faceList.size(); ++i ){
	glBegin( GL_POLYGON ); {
		for ( int j = 0; j < obj.faceList[i].size(); ++j ){
			if ( obj.faceList[i][j].size() == 1 ){
				vIndex = obj.faceList[i][j][0]-1;
				vertex = obj.vertexList[vIndex];

				glVertex3f( vertex[0], vertex[1], vertex[2] );
			} else if ( obj.faceList[i][j].size() == 3 && obj.faceList[i][j][1] != 0 ){
				nIndex = obj.faceList[i][j][2]-1;
				tIndex = obj.faceList[i][j][1]-1;
				vIndex = obj.faceList[i][j][0]-1;

				normal = obj.normalList[nIndex];
				texture = obj.textureList[tIndex];
				vertex = obj.vertexList[vIndex];

				glNormal3f( normal[0], normal[1], normal[2] );
				glTexCoord2f( 1.0+texture[0], 1.0-texture[1] );
				glVertex3f( vertex[0], vertex[1], vertex[2] );
			} else if ( obj.faceList[i][j].size() == 3 ){
				nIndex = obj.faceList[i][j][2]-1;				
				vIndex = obj.faceList[i][j][0]-1;
				
				normal = obj.normalList[nIndex];
				vertex = obj.vertexList[vIndex];
			
				glNormal3f( normal[0], normal[1], normal[2] );
				glVertex3f( vertex[0], vertex[1], vertex[2] );
			}
		}
	}; glEnd();
    }
}

// LINEAR INTERPOLATION FUNCTION
void interpolate( Object modelBegin, Object modelStop ){
    vector<float> normal1, normal2,
		  texture1, texture2,
		  vertex1, vertex2;
    int n1Index, n2Index, nIndex,
	t1Index, t2Index, tIndex,
	v1Index, v2Index, vIndex;

    for ( float k = 0.0; k < IRATE; ++k ){
	    // Erase screen, set background to black, and clear RGB and depth buffer.
	    glClearColor( 0, 0, 0, 1 );
	    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	    glDrawBuffer( GL_BACK );

	    // Update model view matrix based on camera position.
	    glMatrixMode( GL_MODELVIEW );					// Do not change projection matrix.
	    glLoadIdentity();

	    gluLookAt(cameraX+objX, cameraY+objY, cameraZ+objZ,			// Camera is located at (x,y,z)
		objX+viewX, objY+viewY, objZ+viewZ,				// Camera is looking at object
		0.0f, 1.0f, 0.0f);						// Up vector is (0,1,0) (positive Y)

	    for ( int i = 0; i < model.faceList.size(); ++i ){
		for ( int j = 0; j < model.faceList[i].size(); ++j ){
			if ( model.faceList[i][j].size() == 1 ){
				v1Index = modelBegin.faceList[i][j][0]-1;
				v2Index = modelStop.faceList[i][j][0]-1;
				vIndex = model.faceList[i][j][0]-1;			

				vertex1 = modelBegin.vertexList[vIndex];
				vertex2 = modelStop.vertexList[vIndex];
			
				model.vertexList[vIndex][0] = vertex1[0]+(1/IRATE)*vertex2[0];
				model.vertexList[vIndex][1] = vertex1[1]+(1/IRATE)*vertex2[1];
				model.vertexList[vIndex][2] = vertex1[2]+(1/IRATE)*vertex2[2];
			} else if ( model.faceList[i][j].size() == 3 && model.faceList[i][j][1] != 0 ){
				t1Index = modelBegin.faceList[i][j][1]-1;
				t2Index = modelStop.faceList[i][j][1]-1;
				tIndex = model.faceList[i][j][1]-1;

				v1Index = modelBegin.faceList[i][j][0]-1;
				v2Index = modelStop.faceList[i][j][0]-1;
				vIndex = model.faceList[i][j][0]-1;
				
				texture1 = modelBegin.textureList[tIndex];
				texture2 = modelStop.textureList[tIndex];				
				vertex1 = modelBegin.vertexList[vIndex];
				vertex2 = modelStop.vertexList[vIndex];

				model.textureList[tIndex][0] = ((k+1.0)/IRATE)*(texture2[0]-texture1[0])+texture1[0];
				model.textureList[tIndex][1] = ((k+1.0)/IRATE)*(texture2[1]-texture1[1])+texture1[1];

				model.vertexList[vIndex][0] = ((k+1.0)/IRATE)*(vertex2[0]-vertex1[0])+vertex1[0];
				model.vertexList[vIndex][1] = ((k+1.0)/IRATE)*(vertex2[1]-vertex1[1])+vertex1[1];
				model.vertexList[vIndex][2] = ((k+1.0)/IRATE)*(vertex2[2]-vertex1[2])+vertex1[2];
			} else if ( model.faceList[i][j].size() == 3 ){		
				v1Index = modelBegin.faceList[i][j][0]-1;
				v2Index = modelStop.faceList[i][j][0]-1;
				vIndex = model.faceList[i][j][0]-1;
			
				vertex1 = modelBegin.vertexList[vIndex];
				vertex2 = modelStop.vertexList[vIndex];

				model.vertexList[vIndex][0] = vertex1[0]+(1/IRATE)*vertex2[0];
				model.vertexList[vIndex][1] = vertex1[1]+(1/IRATE)*vertex2[1];
				model.vertexList[vIndex][2] = vertex1[2]+(1/IRATE)*vertex2[2];	
			}
		}
	    }

	toggleGrid();
	toggleTex();
	drawObject(model);
	drawSkybox();
	
	glutSwapBuffers();
    }
}

// RENDER FUNCTION
void renderScene() {
    // Erase screen, set background to black, and clear RGB and depth buffer.
    glClearColor( 0, 0, 0, 1 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glDrawBuffer( GL_BACK );

    // Update model view matrix based on camera position.
    glMatrixMode( GL_MODELVIEW );					// Do not change projection matrix.
    glLoadIdentity();

    gluLookAt(cameraX+objX, cameraY+objY, cameraZ+objZ,			// Camera is located at (x,y,z)
	      objX+viewX, objY+viewY, objZ+viewZ,			// Camera is looking at object
	      0.0f, 1.0f, 0.0f);					// Up vector is (0,1,0) (positive Y)

    toggleGrid();
    toggleTex();
    if ( USELIN ){
	interpolate( modelStart, modelEnd );
	interpolate( modelEnd, modelStart );
    } else if ( !USELIN ){
	drawObject(modelStart);
    	drawSkybox();
    }

    glutSwapBuffers();
}

// HANDLE KEYPRESS FUNCTION
void handleKeypress ( unsigned char key, int x, int y ) {
    switch (key) {
	// GENERAL KEYS
	case 'b':
		USEBOX = !USEBOX;			// Toggle skybox.
		break;
	case 'g':					// Toggle grid.
		USEGRID = !USEGRID;
		break;
	case 'i':
		USELIN = !USELIN;			// Toggle linear interpolation.
		break;
	case 'p':					// Screencap.
		screenCap();
		break;
	case 'r':					// Reset default camera.
		cameraRadius = 12.0f;
		cameraTheta = PI;
		cameraPhi = 2.0;
		viewX = viewY = viewZ = 0.0f;
		updateCamera(cameraTheta, cameraPhi, cameraRadius,
			     cameraX, cameraY, cameraZ);
		break;
	case 't':					// Toggle texture.
		USETXT = !USETXT;
		break;
	case 27:					// ESCAPE KEY
	case 'q':					// Quit.
		exit(0);
		break;
	case '+':
		IRATE--;
		break;
	case '-':
		IRATE++;
		break;

	// CAMERA KEYS
	case 'c':					// Center object on camera.
		viewX = objX;
		viewY = objY;
		viewZ = objZ;
		break;
	case 'w':
		cameraRadius--;
		updateCamera(cameraTheta, cameraPhi, cameraRadius, cameraX, cameraY, cameraZ);
		break;
	case 's':
		cameraRadius++;
		updateCamera(cameraTheta, cameraPhi, cameraRadius, cameraX, cameraY, cameraZ);
		break;
	case 'a':
		strafeCamera('a');
		break;
	case 'd':
		strafeCamera('d');
		break;
	case ' ':
		cameraY++;
		viewY++;
		break;
	case 'e':
		cameraY--;
		viewY--;
		break;
    }
}

// MOUSE CLICK FUNCTION
void mouseClick( int button, int state, int x, int y ){
    if ( button == GLUT_LEFT_BUTTON )
	leftMouseButton = state;
    else if ( button == GLUT_RIGHT_BUTTON )
	rightMouseButton = state;

    mouseX = x;
    mouseY = y;
}

// MOUSE DRAG FUNCTION
void mouseDrag( int x, int y ) {
    if ( leftMouseButton == GLUT_DOWN ){
	cameraTheta += (x-mouseX)*0.005;
	cameraPhi += (y-mouseY)*0.005;

	updateCamera(cameraTheta, cameraPhi, cameraRadius, cameraX, cameraY, cameraZ);
    } else if ( rightMouseButton == GLUT_DOWN ){
	double totalChangeSq = (x-mouseX) + (y-mouseY);
	cameraRadius += totalChangeSq*0.01;

	updateCamera(cameraTheta, cameraPhi, cameraRadius, cameraX, cameraY, cameraZ);
    }

    mouseX = x;
    mouseY = y;
}

// TIMER FUNCTION
void displayTimer( int value ){
    glutPostRedisplay();				// Tell GLUT to redraw the display.
    glutTimerFunc( 1000.0/30.0, displayTimer, 0 );	// Loop timer function.
}

// MAIN FUNCTION
int main( int argc, char **argv ) {
    // Initialize GLUT.
    glutInit( &argc, argv );

    // Initialize window.
    glutInitDisplayMode( GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB );
    glutInitWindowPosition( 50, 50 );
    glutInitWindowSize( windowWidth, windowHeight );

    // Parse input file into variables.
    modelStart.parseFile( argc, argv[1] );
    modelEnd.parseFile( argc, argv[2] );
    model.parseFile( argc, argv[1] );

    // Create window.
    glutCreateWindow( "CSE 40166 - Final Project" );

    // Controls how OpenGL stores pixels in memory.
    glPixelStorei(GL_PACK_ROW_LENGTH,0);
    glPixelStorei(GL_PACK_SKIP_PIXELS,0);
    glPixelStorei(GL_PACK_ALIGNMENT,1);

    // Set camera position.
    cameraRadius = 12.0f;
    cameraTheta = 2.8;
    cameraPhi = 2.0;
    objX = objY = objZ = 0.0f;
    viewX = viewY = viewZ = 0.0f;

    updateCamera(cameraTheta, cameraPhi, cameraRadius, cameraX, cameraY, cameraZ);

    // Register functions
    glutDisplayFunc( renderScene );
    glutReshapeFunc( resizeWindow );
    glutKeyboardFunc( handleKeypress );
    glutMouseFunc( mouseClick );
    glutMotionFunc( mouseDrag );
    glutTimerFunc( 1000.0/30.0, displayTimer, 0 );	// Register timer function.

    // Initialize
    initScene();
    if (argv[3] != NULL)
    	initTexture(argv[3]);
    initSkybox();

    // Begin loop
    glutMainLoop();

    return 0;
}
