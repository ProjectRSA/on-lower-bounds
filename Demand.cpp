#include "Vertex.hpp"
#include "Path.hpp"
#include "Edge.hpp"
#include "Arc.hpp"
#include "Demand.hpp"

std::ostream& operator << (std::ostream& flux, const Demand& d)
{
	flux << " " << d.getIndex() << "\t| " << d.getOrigin().getIndex() << "\t| "
		<< d.getDestination().getIndex() << "\t| " << d.getMaxLength() << "\t| "
		<< d.getMinOsnr() << "\t| " << d.getSlots() << "\t| ";
	return flux;
}

void Demand::showRoutings() const 
{
	std::cout << "Routings for demand: " << index_ << std::endl;
	int idx = 1;
	for (std::vector<Path*>::const_iterator it = routings_.begin(); it != routings_.end(); it++){
		std::cout << "Routing: " << idx << std::endl;
		std::cout << "Edges: ";
		for (std::vector<Edge*>::const_iterator it2 = (*it)->getEdges().begin(); it2 != (*it)->getEdges().end(); it++){
			std::cout << "(" << (*it2)->getV1() << "," << (*it2)->getV2() << ") ";
		}
		std::cout << "Routing: " << *it << std::endl;
		std::cout << "-------- " << std::endl ;
		idx++;
	}
	std::cout << "-------- " << std::endl ;
}