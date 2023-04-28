#include "Vertex.hpp"
#include "Edge.hpp"
#include "Arc.hpp"
#include <stdio.h> 
#include <vector>

void Vertex::showNeighborhood() const
{
	std::cout << "Neighborhood of vertex: " << index_ << std::endl;
	std::cout << "Vertices: ";
	for (std::vector<Vertex*>::const_iterator it = neighborhood_.begin(); it != neighborhood_.end(); it++)
		std::cout << (*it)->getIndex() << " ";
	std::cout << ")" << std::endl ;

	std::cout << "-------- " << std::endl ;
}

void Vertex::showVertexEdges() const
{
	std::cout << "Edges of vertex: " << index_ << std::endl;
	std::cout << "Edges: ";
	for (std::vector<Edge*>::const_iterator it = vertexEdges_.begin(); it != vertexEdges_.end(); it++)
		std::cout << "(" << (*it)->getV1() << "," << (*it)->getV2() << ") ";
	std::cout << "-------- " << std::endl ;
}

void Vertex::showVertexInArcs() const
{
	std::cout << "In arcs of vertex: " << index_ << std::endl;
	std::cout << "Arcs: ";
	for (std::vector<Arc*>::const_iterator it = vertexInArcs_.begin(); it != vertexInArcs_.end(); it++)
		std::cout << "(" << (*it)->getOrigin().getIndex() << "," << (*it)->getDestination().getIndex() << ") ";
	std::cout << "-------- " << std::endl ;
}

void Vertex::showVertexOutArcs() const
{
	std::cout << "Out arcs of vertex: " << index_ << std::endl;
	std::cout << "Arcs: ";
	for (std::vector<Arc*>::const_iterator it = vertexOutArcs_.begin(); it != vertexOutArcs_.end(); it++)
		std::cout << "(" << (*it)->getOrigin().getIndex() << "," << (*it)->getDestination().getIndex() << ") ";
	std::cout << "-------- " << std::endl ;
}

std::ostream & operator << (std::ostream & flux, const Vertex & v) {
	flux << "--------" << "Index: " << v.getIndex() << std::endl ;
	flux << "--------" << "Weight: " << v.getWeight() << std::endl ;
	flux << "-------- " << std::endl ;
	return flux;
}