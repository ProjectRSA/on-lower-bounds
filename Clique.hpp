#ifndef CLIQUE_HPP
#define CLIQUE_HPP
#include <stdio.h>
#include <vector>
#include <iostream>

using namespace std;

//Forward declaration
class Vertex;
class Demand;
class Path;

struct forbiddenEdgesFromCliques
{
	std::vector<Demand*> 	indexDemand;	// The demands that share this edge
	Edge*					edges;  		// The shared edge (used to build the constraint tha will forbid the clique)
};

class Clique
{
	private:
		std::vector<Vertex*>					vertices_;
		unsigned								index_;
		unsigned 								size_;
		unsigned								weight_;  						//sum of weights of composing vertices
		std::vector<forbiddenEdgesFromCliques>	forbiddenEdgesFromCliques_; 	
		std::vector<Path*>						paths_;								//Paths that originated clique
		std::vector<string>						pathsLabels_;						//Paths that originated clique

	public:
		//Constructors
		Clique(unsigned i = 0, unsigned s = 0, unsigned omega = 0): vertices_(), index_(i), size_(s), weight_(omega){}
		Clique(std::vector<Vertex*> v);

		//Getters
		const std::vector<Vertex*> &					getVertices() const {return vertices_;}
		const unsigned &								getIndex() const {return index_;}
		const unsigned &								getSize() const {return size_;}
		const unsigned &								getOmega() const {return weight_;}
		const std::vector<forbiddenEdgesFromCliques> &  getForbiddenEdgesFromCliques ()const { return forbiddenEdgesFromCliques_;}
		const std::vector<Path*> & 						getPaths ()const { return paths_;}
		friend ostream &operator<<(std::ostream &flux, const Clique &c);

		//Non constant getters
		std::vector<forbiddenEdgesFromCliques> &  		getForbiddenEdgesFromCliques () { return forbiddenEdgesFromCliques_;}

		//Functions
		void                            addVertex(Vertex* & v);												//adds a vertex to the clique
		void                            removeTheLastVertex(); 												//remove the last vertex of the clique for backtracking
		bool                            tryAddVertex(Vertex* & v);											//try to add a vertex in the clique if possible
		void                            addForbiddenEdgesFromCliques(Edge* & e, Demand* d1, Demand* d2);	//based in two demands, verify if and which edge they share and saves the information
		void							addPath(string label, Path* & p);													//add path that originated the clique

		//Destructors
		~Clique(){}
};

// Overloading flux operator to print object in screen
std::ostream &operator<<(std::ostream &flux, const Clique &c);

#endif // CLIQUE_H