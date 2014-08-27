--------------------------
Instructions for Compiling
--------------------------
1) Unpack the .tar file and place all of its file within the same directory.
   
   	File list:
	.cpp - main, parser, textures
	.h   - parser, textures
	.bmp - front, back, right, left, top, bottom
	.obj - waddles_neutral, waddles_sassy
	.ppm - waddles_texture_with_occlusion

2) Compile the program with the following command.

	g++ main.cpp parser.cpp textures.cpp -o main -lGL -lGLU -lglut

3) Run the program with the following command.

	./main waddles_neutral.obj waddles_sassy.obj waddles_texture_with_occlusion.ppm

   (First argument is initial keyframe. Second is ending keyframe. Third is optional texture.)

-----------------
Keyboard Commands
-----------------
General
	B – Toggle skybox		G – Toggle grid
	P – Capture screen		I – Toggle animation/linear interpolation
	T – Toggle .obj texture		Q – Quit
	PLUS – Speed up animation	MINUS – Slow down animation

Camera
	A – Strafe left			D – Strafe right
	SPACE – Move up			E – Move down
	W – Zoom in			D – Zoom out
	R – Reset default		C – Center object on screen
	LEFT MOUSE – Rotate camera around object.
	RIGHT MOUSE – Zoom in or out.