#include "RSA_Input.hpp"
#include <iostream> 
#include <fstream>
#include <sstream>      
#include <stdio.h>
#include <string>
#include <cstdlib>
#include "Vertex.hpp"
#include "Edge.hpp"
#include "Arc.hpp"
#include "Demand.hpp"
#include "Path.hpp"
#include <math.h>

using namespace std;     

void RSA_Input::showRequests() const {
	
	for (std::vector<Demand*>::const_iterator it = requests_.begin(); it != requests_.end(); it++)
		cout << **it << endl;
}

std::vector<std::string> split(std::string const & line, char delim)
{
	std::vector<std::string> result;
	std::istringstream temp(line);
	std::string current;
	while (std::getline(temp, current, delim)) {
		result.push_back(current);
	}
	return result;
}

vector<vector<string>> readingCsvFile (string fileName)
{
	vector<vector<string>> _data;
	ifstream in(fileName.c_str());
	_data.push_back(vector <string>());
	int i = 0;
	if (in.is_open()) {
		//cout << "------------FILE HAS BEEN OPENED WITH SUCCESS----------" << endl;
		std::string line;
		while (getline(in, line))
		{
			vector<string> subdata;
			vector<string> tempo;
			tempo = split(line, ';');
			int size = tempo.size();
			for (int k = 0; k < size; ++k)
			{
				vector<string> tmp;
				//cout <<tempo[k]<< endl;
				tmp = split(tempo[k], '(');
				if (tmp.size() >= 2)
				{
					//subdata.push_back(tmp[0]);
					//cout <<tmp[0] << " - "<< tmp[1].substr(0,tmp[1].size()-1)<< endl;
					subdata.push_back(tmp[1].substr(0, tmp[1].size() - 1));
				}
				else
				{
					subdata.push_back(tempo[k]);
					//cout <<tempo[k] << " Estoy la  "<< endl;
				}	
			}
			//numberOfColumns = subdata.size();
			_data.push_back(subdata);
			i++;
		}
		//numberOfRows = i;
	}
	else {
		cout << "ERROR NO FILE HAS BEEN OPENED with Name = " <<fileName<< endl;
		exit(EXIT_FAILURE);
	}
	in.close();
	return _data;
}

bool point_found(string value, int i)
{
	bool found=true;
	if(value[i]=='0')
		found= false;
	if(value[i]=='1')
		found= false;
	if(value[i]=='2')
		found= false;
	if(value[i]=='3')
		found= false;
	if(value[i]=='4')
		found= false;
	if(value[i]=='5')
		found= false;
	if(value[i]=='6')
		found= false;
	if(value[i]=='7')
		found= false;
	if(value[i]=='8')
		found= false;
	if(value[i]=='9')
		found= false;
	return found; 
}

float convert_string_to_float(string value)
{
	/*
	bool point = false;
	std::string bef;
	std::string aft;
	for (unsigned int i = 0; i < value.size(); i++)
	{
		if ((point_found(value, i)== true) || (value[i] == ',')|| (value[i] == '.'))
			point = true;
		else
			if (point == true)
			{
				char c = value[i];
				if (aft.size() < 5)
				{
					aft += c;
				}
			}
			else
			{
				char c = value[i];
				bef += c;	
			}
	}
	float val = 0;
	float v = 0;
	if (aft.size() != 0)
	{
		v = float(stoi(aft.c_str()));
		int sz = aft.size();
		v *= float(pow(10, -sz));
	}
	float v2 = 0;
	if (bef.size() != 0)
	{
		v2 = float(stoi(bef.c_str()));
	}
	val = v2 + v;
	*/
	float val = std::stod(value);
	return val;
}

void RSA_Input::data_load_node(const std::string filename)
{
	//cout<<endl<< "============= SET OF NODES ===============" << endl;
	vector<vector<string>> Nod = readingCsvFile(filename);

	for (unsigned k = 2; k < Nod.size(); k++)
	{	
		Vertex * Nd = new Vertex(convert_string_to_float(Nod[k][0]));
		nodes_.push_back(Nd);
	}
}

void RSA_Input::data_load_edge(const std::string  filename)
{
	//cout<<endl<< "============= SET OF EDGES ===============" << endl;	
	edges_.resize(0);	
	vector<vector<string>> Lin = readingCsvFile(filename);

	for (unsigned k = 2; k < Lin.size(); ++k)
	{
		Edge * edge = new Edge(convert_string_to_float(Lin[k][0]),convert_string_to_float(Lin[k][3]),  convert_string_to_float(Lin[k][7]),convert_string_to_float(Lin[k][5]), convert_string_to_float(Lin[k][4]) );

		unsigned v1 = convert_string_to_float(Lin[k][1]);
		unsigned v2 = convert_string_to_float(Lin[k][2]);

		for(vector<Vertex*>::iterator it = nodes_.begin(); it!=nodes_.end(); it++)
		{	
			if ((*it)->getIndex() == v1){
				edge->setV1(**it);
				//edge->getV1().setWeight(11);
			}
			else if ((*it)->getIndex() == v2){
				edge->setV2(**it);
			}	
		}	
		//cout << (*edge) << endl;
		edges_.push_back(edge);
	}
}

void RSA_Input::data_load_demand(const std::string filename)
{
	//cout<<endl<< "============= SET OF DEMANDS ===============" << endl;		
	vector<vector<string>> Dem = readingCsvFile(filename);

	for (unsigned k = 2; k < Dem.size(); ++k)
	{
		//cout << convert_string_to_float(Dem[k][0]) << ";" << convert_string_to_float(Dem[k][1]) << ";" << convert_string_to_float(Dem[k][2])<< ";" << convert_string_to_float(Dem[k][3])<< ";" << convert_string_to_float(Dem[k][4])<< ";" << convert_string_to_float(Dem[k][5])<< ";" << convert_string_to_float(Dem[k][6])<< ";" << convert_string_to_float(Dem[k][7])<< ";" << convert_string_to_float(Dem[k][8])<< ";" << convert_string_to_float(Dem[k][9])<< ";" << convert_string_to_float(Dem[k][10])<< ";" << convert_string_to_float(Dem[k][11])<< ";" << convert_string_to_float(Dem[k][12]);
		Demand * dm = new Demand(convert_string_to_float(Dem[k][0]), convert_string_to_float(Dem[k][3]), convert_string_to_float(Dem[k][4])/17.0, convert_string_to_float(Dem[k][5]) , convert_string_to_float(Dem[k][12]));
		
		unsigned v1 = convert_string_to_float(Dem[k][1]);
		unsigned v2 = convert_string_to_float(Dem[k][2]);
		for(vector<Vertex*>::iterator it = nodes_.begin(); it!=nodes_.end(); it++)
		{
			if ((*it)->getIndex() == v1){
				dm->setOrigin(**it);
				dm->getOrigin().setWeight(2);}
			else if ((*it)->getIndex() == v2)
				dm->setDestination(**it);	
		}	
		requests_.push_back(dm);
	}
}