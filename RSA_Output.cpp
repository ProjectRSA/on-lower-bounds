#include "RSA_Output.hpp"
#include "Vertex.hpp"
#include "Edge.hpp"
#include "Arc.hpp"
#include "Path.hpp"
#include "Demand.hpp"

using namespace std;

ostream & operator << (ostream & flux, const RSA_Output & output)
{
    flux << "Output - RSA Problem" << endl;
    flux << "Objective Value Z* = " << output.getSlots() << endl;
    flux << "Routing : " << endl;
    for (unsigned i = 0; i < output.getRouting().size(); ++i)
    {
		Demand* dem = output.demands_[i];
        flux << "From demand: " << dem->getIndex();
        flux << " Between nodes: " << dem->getOrigin().getIndex() << " to " << dem->getDestination().getIndex() ;
    	output.getRouting()[i]->viewOriented(flux, dem->getOrigin().getIndex());
		flux << "Slots : " << output.getSpectrumAssignment()[i].spectrumInitial << " - " << output.getSpectrumAssignment()[i].spectrum << endl;
	}
    return flux;
}

unsigned RSA_Output::getSpan()
{
	unsigned maxSpectrumPosition = 0;
	for (int i = 0; i < spectrumAssignment_.size(); ++i)
	{
		if (spectrumAssignment_[i].spectrum > maxSpectrumPosition)
		{
			maxSpectrumPosition = spectrumAssignment_[i].spectrum;
		}
	}
	return maxSpectrumPosition;
}
