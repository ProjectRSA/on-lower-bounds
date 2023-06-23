#ifndef RSA_OUTPUT_HPP
#define RSA_OUTPUT_HPP
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

using namespace std;
class Demand;

struct SpectrumAssignmentPerDemand
{
	unsigned indexDemand;
	unsigned spectrumInitial;
	unsigned spectrum;  			// The last slot ocupped per demand
};

class Path;

class RSA_Output
{
	private:
		vector<Demand*> demands_;
		std::vector<Path*> 									routing_;
		std::vector<SpectrumAssignmentPerDemand> 			spectrumAssignment_;
		unsigned 											slots_;

	public:
		//Constructors
		RSA_Output (const unsigned & slots = 0) : routing_(), spectrumAssignment_(), slots_(slots) {}
        RSA_Output (std::vector<Path*> & routing, std::vector<SpectrumAssignmentPerDemand> & spectrumAssignment, const unsigned & slots = 0) : routing_(routing), spectrumAssignment_(spectrumAssignment), slots_(slots) {}
        RSA_Output (vector<Demand*> & dems, vector<Path*> & routing, std::vector<SpectrumAssignmentPerDemand> & spectrumAssignment, const unsigned & slots = 0) : demands_(dems), routing_(routing), spectrumAssignment_(spectrumAssignment), slots_(slots) {}

		//Getters
		const std::vector<Path*> & 							getRouting() const {return routing_;}
		const unsigned & 									getSlots() const {return slots_;}
		const std::vector<SpectrumAssignmentPerDemand> & 	getSpectrumAssignment() const {return spectrumAssignment_;}

		//Non constant getters
		std::vector<Path *> & getRouting() { return routing_; }
		
		//Functions
		unsigned getSpan();

		//Show
		friend ostream &operator<<(ostream &flux, const RSA_Output &output);

		//Destructors
		~RSA_Output(){}
};

std::ostream &operator<<(std::ostream &flux, const RSA_Output &output);

#endif // RSA_OUTPUT_HPP