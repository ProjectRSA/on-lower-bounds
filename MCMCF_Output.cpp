#include "MCMCF_Output.hpp"
#include "Vertex.hpp"
#include "Arc.hpp"
#include "Edge.hpp"
#include "Path.hpp"
#include "Demand.hpp"

using namespace std;

ostream & operator << (ostream & flux, const MCMCF_Output & output)
{
    flux << "Output - MCMCF Problem - " << endl;
    flux << "Capacity = " << output.getCapacity() << endl;
    flux << "Routing : " << endl;
    int idx = 0;
    for (vector<Path*>::const_iterator it = output.getRouting().begin();  it != output.getRouting().end(); ++it )
    {
        Demand* dem = output.demands_[idx];
        flux << "From demand: " << dem->getIndex();
        flux << " Between nodes: " << dem->getOrigin().getIndex() << " to " << dem->getDestination().getIndex() ;
        (*it)->viewOriented(flux, dem->getOrigin().getIndex());
        idx++;
    }
    return flux;
}