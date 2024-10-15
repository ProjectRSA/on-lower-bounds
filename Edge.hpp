#ifndef EDGE_HPP
#define EDGE_HPP
#include <stdio.h>
#include <vector>

//Forward declaration
class Vertex;

class Edge
{
	private:
		Vertex* 	v1_;
		Vertex* 	v2_;
		unsigned 	index_;
		double 		length_;
		double 		noise_;
		double 		cost_;
		unsigned	numberSlices_;

	public:
		//Constructors
		Edge(){}
		Edge(const Edge & e) : v1_(e.v1_), v2_(e.v2_), index_(e.index_), length_(e.length_), noise_(e.noise_), cost_(e.cost_), numberSlices_(e.numberSlices_){}
		Edge(Vertex v1, Vertex v2, unsigned i, double l, double no, double c, unsigned n) : v1_(&v1), v2_(&v2), index_(i), length_(l), noise_(no), cost_(c), numberSlices_(n){}
		Edge(Vertex* v1, Vertex* v2, unsigned i) : v1_(v1), v2_(v2), index_(i), length_(), noise_(), cost_(), numberSlices_(){}
		Edge(unsigned i, double l, double no, double c, unsigned n) : index_(i), length_(l), noise_(no), cost_(c), numberSlices_(n){}


		//Getters
		const Vertex & 		getV1() const {return *v1_;}
		const Vertex &		getV2() const {return *v2_;}
		const unsigned &	getIndex() const {return index_;}
		const double & 		getLength() const {return length_;}
		const double & 		getNoise() const {return noise_;}
		const double &		getCost() const {return cost_;}
		const unsigned & 	getNumberSlices() const {return numberSlices_;}
		
		//Non constant getters
		Vertex &  			getV1() {return *v1_;}
		Vertex &			getV2() {return *v2_;}

		//Setters
		void 				setV1(Vertex & v) {v1_ = &v;}
		void 				setV2(Vertex & v) {v2_ = &v;}

		//Edge & operator = (const Edge & e) ;

		//Destructors
		~Edge(){}

};

// Overloading flux operator to print object in screen
std::ostream & operator << (std::ostream & flux, const Edge & e) ;

#endif // EDGE_H