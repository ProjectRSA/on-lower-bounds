#include "Vertex.hpp"
#include "Edge.hpp"
#include "Arc.hpp"

Arc::Arc(Edge & e)
{
	index_ = e.getIndex();
	length_ = e.getLength();
 	cost_= e.getCost();
	numberSlices_ = e.getNumberSlices();
	origin_ = &e.getV1();
	destination_= &e.getV2();
	edgeRsa_ = &e; 
}

void Arc::InvertOrientation()
{
	Vertex* aux = origin_;
	origin_= destination_;
	destination_ = aux;
}

std::ostream & operator << (std::ostream & flux, const Arc & a) 
{
	flux << "--------" << "Index: " << a.getIndex() << std::endl ;
	flux << "Origin: " << a.getOrigin().getIndex() << "| Destination: " << a.getDestination().getIndex() << std::endl ; 
	flux << "Lenght: " << a.getLength() << "| Cost: " << a.getCost() << "| Slices: " << a.getNumberSlices() << std::endl ; 
	flux << "-------- " << std::endl ;
	return flux;
}