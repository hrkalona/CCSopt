/*
 * NetEdge.hpp
 *
 *  Created on: May 8, 2014
 *      Author: hrkalona
 */

#ifndef NETEDGE_HPP_
#define NETEDGE_HPP_

#include <string>
#include "types.hpp"
#include "Edge.hpp"

using namespace std;

class NetEdge: public Edge
{
private:
	double capacitance;
	double resistance;
	double Sot_hat_fall_sqr;
	double Sot_hat_rise_sqr;
public:
	NetEdge(string);
	~NetEdge()
	{
	}
	;
	NET_TYPE getNetType();
	void setCapacitance(double);
	double getCapacitance();
	void setResistance(double);
	double getResistance();
	void setBFactorFall(double);
	void setBFactorRise(double);
	double getOutputTransitionFall(double);
	double getOutputTransitionRise(double);

};

#endif /* NETEDGE_HPP_ */
