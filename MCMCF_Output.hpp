#ifndef MCMCF_OUTPUT_HPP
#define MCMCF_OUTPUT_HPP
#include <vector>
#include <iostream> 
#include <fstream>
#include <string>
#include <cstdlib>

using namespace std;
class Path;
class Demand;

class MCMCF_Output
{
	private:
		vector<Demand *> demands_;
		std::vector<Path*> 				routing_;
		unsigned 						capacity_;

	public:
		//Constructors
		MCMCF_Output(const unsigned & cap = 0): routing_(), capacity_(cap) {}
		MCMCF_Output(std::vector<Path*> & rout, const unsigned & cap = 0) : routing_(rout), capacity_(cap) {}
		MCMCF_Output(vector<Demand*> & dems, std::vector<Path*> & rout, const unsigned & cap = 0) : demands_(dems), routing_(rout), capacity_(cap) {}

		//Getters
		const std::vector<Path*> & 		getRouting() const {return routing_;}
		const unsigned & 				getCapacity() const {return capacity_;}

		//Non constant getters
		std::vector<Path*>&             getRouting() { return routing_; }

		//Destructors
		~MCMCF_Output(){}

		//Show
		friend ostream &operator<<(ostream &flux, const MCMCF_Output &output);
};

std::ostream &operator<<(std::ostream &flux, const MCMCF_Output &output);

#endif // MCMCF_OUTPUT_HPP