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
		unsigned 				index_;
		std::vector<Edge*> 		edges_; 			//Edges that compose the path
		double 					lengthPath_;		//Total length of all edges
		double					noisePath_;		//Total noise of all edges
		Demand*				    originalDemand_;	//Demand served by the path

	public:
		//Constructors
		Path(unsigned i = 0): index_(i) {}
		Path(std::vector<Edge*> & e);
		Path(std::vector<Edge*> & e, Demand* d);

		//Getters
		const unsigned &			getIndex() const {return index_;}
		const std::vector<Edge*> &	getEdges() const {return edges_;}		
		const double &				getLengthPath() const {return lengthPath_;}
		const double &				getNoisePath() const {return noisePath_;}
		const unsigned &			getIndexDemand() const;
        const Demand* 			    getDemand() const {return originalDemand_;}

        //Non constant getters
        Demand* 			        getDemand() {return originalDemand_;}
        std::vector<Edge*> &		getEdges()  {return edges_;}
		//Setters

        //Shows
        void                        showInVerticesPath();

        //Views
        std::ostream &              viewInVertices(std::ostream & flux);

		//Destructors
		~Path(){}
};

//Overloading flux operator to print object in screen
std::ostream & operator << (std::ostream & flux, const Path & p) ;
//Overloading == operator to compare objects easily
bool operator == (const Path& lhs, const Path& rhs);

#endif // PATH_HPP