#include "Vertex.hpp"
#include "Edge.hpp"
#include "Arc.hpp"
#include "Path.hpp"
#include "Demand.hpp"
#include <algorithm>

using namespace std;

Path::Path(vector<Edge*> & e) : edges_(e)
{
	lengthPath_ = 0.0;
	for (vector<Edge*>::iterator it = edges_.begin(); it != edges_.end(); it++){
		lengthPath_ += (*it)->getLength() ;
	}
}

ostream & operator << (ostream & flux, const Path & p)
{
	flux << "Edges: ";
	for (vector<Edge*>::const_iterator it = p.getEdges().begin(); it != p.getEdges().end(); it++)
		flux << "(" << (*it)->getV1().getIndex() << "," << (*it)->getV2().getIndex() << ") ";

	flux << "Path Lenght: " << p.getLengthPath()<< std::endl ;
	return flux;
}

bool operator==(const Path &lhs, const Path &rhs)
{
    if (lhs.getLengthPath() != rhs.getLengthPath())
        return false;
    if (lhs.getEdges().size() != rhs.getEdges().size())
        return false;
    else
    {
        for (unsigned i = 0; i < lhs.getEdges().size(); ++i)
        {
            int j = 0;
            while (j < rhs.getEdges().size() && lhs.getEdges()[i]->getIndex() != rhs.getEdges()[j]->getIndex())
            {
                j++;
            }
            if (j == rhs.getEdges().size())
            {
                return false;
            }
        }
    }
    return true;
}

ostream & Path::viewOriented(ostream & flux, int index_vertex)
{
    flux << " :: " << index_vertex;
    if(index_vertex == edges_.front()->getV1().getIndex() || index_vertex == edges_.front()->getV2().getIndex())
    {
        for (vector<Edge*>::const_iterator it = edges_.begin(); it != edges_.end(); it++)
        {
            if((*it)->getV1().getIndex() == index_vertex)
            {
                flux << " - " << (*it)->getV2().getIndex();
                index_vertex =  (*it)->getV2().getIndex();
            }
            else
            {
                flux << " - " << (*it)->getV1().getIndex();
                index_vertex =  (*it)->getV1().getIndex();
            }
        }
    }
    else
    {
        for (vector<Edge*>::reverse_iterator it = edges_.rbegin(); it != edges_.rend(); it++)
        {
            if((*it)->getV1().getIndex() == index_vertex)
            {
                flux << " - " << (*it)->getV2().getIndex();
                index_vertex =  (*it)->getV2().getIndex();
            }
            else
            {
                flux << " - " << (*it)->getV1().getIndex();
                index_vertex =  (*it)->getV1().getIndex();
            }
        }
    }
    

    flux << " :: with length " << lengthPath_ << endl;
    return flux;
}

void Path::reorient(int origin_index)
{
    vector<Edge> reoriented(0);
    if(edges_.back()->getV1().getIndex() == origin_index || edges_.back()->getV2().getIndex() == origin_index)
    {
        reverse(edges_.begin(), edges_.end());
    }
}