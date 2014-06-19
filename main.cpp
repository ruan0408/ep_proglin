#include <iostream>
#include <vector>
#include <list>
#include <stdio.h>
#include <cmath>
#include <queue>
#include <fstream>
#include <string>

using namespace std;

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
	Arc(int a, int b, double c, double f)
	{
		this->a = a;
		this->b = b;
		this->cost = c;
		this->flow = f;
		this->p = 0;		//arcos originais tem custo artificial zero
		this->tree = false;
		this->reverse = false;
	}

	void printArc()
	{
		printf("%d -> %d\n",this->a, this->b);
	}
	bool isOriginal()
	{
		if(this->p == 0)
			return true;
		else
			return false;
	}

	bool isReverse()
	{
		return this->reverse;
	}
	void setReverse(bool r)
	{
		this->reverse = r;
	}
	void setTree(bool v)
	{
		this->tree = v;
	}

	bool isOnTree()
	{
		return this->tree;
	}

	void setArtificialCost()
	{
		this->p = 1;
	}

	int getTarget()
	{
		return this->b;
	}

	int getOrigin()
	{
		return this->a;
	}

	int getOpposite(int v)
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

	double getCost(bool ini)
	{
		if(ini) return this->p;
		else return this->cost;
	}
	void setFlow(double f)
	{
		this->flow = f;
	}
	double getFlow()
	{
		return this->flow;
	}

};

class Network
{
private:
	int v;
	int source;
	int sink;
	
	vector<double> demand;
	vector<double> parcialCost;
	vector<int> depth;

	int getDepth(int v)
	{
		return this->depth[v];
	}

	void setDepth(int v, int val)
	{
		this->depth[v] = val;
	}

	void setParcialCost(int v, double val)
	{
		this->parcialCost[v] = val;
	}

	double getParcialCost(int v)
	{
		return this->parcialCost[v];
	}

	void setDemand(int v, double dem)
	{
		this->demand[v] = dem;
	}

	double getDemand(int v)
	{
		return this->demand[v];
	}

	void addArtificialArcs(int fonte)//vai ser a fonte
	{
		vector<bool> label(this->getV(), false);
		
		this->setParcialCost(fonte, 0);//custo parcial da raiz é nulo
		this->setDepth(fonte, 0);
		label[fonte] = true;//nao quero por arcos para mim mesmo

		for(auto& arc : this->adj[fonte])
		{
			label[arc.getTarget()] = true;
			this->setParcialCost(arc.getTarget(), 0);
			arc.setFlow(getDemand(arc.getTarget()));
			arc.setTree(true);
			this->setDepth(arc.getTarget(), 1);
		}
		
		for(int i = 0; i < label.size(); i++)
		{
			if(label[i] == false)
			{
				Arc* a = this->newArc(fonte, i, 0, this->getDemand(i));//custo 'original' zero, por razoes
				a->setArtificialCost();//custo artificial 1
				a->setTree(true);
				this->setParcialCost(i, 1);
				this->setDepth(i, 1);
			}
		}
	}

	void removeArtificialArcs(int fonte)//v acabará por ser sempre a fonte
	{
		list<Arc*>::iterator it;
		list<Arc>::iterator it2;
		
		/*PRIMEIRO APAGO AS REFERENCIAS*/
		for(int i = 0; i < this->getV(); i++)
			for(it = inc[i].begin(); it != inc[i].end();)
				if(!(*it)->isOriginal())
					it = inc[i].erase(it);
				else
					it++;
		/*DEPOIS OS ARCOS DE FATO(só a fonte de arcos artificiais saindo)*/
		for(it2 = adj[fonte].begin(); it2 != adj[fonte].end();)
			if(!(*it2).isOriginal())
				it2 = adj[fonte].erase(it2);
			else
				it2++;
	}

