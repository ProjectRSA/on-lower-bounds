#ifndef PATH_HPP
#define PATH_HPP
#include <stdio.h>
#include <vector>
#include <iostream>

//Forward declaration
class Edge;
class Demand;

class Path
{
	private:
		std::vector<Edge*> 		edges_; 			//Edges that compose the path
		double 					lengthPath_;		//Total length of all edges
		double					noisePath_;		//Total noise of all edges

	public:
		//Constructors
		Path(std::vector<Edge*> & e);
		Path(std::vector<Edge*> & e, Demand* d);

		//Getters
		const std::vector<Edge*> &	getEdges() const {return edges_;}		
		const double &				getLengthPath() const {return lengthPath_;}
		const double &				getNoisePath() const {return noisePath_;}
        //Non constant getters
        std::vector<Edge*> &		getEdges()  {return edges_;}
		void reorient(int origin_index);

        //Shows
        std::ostream &              viewOriented(std::ostream & flux, int index_vertex);
		

		//Destructors
		~Path(){}
};

//Overloading flux operator to print object in screen
std::ostream & operator << (std::ostream & flux, const Path & p) ;
//Overloading == operator to compare objects easily
bool operator == (const Path& lhs, const Path& rhs);

#endif // PATH_HPP