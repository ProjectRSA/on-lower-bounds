#ifndef VERTEX_HPP
#define VERTEX_HPP
#include <stdio.h>
#include <vector>
#include <iostream>

//Forward Declaration
class Arc;
class Edge;

class Vertex {

	private:

		unsigned index_;
		unsigned 				weight_;
		std::vector<Vertex*> 	neighborhood_; 			//Vertices that share and edge
		std::vector<Edge*> 		vertexEdges_;			//Edges containing the vertex
		std::vector<Arc*> 		vertexInArcs_;			//Arcs containing the vertex as destination
		std::vector<Arc*> 		vertexOutArcs_;			//Arcs containing the vertex as origin
		unsigned                weightNeighborhood_;    //Total weight of neighboors

	public:
		//Constructors
		Vertex(){}
		Vertex(const Vertex & v) : index_(v.index_), weight_(v.weight_), neighborhood_(v.neighborhood_), vertexEdges_(v.vertexEdges_), vertexInArcs_(v.vertexInArcs_), vertexOutArcs_(v.vertexOutArcs_), weightNeighborhood_(v.weight_) {}
		Vertex(unsigned i) : index_(i), weight_(), neighborhood_(), vertexEdges_(), vertexInArcs_(), vertexOutArcs_(){}
		Vertex(unsigned i, unsigned weight) : index_(i), weight_(weight), neighborhood_(), vertexEdges_(), vertexInArcs_(), vertexOutArcs_(){}

		//Getters
		const unsigned & 		getIndex() const {return index_;}
		const unsigned & 		getWeight() const {return weight_;}

		//Non constant getters
		unsigned 				getWeight(){return weight_;}
        unsigned 				getWeightNeighborhood(){return weightNeighborhood_;}
		std::vector<Vertex*> 	getNeighborhood(){return neighborhood_;}
		std::vector<Edge*>		getVertexEdges(){return vertexEdges_;}
		std::vector<Arc*> 		getVertexInArcs(){return vertexInArcs_;}
		std::vector<Arc*> 		getVertexOutArcs(){return vertexOutArcs_;}

		//Setters
		void 					setWeight(unsigned w){weight_ = w;}

		//Shows
		void 					showNeighborhood() const;
		void 					showVertexEdges() const;
		void 					showVertexInArcs() const;
		void 					showVertexOutArcs() const;

		//Functions
		void 					addNeighborhood(Vertex & n){neighborhood_.push_back(&n); weightNeighborhood_ += n.getWeight();}
		void 					addVertexEdges(Edge & e){vertexEdges_.push_back(&e);}
		void 					addVertexInArcs(Arc & i){vertexInArcs_.push_back(&i);}
		void 					addVertexOutArcs(Arc & o){vertexOutArcs_.push_back(&o);}

		//Destructors
		~Vertex(){}
};

// Overloading flux operator to print object in screen
std::ostream & operator << (std::ostream & flux, const Vertex & v) ;

#endif // VERTEX_HPP