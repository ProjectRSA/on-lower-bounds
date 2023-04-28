#include "RSA_Output.hpp"
#include "Vertex.hpp"
#include "Edge.hpp"
#include "Arc.hpp"
#include "Path.hpp"
#include "Demand.hpp"

using namespace std;

ostream & operator << (ostream & flux, const RSA_Output & output)
{
    flux << "Output - RSA Problem - " << endl;
    flux << "Interval chromatic number = " << output.getSlots() << endl;
    flux << "Routing : " << endl;
    for (unsigned i = 0; i < output.getRouting().size(); ++i)
    {
    	output.getRouting()[i]->viewInVertices(flux);
        flux << " Slots : " << ((output.getSpectrumAssignment()[i].spectrum) - output.getRouting()[i]->getDemand()->getSlots());
        flux << " - " << output.getSpectrumAssignment()[i].spectrum << endl;
    }
    return flux;
}