#include "MCMCF_Output.hpp"
#include "Vertex.hpp"
#include "Arc.hpp"
#include "Edge.hpp"
#include "Path.hpp"

using namespace std;

ostream & operator << (ostream & flux, const MCMCF_Output & output)
{
    flux << "Output - MCMCF Problem - " << endl;
    flux << "Capacity = " << output.getCapacity() << endl;
    flux << "Routing : " << endl;
    for (vector<Path*>::const_iterator it = output.getRouting().begin();  it != output.getRouting().end(); ++it )
    {
        (*it)->viewInVertices(flux);
    }
    return flux;
}