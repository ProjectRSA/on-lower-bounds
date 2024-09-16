#include "Vertex.hpp"
#include "Arc.hpp"
#include "Edge.hpp"

std::ostream & operator << (std::ostream & flux, const Edge & e) 
{
	flux << "--------" << "Index: " << e.getIndex() << std::endl ;
	flux << "v1: " << e.getV1().getIndex() << "| v2: " << e.getV2().getIndex() << std::endl ; 
	flux << "Lenght: " << e.getLength()<< "| Noise: " << e.getNoise() << "| Cost: " << e.getCost() << "| Slices: " << e.getNumberSlices() << std::endl ; 
	flux << "-------- " << std::endl ;
	return flux;
}
/*
Edge & Edge::operator = (const Edge & e) 
{
	v1_ = e.v1_;
	v2_ = e.v2_;
	index_= e.index_;
	length_= e.length_;
	cost_= e.cost_;
	numberSlices_= e.numberSlices_;
	return *this;
}*/