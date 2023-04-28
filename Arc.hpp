#ifndef ARC_HPP
#define ARC_HPP
#include <stdio.h> 
#include <vector> 
#include <ostream> 

//Forward declaration
class Vertex;
class Edge;

class Arc {

	private:
		Vertex*		origin_;
		Vertex*	 	destination_; 
		unsigned 	index_; 
		double	 	length_;
		double	 	cost_;
		unsigned 	numberSlices_;
		Edge*		edgeRsa_;

	public:
		//Constructors
		Arc(){}
		Arc(const Arc & a): origin_(a.origin_), destination_(a.destination_), index_(a.index_), length_(a.length_), cost_(a.cost_), numberSlices_(a.numberSlices_),edgeRsa_(a.edgeRsa_){};
		Arc(Arc & a): origin_(a.origin_), destination_(a.destination_), index_(a.index_), length_(a.length_), cost_(a.cost_), numberSlices_(a.numberSlices_),edgeRsa_(a.edgeRsa_){};
		Arc(Vertex o, Vertex d, unsigned i, double l, double c, unsigned n) : origin_(&o), destination_(&d), index_(i), length_(l), cost_(c), numberSlices_(n){}
		Arc(Edge & e); 

		//Getters
		const Vertex &		getOrigin() const {return *origin_;}
		const Vertex &		getDestination() const {return *destination_;}
		const unsigned &	getIndex() const {return index_;}
		const double &		getLength() const {return length_;}
		const double & 		getCost() const {return cost_;}
		const unsigned &	getNumberSlices() const {return numberSlices_;}

		//Non constant getters
		Vertex &			getOrigin() {return *origin_;}
		Vertex &			getDestination() {return *destination_;}
		Edge* & 			getEdge() {return edgeRsa_;}
		
		//Setters
		void				setIndex(unsigned index) {index_ = index;}

		//Functions
		void 				InvertOrientation();	//invert orientation of the arc

		//Destructors
		~Arc(){}
	
};

// Overloading flux operator to print object in screen
std::ostream & operator << (std::ostream & flux, const Arc & a) ;

#endif // ARC_HPP