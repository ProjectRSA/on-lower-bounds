#include "Vertex.hpp"
#include "Edge.hpp"
#include "Arc.hpp"
#include "Graph.hpp"

Graph::Graph(std::vector<Vertex*>& v, std::vector<Edge*>& e) : vertices_(v), edges_(e), oriented_(false), notOriented_(true)
{
	index_ = 0;
	numberNodes_ = v.size(); 
	numberLinks_ = e.size();
}

Graph::Graph(std::vector<Vertex*>& v, std::vector<Arc*> & a) : vertices_(v), arcs_(a), oriented_(true), notOriented_(false)
{
	index_ = 0;
	numberNodes_ = v.size();	
	numberLinks_ = a.size();
}

std::ostream & operator << (std::ostream & flux, const Graph & g) 
{
	//flux << "--------" << "Index: " << g.getIndex() << std::endl ;
	flux << "Number of nodes: " << g.getnumberNodes() << "| Number of links: : " << g.getnumberLinks() << std::endl ;

	flux << "Nodes: ( ";
	for (std::vector<Vertex*>::const_iterator it = g.getVertices().begin(); it != g.getVertices().end(); it++)
		flux << (*it)->getIndex() << " ";
	flux << ")" << std::endl ;

	if (g.isOriented()){
		flux << "Oriented" << std::endl;
		flux << "Arcs: ";
		for (std::vector<Arc*>::const_iterator it = g.getArcs().begin(); it != g.getArcs().end(); it++)
		{
			flux << " (" << (*it)->getOrigin().getIndex() << "," << (*it)->getDestination().getIndex() << ") " << "Length " << (*it)->getLength() << std::endl;
		}
	}
	if (g.isNotOriented()){
		flux << "Not oriented" << std::endl;
		flux << "Edges: ";
		for (std::vector<Edge*>::const_iterator it = g.getEdges().begin(); it != g.getEdges().end(); it++)
		{
			flux << "(" << (*it)->getV1().getIndex() << "," << (*it)->getV2().getIndex() << ") " << "Length " << (*it)->getLength() << " Noise " << (*it)->getNoise()<< std::endl;
		}
	}
	flux << "-------- " << std::endl ;
	return flux;
}