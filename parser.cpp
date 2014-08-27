/*  File: main.cpp
    Programmer: Byron Zaragoza
    Course: CSE 40166 - Computer Graphics, Fall 2013

    Instructions:
	Write a simple parser for .obj files based on what you've read about them. Your parser should read an obj
	file and store the vertices, texture vertices, normals, and faces into suitable data structures. Your
	parser should correctly parse the obj files attached at the bottom of this web page. Note: you may assume
	that the obj file will contain vertices and faces and, optionally, either normals or texture vertices or
	both. It need not contain either normals or texture vertices. Your program must read the file, and then
	print the number of vertices, normals, texture vertices, and faces.  Also compute and print the surface
	area of the mesh shape by adding up the areas of the polygonal faces. Compute the areas of the polygonal
	faces by computing the cross product of any two vectors that share a vertex as their start point and
	dividing by 2. Report the area.  Run the parser on every obj file attached below (including every obj
	file in the footbones zip file).  Submit source code and a README.txt file that contains the output of
	the parser (counts, area, etc.) for each input file (along with instructions to compile and run the
	program).
*/

#include <stdlib.h>
#include <math.h>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include "parser.h"

#define PI 3.14159265

using namespace std;

// PARSER FUNCTION
void Object::parseFile( int argc, char *argv ) {
    // Temporary variables.
    float tempFloat;
    string lineType;

    // File variables.
    ifstream file;
    string lineText;

    // Parse file into main variables.
    file.open( argv );
    while ( !file.eof() ){
	vector<float>		floatList;
	vector< vector<int> >	intListList;

	getline( file, lineText );
	if ( !lineText.empty() && lineText[0] != '#' && lineText[0] != 'g' ){
		lineType = lineText.substr( 0, lineText.find(' ') );
		lineText.erase( 0, lineText.find(' ') );
		lineText.erase( 0, lineText.find_first_not_of(' ') );
		lineText.erase( lineText.find_last_not_of(' ')+1, lineText.npos );

		if ( lineType.compare("v") == 0 ){
			while ( !lineText.empty() ){
				lineText.erase( 0, lineText.find_first_not_of(' ') );
				tempFloat = atof( (lineText.substr( 0, lineText.find(' ') ).c_str()) );
				lineText.erase( 0, lineText.find(' ') );

				floatList.push_back( tempFloat );
			}

			vertexList.push_back( floatList );
		} else if ( lineType.compare("vt") == 0 ){
			while ( !lineText.empty() ){
				lineText.erase( 0, lineText.find_first_not_of(' ') );
				tempFloat = atof( (lineText.substr( 0, lineText.find(' ') ).c_str()) );
				lineText.erase( 0, lineText.find(' ') );

				floatList.push_back( tempFloat );
			}

			textureList.push_back( floatList );
		} else if ( lineType.compare("vn") == 0 ){
			while ( !lineText.empty() ){
				lineText.erase( 0, lineText.find_first_not_of(' ') );
				tempFloat = atof( (lineText.substr( 0, lineText.find(' ') ).c_str()) );
				lineText.erase( 0, lineText.find(' ') );

				floatList.push_back( tempFloat );
			}

			normalList.push_back( floatList );
		} else if ( lineType.compare("f") == 0 ){
			while ( !lineText.empty() ){
				vector<int>	intList;

				string currentBlock = lineText.substr( 0, lineText.find(' ') );
				lineText.erase( 0, lineText.find(' ') );
				lineText.erase( 0, lineText.find_first_not_of(' ') );
				while ( !currentBlock.empty() ){
					if ( currentBlock.find('/') == currentBlock.npos ){
						tempFloat = atof( currentBlock.c_str() );
						currentBlock.clear();

						intList.push_back( tempFloat );
					} else if ( currentBlock[0] != '/' ){
						tempFloat = atof( currentBlock.substr( 0, currentBlock.find('/') ).c_str() );
						currentBlock.erase( 0, currentBlock.find('/')+1 );

						intList.push_back( tempFloat );
					} else if ( currentBlock[0] == '/' ){
						currentBlock.erase( 0, currentBlock.find_first_not_of('/') );

						intList.push_back( 0 );
					}
				}

				intListList.push_back( intList );
			}

			faceList.push_back( intListList );
		}
	}
    }
    file.close();

    cout << "Vertices: " << vertexList.size() << endl;
    cout << "Textures: " << textureList.size() << endl;
    cout << "Normals: " << normalList.size() << endl;
    cout << "Faces: " << faceList.size() << endl;
}
