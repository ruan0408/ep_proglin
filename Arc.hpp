#ifndef HPP_ARC
#define HPP_ARC

#include <cstdio>

class Arc
{
private:
	int a;
	int b;
	double cost;
	double flow;
	int p;			//custo artificial(0 ou 1, só é utilizado na inicialização)
	bool tree;		//esta na arvore atual?
	bool reverse;	//é arco reverso?
public:
	Arc(int a, int b, double c, double f);
	void printArc();
	bool isOriginal();
	bool isReverse();
	void setReverse(bool r);
	void setTree(bool v);
	bool isOnTree();
	void setArtificialCost();
	int getTarget();
	int getOrigin();
	int getOpposite(int v);
	double getCost(bool ini);
	void setFlow(double f);
	double getFlow();	
};

#endif