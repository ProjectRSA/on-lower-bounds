#ifndef MCMCF_OUTPUT_HPP
#define MCMCF_OUTPUT_HPP
#include <vector>
#include <iostream> 
#include <fstream>
#include <string>
#include <cstdlib>

class Path;

class MCMCF_Output
{
	private:
		std::vector<Path*> 				routing_;
		unsigned 						capacity_;

	public:
		//Constructors
		MCMCF_Output(const unsigned & cap = 0): routing_(), capacity_(cap) {}
		MCMCF_Output(std::vector<Path*> & rout, const unsigned & cap = 0) : routing_(rout), capacity_(cap) {}

		//Getters
		const std::vector<Path*> & 		getRouting() const {return routing_;}
		const unsigned & 				getCapacity() const {return capacity_;}

		//Destructors
		~MCMCF_Output(){}

};

std::ostream & operator << (std::ostream & flux, const MCMCF_Output & output);

#endif // MCMCF_OUTPUT_HPP