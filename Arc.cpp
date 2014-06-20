#include "Arc.hpp"

Arc::Arc(int a, int b, double c, double f)
{
	this->a = a;
	this->b = b;
	this->cost = c;
	this->flow = f;
	this->p = 0;		//arcos originais tem custo artificial zero
	this->tree = false;
	this->reverse = false;
}

void Arc::printArc()
{
	printf("%d -> %d\n",this->a, this->b);
}
bool Arc::isOriginal()
{
	if(this->p == 0)
		return true;
	else
		return false;
}

bool Arc::isReverse()
{
	return this->reverse;
}
void Arc::setReverse(bool r)
{
	this->reverse = r;
}
void Arc::setTree(bool v)
{
	this->tree = v;
}

bool Arc::isOnTree()
{
	return this->tree;
}

void Arc::setArtificialCost()
{
	this->p = 1;
}

int Arc::getTarget()
{
	return this->b;
}

int Arc::getOrigin()
{
	return this->a;
}

int Arc::getOpposite(int v)
{
	if(v == this->a)
		return this->b;
	else if(v == this->b)
		return this->a;
	else
	{
		return 0;
		printf("%d não é uma de minhas pontas!", v);
	}
}

double Arc::getCost(bool ini)
{
	if(ini) return this->p;
	else return this->cost;
}
void Arc::setFlow(double f)
{
	this->flow = f;
}
double Arc::getFlow()
{
	return this->flow;
}