	Arc* findEnteringArc(bool ini)//ini diz se usaresmos os custos artificiais ou não
	{
		for(int i = 0; i < this->getV(); i++)
			for(auto& arc : this->adj[i])
				if(!arc.isOnTree() and ((getParcialCost(i)+arc.getCost(ini)) < getParcialCost(arc.getTarget())))
					return &arc;

		return nullptr;
	}

	int findCycle(Arc* e, list<Arc*>& ciclo, double& min_flow)//retorna em qual dos lados houve o corte
	{
		int a = e->getOrigin();
		int b = e->getTarget();
		int c = a;
		int diff_c = 0;
		bool aux;
		
		min_flow = INFINITY;
		
		ciclo.push_back(e);
		
		if(depth[a] > depth[b]) {c = a; diff_c = depth[a] - depth[b];}
		else if(depth[b] > depth[a]) {c = b; diff_c = depth[b] - depth[a];}
		
		int v_atual = c;//C É TAMBÉM A PONTA DA QUAL SURGIU O MENOR FLUXO, PELO MENOS À PRINCIPIO
		
		while(diff_c > 0)
		{
			aux = false;
			for(auto& arc : adj[v_atual])
				if(arc.isOnTree() and depth[v_atual] > depth[arc.getTarget()])
				{	
					ciclo.push_back(&arc);
					aux = true;//nao precisa procurar nos incidentes
					if(c == a and arc.getFlow() < min_flow)
					{
						arc.setReverse(true);
						min_flow = arc.getFlow();
					}
					v_atual = arc.getTarget();
					break;
				}
			if(!aux)
				for(auto& arc_ptr : inc[v_atual])
					if(arc_ptr->isOnTree() and depth[v_atual] > depth[arc_ptr->getOrigin()])
					{
						ciclo.push_back(arc_ptr);
						if(c == b and arc_ptr->getFlow() < min_flow)
						{
							arc_ptr->setReverse(true);
							min_flow = arc_ptr->getFlow();
						}
						v_atual = arc_ptr->getOrigin();
						break;
					}
			diff_c--;
		}

		int ladoCorte = c;
		int v_atual1 = v_atual;//C ESTÁ VINCULADO COM V_ATUAL1
		int v_atual2;
		if(c == a) v_atual2 = b;
		else v_atual2 = a;

		while(v_atual1 != v_atual2)
		{
			/************** ANALISA LADO QUE ERA O MAIOR ANTES ***********/
			aux = false;
			for(auto& arc : adj[v_atual1])
			{	
				if(arc.isOnTree() and depth[v_atual1] > depth[arc.getTarget()])
				{
					ciclo.push_back(&arc);
					aux = true;//nao precisa procurar nos incidentes
					if(c == a and arc.getFlow() < min_flow)
					{
						arc.setReverse(true);
						min_flow = arc.getFlow();
						ladoCorte = a;
					}
					v_atual1 = arc.getTarget();
					break;
				}
			}
	
			if(!aux)
				for(auto& arc_ptr : inc[v_atual1])
				{
					if(arc_ptr->isOnTree() and depth[v_atual1] > depth[arc_ptr->getOrigin()])
					{
						ciclo.push_back(arc_ptr);
						if(c == b and arc_ptr->getFlow() < min_flow)
						{
							arc_ptr->setReverse(true);
							min_flow = arc_ptr->getFlow();
							ladoCorte = b;
						}
						v_atual1 = arc_ptr->getOrigin();
						break;
					}
				}
			/********************** OUTRO LADO **************************/
			aux = false;
			for(auto& arc : adj[v_atual2])
			{
				if(arc.isOnTree() and depth[v_atual2] > depth[arc.getTarget()])
				{
					ciclo.push_back(&arc);
					aux = true;//nao precisa procurar nos incidentes
					if(c != a and arc.getFlow() < min_flow)
					{
						arc.setReverse(true);
					 	min_flow = arc.getFlow();
					 	ladoCorte = a;
					}
					v_atual2 = arc.getTarget();
					break;
				}
			}
			if(!aux)
				for(auto& arc_ptr : inc[v_atual2])
				{
					if(arc_ptr->isOnTree() and depth[v_atual2] > depth[arc_ptr->getOrigin()])
					{
						ciclo.push_back(arc_ptr);
						if(c != b and arc_ptr->getFlow() < min_flow)
						{
							arc_ptr->setReverse(true);
						 	min_flow = arc_ptr->getFlow();
						 	ladoCorte = b;
						}
						v_atual2 = arc_ptr->getOrigin();
						break;
					}
				}
		}
		return ladoCorte;
	}

