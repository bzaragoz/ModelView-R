# ifndef PARSER_H
# define PARSER_H

#include <vector>

using namespace std;

class Object {
	public:
		vector< vector<float> > vertexList;
		vector< vector<float> > textureList;
		vector< vector<float> > normalList;
		vector< vector< vector<int> > > faceList;
		void parseFile( int, char* );
};

# endif
