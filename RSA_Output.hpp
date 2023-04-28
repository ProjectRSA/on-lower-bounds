#ifndef RSA_OUTPUT_HPP
#define RSA_OUTPUT_HPP
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

struct SpectrumAssignmentPerDemand
{
	unsigned indexDemand;
	unsigned spectrum;  			// The last slot ocupped per demand
};

class Path;

class RSA_Output
{
	private:
		std::vector<Path*> 									routing_;
		std::vector<SpectrumAssignmentPerDemand> 			spectrumAssignment_;
		unsigned 											slots_;

	public:
		//Constructors
		RSA_Output (const unsigned & slots = 0) : routing_(), spectrumAssignment_(), slots_(slots) {}
        RSA_Output (std::vector<Path*> & routing, std::vector<SpectrumAssignmentPerDemand> & spectrumAssignment, const unsigned & slots = 0) : routing_(routing), spectrumAssignment_(spectrumAssignment), slots_(slots) {}

		//Getters
		const std::vector<Path*> & 							getRouting() const {return routing_;}
		const unsigned & 									getSlots() const {return slots_;}
		const std::vector<SpectrumAssignmentPerDemand> & 	getSpectrumAssignment() const {return spectrumAssignment_;}

		//Non constant getters
		std::vector<Path*> &								getRouting() {return routing_;}

		//Destructors
		~RSA_Output(){}
};

std::ostream & operator << (std::ostream & flux, const RSA_Output & output);

#endif // RSA_OUTPUT_HPP