	void fixTree(list<Arc*>& ciclo, double min_flow, int ladoCorte, bool ini)//o primeiro elemento é o arco que entrará
	{
		Arc* arco_saida = nullptr;
		Arc* arco_entra;
		int a, b;
		int v_fixo;

		arco_entra = ciclo.front();
		a = arco_entra->getOrigin();
		b = arco_entra->getTarget();

		/********** ATUALIZA OS FLUXOS ************/
		list<Arc*>::iterator it;
		for(it = ciclo.begin(); it != ciclo.end(); it++)
		{
			if((*it)->isReverse())
			{
				(*it)->setFlow((*it)->getFlow() - min_flow);
				if(arco_saida == nullptr){arco_saida = *it;}
			}
			else
				(*it)->setFlow((*it)->getFlow() + min_flow);
		}
		/****** TIRA ARCO DA ARVORE **************/
		arco_saida->setTree(false);

		/********* ATUALIZA AS PROFUNDIDADES E OS CUSTOS PARCIAIS *******/
		v_fixo = arco_entra->getOpposite(ladoCorte);
		this->setDepth(ladoCorte, getDepth(v_fixo)+1);
		
		if(ladoCorte == a)//ladoCorte é a origem
			setParcialCost(ladoCorte, getParcialCost(b) - arco_entra->getCost(ini));
		else//ladoCorte é o destino
			setParcialCost(ladoCorte, getParcialCost(a) + arco_entra->getCost(ini));
			
		this->fixDepthCost(ladoCorte, ini);//ainda não inclui o novo arco na arvore

		/************ DESMARCO OS REVERSOS *****************/
		for(it = ciclo.begin(); it != ciclo.end(); it++)
			(*it)->setReverse(false);
		
		/****** PONHO O ARCO DE ENTRADA NA ARVORE ******/
		arco_entra->setTree(true);
	}

	void fixDepthCost(int fix_me, bool ini)//este nó está com custoParcial e depth correto
	{
		vector<bool> lbl(this->getV(), false);
		queue<int> fila;
		int v_atual, vizinho;
		bool aux;
		list<Arc>::iterator it1;
		list<Arc*>::iterator it2;

		fila.push(fix_me);
		lbl[fix_me] = true;
		while(!fila.empty())
		{
			v_atual = fila.front();
			fila.pop();
			aux = false;
			
			for(it1 = adj[v_atual].begin(); it1 != adj[v_atual].end(); it1++)//olho lista de adjacencia
			{
				vizinho = (*it1).getTarget();
				if((*it1).isOnTree() and !lbl[vizinho])
				{
					setDepth(vizinho, getDepth(v_atual)+1);//arruma depth
					setParcialCost(vizinho, getParcialCost(v_atual) + (*it1).getCost(ini));//arruma custo parcial
					fila.push(vizinho);
					lbl[vizinho] = true;
					aux = true;
				}
			}
			for(it2 = inc[v_atual].begin(); !aux and it2 != inc[v_atual].end(); it2++)//olho lista de incidencia
			{
				vizinho = (*it2)->getOrigin();
				if((*it2)->isOnTree() and !lbl[vizinho])
				{
					setDepth(vizinho, getDepth(v_atual)+1);//arruma depth
					setParcialCost(vizinho, getParcialCost(v_atual) - (*it2)->getCost(ini));//arruma custo parcial
					fila.push(vizinho);
					lbl[vizinho] = true;
				}	
			}
		}
	}

public:
	vector<list<Arc> > adj;
	vector<list<Arc*> > inc;

