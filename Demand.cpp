#include "Vertex.hpp"
#include "Path.hpp"
#include "Edge.hpp"
#include "Arc.hpp"
#include "Demand.hpp"

std::ostream & operator << (std::ostream & flux, const Demand & d) 
{
	flux << "--------" << "Index: " << d.getIndex() << std::endl ;
	flux << "Origin: " << d.getOrigin().getIndex() << "| Destination: " << d.getDestination().getIndex() << std::endl ;
	flux << "Slots: " << d.getSlots() << "| Max Lenght: " << d.getMaxLength() << "| Min OSNR: " << d.getMinOsnr()<< std::endl ;
	return flux;
}

void Demand::showRoutings() const 
{
	std::cout << "Routings for demand: " << index_ << std::endl;
	for (std::vector<Path*>::const_iterator it = routings_.begin(); it != routings_.end(); it++){
		std::cout << "Routing: " << (*it)->getIndex() << std::endl;
		std::cout << "Edges: ";
		for (std::vector<Edge*>::const_iterator it2 = (*it)->getEdges().begin(); it2 != (*it)->getEdges().end(); it++){
			std::cout << "(" << (*it2)->getV1() << "," << (*it2)->getV2() << ") ";
		}
		std::cout << "Routing: " << *it << std::endl;
		std::cout << "-------- " << std::endl ;
	}
	std::cout << "-------- " << std::endl ;
}