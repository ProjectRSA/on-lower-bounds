#ifndef GRAPH_HPP
#define GRAPH_HPP
#include <stdio.h> 
#include <vector>  
#include <iostream>

//Forward declaration
class Vertex;
class Edge;
class Arc;

class Graph 
{
	private:
		unsigned 				index_;
		unsigned 				numberNodes_;
		unsigned 				numberLinks_;
		std::vector<Vertex*> 	vertices_;
		std::vector<Edge*> 		edges_;
		std::vector<Arc*> 		arcs_;
		bool 					oriented_;
		bool 					notOriented_;

	public:
		//Constructors
		Graph(unsigned i = 0) : index_(i){}
		Graph(Graph & g) : vertices_(g.vertices_), edges_(g.edges_), arcs_(g.arcs_), oriented_(g.oriented_), notOriented_(g.notOriented_){}
		Graph(std::vector<Vertex*>& v, std::vector<Edge*>& e);
		Graph(std::vector<Vertex*>& v, std::vector<Arc*> & a);

		//Getters
		const unsigned &				getIndex() const {return index_;}	
		const unsigned &				getnumberNodes() const {return numberNodes_;}
		const unsigned &				getnumberLinks() const {return numberLinks_;}
		const std::vector<Vertex*> & 	getVertices() const {return vertices_;}
		const std::vector<Edge*> &		getEdges() const {return edges_;}
		const std::vector<Arc*> &		getArcs() const {return arcs_;}
		const bool &					isOriented() const {return oriented_;}
		const bool &					isNotOriented() const {return notOriented_;}
		
		//Non constant getters
		std::vector<Vertex*> & 			getVertices() {return vertices_;}
		std::vector<Edge*> &			getEdges() {return edges_;}
		std::vector<Arc*> &				getArcs() {return arcs_;}

		//Destructors
		~Graph(){}

};

// Overloading flux operator to print object in screen
std::ostream & operator << (std::ostream & flux, const Graph & g) ;

#endif // GRAPH_HPP