	Network(fstream& file)
	{
		int demand, origin, target;
		double cost;

		file >> this->v;
		file >> this->source;
		file >> this->sink;
		file >> demand;
		
		this->adj.resize(this->v);
		this->inc.resize(this->v);
		this->depth.resize(this->v);
		this->demand.resize(this->v);
		this->parcialCost.resize(this->v);

		for(int i = 0; i < this->v; i++)
			this->setDemand(i, 0);

		this->setDemand(this->getSource(), -demand);
		this->setDemand(this->getSink(), demand);
		
		for(int i = 0; i < this->v; i++)
			this->setParcialCost(i, INFINITY);

		this->setParcialCost(this->getSource(), 0);

		while(file >> origin >> target >> cost)
			this->newArc(origin, target, cost, 0);//fluxo zero
	}

	int getSource()
	{
		return this->source;
	}

	int getSink()
	{
		return this->sink;
	}

	int getV()
	{
		return this->v;
	}

	Arc* newArc(int a, int b, int custo, double fluxo)
	{
		Arc arco (a, b, custo, fluxo);
		Arc* ac;

		adj[a].push_back(arco);
		ac = &adj[a].back();
		
		inc[b].push_back(ac);

		return ac;
	}

	bool initialize()
	{
		int fonte = this->getSource();
		this->addArtificialArcs(fonte);
		
		Arc* cand;
		list<Arc*> ciclo;
		double min_flow;
		int ladoCorte;
		
		/*PROCURO SOLUÇÃO INICIAL*/
		while((cand = this->findEnteringArc(true)) != nullptr)//true pois estamos usando os custos artificiais
		{
			ladoCorte = this->findCycle(cand, ciclo, min_flow);//min_flow é alterado la dentro
			fixTree(ciclo, min_flow, ladoCorte, true);
			ciclo.clear();
		}

		/*VERIFICO SE TODOS OS ARCOS DA ARVORE SÃO ORIGINAIS*/
		list<Arc>::iterator it;
		for(it = adj[fonte].begin(); it != adj[fonte].end(); it++)
			if((*it).isOnTree() and !(*it).isOriginal())
				return false;
		
		/*REMOVO ARCOS ARTIFICIAIS*/
		removeArtificialArcs(fonte);
		
		/*DEIXO DEPTH E CUSTOS INCIAIS PRONTOS*/
		fixDepthCost(fonte, false);

		return true;
	}

	void executeSimplex()//o grafo já está com uma arvore factivel
	{
		Arc* cand;
		list<Arc*> ciclo;
		double min_flow;
		int ladoCorte;

		/*PARO QUANDO NÃO HOUVER MAIS CANDIDATOS À ENTRAR NA ÁRVORE*/
		while((cand = this->findEnteringArc(false)) != nullptr)//false pois estamos usando os custos reais
		{
			ladoCorte = this->findCycle(cand, ciclo, min_flow);//min_flow é alterado la dentro
			fixTree(ciclo, min_flow, ladoCorte, false);
			ciclo.clear();
		}
	}

	void showAnswer()//imprime o custo para chegar na pia
	{
		int pia = this->getSink();

		for(int i = 0; i < this->getV(); i++)
			for(auto& arco : adj[i])
				if(arco.isOnTree())
					printf("%d, %d - %f\n",arco.getOrigin(), arco.getTarget(), arco.getFlow());
		
		printf("%f\n",this->getParcialCost(pia));
	}
};

int main(int argc, char* argv[])
{
	fstream entrada;
	fstream saida;
	
	string nome_entra = "";
	nome_entra += argv[1];
	entrada.open(nome_entra);
	
	Network net(entrada);
	entrada.close();
	
	if(!net.initialize())
	{
		printf("Problema inviável\n");
		return 1;
	}

	net.executeSimplex();
	net.showAnswer();
}