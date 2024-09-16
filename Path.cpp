#include "Vertex.hpp"
#include "Edge.hpp"
#include "Arc.hpp"
#include "Path.hpp"
#include "Demand.hpp"
#include <math.h> 
using namespace std;

Path::Path(vector<Edge*> & e) : edges_(e)
{
	lengthPath_ = 0.0;
    index_ = 0;
    noisePath_ = 0.0;

	for (vector<Edge*>::iterator it = edges_.begin(); it != edges_.end(); it++){
		lengthPath_ += (*it)->getLength() ;
	}

    for (vector<Edge*>::iterator it = edges_.begin(); it != edges_.end(); it++){
		noisePath_ += (*it)->getNoise() ;
	}
}

Path::Path(vector<Edge*> & e, Demand* d) : edges_(e) , originalDemand_(d)
{
	lengthPath_ = 0.0;
    index_ = 0;
    noisePath_ = 0.0;

	for (vector<Edge*>::iterator it = edges_.begin(); it != edges_.end(); it++){
		lengthPath_ += (*it)->getLength() ;
	}

    for (vector<Edge*>::iterator it = edges_.begin(); it != edges_.end(); it++){
		noisePath_ += (*it)->getNoise() ;
	}
}

const unsigned & Path::getIndexDemand() const {return originalDemand_->getIndex();}

ostream & operator << (ostream & flux, const Path & p)
{
	//flux << "--------" << "Index: " << p.getIndex() << std::endl ;
	//flux << "--------" << "From demand: " << p.getIndexDemand() << std::endl ;
	flux << "Edges: ";
	for (vector<Edge*>::const_iterator it = p.getEdges().begin(); it != p.getEdges().end(); it++)
		flux << "(" << (*it)->getV1().getIndex() << "," << (*it)->getV2().getIndex() << ") ";

	double pathNoise = p.getNoisePath();
	double pch = p.getDemand()->getPch();
	double osnr;
	double osnrdb;		
	osnr = pch/(pathNoise);
	osnrdb = 10.0 * log10(osnr);
    
    flux << "Path Lenght: " << p.getLengthPath()<< " Path OSNR: " << osnrdb<< std::endl ;
	//flux << "-------- " << std::endl ;
	return flux;
}

bool operator==(const Path& lhs, const Path& rhs)
{
    if (lhs.getDemand()->getIndex() != rhs.getDemand()->getIndex())
        return false;
    if (lhs.getLengthPath() != rhs.getLengthPath())
        return false;
    if (lhs.getEdges().size() != rhs.getEdges().size())
        return false;
    else
    {
        for (unsigned i = 0; i < lhs.getEdges().size(); ++i)
        {
            if ( lhs.getEdges()[i]->getIndex() != rhs.getEdges()[i]->getIndex())
                return false;
        }
    }
    return true;
}

void Path::showInVerticesPath()
{
    cout << "From demand: " << originalDemand_->getIndex();
    cout << " Between nodes: " << originalDemand_->getOrigin().getIndex() << " to " << originalDemand_->getDestination().getIndex() ;
    cout << " :: " << originalDemand_->getOrigin().getIndex();
    unsigned index_vertex = originalDemand_->getOrigin().getIndex();
    for (vector<Edge*>::const_iterator it = edges_.begin(); it != edges_.end(); it++)
    {
        if((*it)->getV1().getIndex() == index_vertex)
        {
            cout << " - " << (*it)->getV2().getIndex();
            index_vertex =  (*it)->getV2().getIndex();
        }
        else
        {
            cout << " - " << (*it)->getV1().getIndex();
            index_vertex =  (*it)->getV1().getIndex();
        }
    }

    cout << " :: with length " << lengthPath_ << endl;
}

ostream & Path::viewInVertices(ostream & flux)
{
    
    flux << "From demand: " << originalDemand_->getIndex();
    flux << " Between nodes: " << originalDemand_->getOrigin().getIndex() << " to " << originalDemand_->getDestination().getIndex() ;
    flux << " :: " << originalDemand_->getOrigin().getIndex();
    unsigned index_vertex = originalDemand_->getOrigin().getIndex();
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

    flux << " :: with length " << lengthPath_ << endl;
    return flux;
}