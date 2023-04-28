#ifndef DEMAND_HPP
#define DEMAND_HPP
#include <stdio.h>    
#include <vector>   
#include <iostream>

//Forward declaration
class Vertex;
class Path;

class Demand 
{
	private:
		Vertex*					origin_;
		Vertex*					destination_;
		unsigned 				index_;
		unsigned				slots_;
		double 					maxLength_;  	//max length allowed for this demand
		std::vector<Path*> 		routings_;		//possible routings for this demand

	public:
		//Constructors
		Demand(){}
		Demand(Vertex o, Vertex d, unsigned i, unsigned s, double m) : origin_(&o), destination_(&d), index_(i), slots_(s), maxLength_(m), routings_(){}
		Demand(unsigned i, unsigned s, double m) : index_(i), slots_(s), maxLength_(m), routings_(){}
	
		//Getters
		const Vertex & 			getOrigin() const {return *origin_;}
		const Vertex &			getDestination() const {return *destination_;}
		const unsigned &		getIndex() const {return index_;}
		const unsigned &		getSlots() const {return slots_;}
		const double &			getMaxLength() const {return maxLength_;}

		//Non constant getters
		Vertex & 				getOrigin() {return *origin_;}
		Vertex &				getDestination() {return *destination_;}
		std::vector<Path*> &	getRoutings(){return routings_;}

		//Setters
		void 					setRoutings(std::vector<Path*> p){routings_ = p;}
		void 					setOrigin(Vertex & v) {origin_ = &v;}
		void 					setDestination(Vertex & v) {destination_ = &v;}

		//Shows
		void showRoutings() const;
		
		
		//Destructors
		~Demand(){}
};

// Overloading flux operator to print object in screen
std::ostream & operator << (std::ostream & flux, const Demand & d);

#endif // DEMAND_HPP