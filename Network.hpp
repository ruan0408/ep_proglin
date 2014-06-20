#ifndef HPP_NETWORK
#define HPP_NETWORK

#include <iostream>
#include <vector>
#include <list>
#include <cstdio>
#include <cmath>
#include <queue>
#include <fstream>
#include <string>
#include "Arc.hpp"

class Network
{
private:
	int v;
	int source;
	int sink;
	
	std::vector<std::list<Arc> > adj;			//lista de adjacencia
	std::vector<std::list<Arc*> > inc;		//lista de ponteiros de arcos incidentes

	std::vector<double> demand;
	std::vector<double> parcialCost;
	std::vector<int> depth;

	int getDepth(int v);
	void setDepth(int v, int val);
	void setParcialCost(int v, double val);
	double getParcialCost(int v);
	void setDemand(int v, double dem);
	double getDemand(int v);
	void addArtificialArcs(int fonte);							//vai ser a fonte
	void removeArtificialArcs(int fonte);						//v acabará por ser sempre a fonte
	Arc* findEnteringArc(bool ini);								//ini diz se usaresmos os custos artificiais ou não
	int upOnCycle(int v, int lado_v, int& ladoCorte, Arc* e, std::list<Arc*>& ciclo, double& min_flow);//anda no ciclo
	int findCycle(Arc* e, std::list<Arc*>& ciclo, double& min_flow);	//retorna em qual dos lados houve o corte
	void fixTree(std::list<Arc*>& ciclo, double min_flow, int ladoCorte, bool ini);//o primeiro elemento do ciclo é o arco que entrará
	void fixDepthCost(int fix_me, bool ini);					//este nó está com custoParcial e depth correto
public:
	Network(std::fstream& entrada);
	int getSource();
	int getSink();
	int getV();
	Arc* newArc(int a, int b, int custo, double fluxo);
	std::list<Arc>& neighborsOf(int v);
	std::list<Arc*>& incidentArcsOn(int v);
	bool initialize();
	void executeSimplex();	//usado quando o grafo já está com uma arvore factivel
	void showAnswer(std::ofstream& saida);		//imprime o custo para chegar na pia(sink)
};
#endif