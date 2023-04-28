#include "Vertex.hpp"
#include "Arc.hpp"
#include "Edge.hpp"
#include "Path.hpp"
#include "Clique.hpp"
#include "Demand.hpp"

using namespace std;

Clique::Clique(std::vector<Vertex*> v) : vertices_(v)
{
	size_ = v.size();
	omega_ = 0;
    index_ = 0;

	for (std::vector<Vertex*>::iterator it = v.begin(); it!= v.end(); it++){
		omega_ += (*it)->getWeight() ;
	}
}

void Clique::addVertex(Vertex* & v)
{
    vertices_.push_back(v);
    omega_ += v->getWeight();
    size_ ++;
}

void Clique::removeTheLastVertex()
{
    omega_ -= vertices_[vertices_.size()-1]->getWeight();
    vertices_.pop_back();
    size_ --;
}

void Clique::addPath(Path* & p)
{
    paths_.push_back(p);
}

bool Clique::tryAddVertex(Vertex* & v)
{
    if (vertices_.size() == 0)
    {
        addVertex(v);
        return true;
    }
    bool isPossible;
    for (unsigned i = 0; i < vertices_.size(); ++i)
    {
        isPossible = false;
        for (unsigned j = 0; j < vertices_[i]->getNeighborhood().size(); ++j)
        {
            if (vertices_[i]->getNeighborhood()[j]->getIndex() == v->getIndex() )
            {
                isPossible = true;
                break;
            }
        }
        if(isPossible == false)
            return false;
    }
    addVertex(v);
    return true;
}

void Clique::addForbiddenEdgesFromCliques(Edge* & e, Demand* d1, Demand* d2)
{
    for (vector<forbiddenEdgesFromCliques>::iterator it = forbiddenEdgesFromCliques_.begin(); it != forbiddenEdgesFromCliques_.end(); ++it)
    {
        if((*it).edges->getIndex() == e->getIndex())
        {
            bool v1 = true;
            bool v2 = true;
            for (vector<Demand*>::const_iterator it2 = (*it).indexDemand.begin(); it2 != (*it).indexDemand.end(); ++it2)
            {
                if ((*it2)->getIndex() == d1->getIndex())
                    v1 = false;
                if ((*it2)->getIndex() == d2->getIndex())
                    v2 = false;
            }
            if (v1 == true)
                (*it).indexDemand.push_back(d1);
            if (v2 == true)
                (*it).indexDemand.push_back(d2);

            return ;
        }
    }
    forbiddenEdgesFromCliques fefc;
    fefc.edges = e;
    fefc.indexDemand.push_back(d1);
    fefc.indexDemand.push_back(d2);
    forbiddenEdgesFromCliques_.push_back(fefc);
    return ;
}

std::ostream & operator << (std::ostream & flux, const Clique & c)
{
	//flux << "--------" << "Index: " << c.getIndex() << std::endl ;
	flux << "Size: " << c.getSize() << "| Omega: " << c.getOmega() << std::endl ;
	flux << "Clique: ( " ;
	for (std::vector<Vertex*>::const_iterator it = c.getVertices().begin(); it != c.getVertices().end(); it++)
		flux << (*it)->getIndex() << " ";
	flux << ")" << std::endl ;
    flux << "Paths of this clique:" << endl;
    for (unsigned i = 0; i < c.getPaths().size(); ++i)
    {
        flux << (*(c.getPaths()[i]));
    }
	flux << "-------- " << std::endl ;
	return flux;
}