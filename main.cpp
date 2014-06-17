#include <iostream>
#include <vector>
#include <list>
#include <stdio.h>
#include <cmath>
#include <queue>

using namespace std;

class Arco
{

private:
	int a;
	int b;
	double custo;
	double fluxo;
	int p;//custo artificial
	bool tree;
	bool reverso;
public:
	Arco(int a, int b, double c, double f)
	{
		this->a = a;
		this->b = b;
		this->custo = c;
		this->fluxo = f;
		this->p = 0;//arcos originais tem custo artificial zero
		this->tree = false;
		this->reverso = false;
	}

	int getOposto(int v)
	{
		if(v == this->a)
			return this->b;
		else if(v == this->b)
			return this->a;
		else
		{
			return 0;
			printf("This shouldn't have happened!");
		}
	}
	bool isReverso()
	{
		return this->reverso;
	}
	void setReverso(bool r)
	{
		this->reverso = r;
	}
	void setTree(bool v)
	{
		this->tree = v;
	}

	bool isOnTree()
	{
		return this->tree;
	}

	void setCustoArt()
	{
		this->p = 1;
	}

	int getCustoIni()
	{
		return this->p;
	}

	int getTarget()
	{
		return this->b;
	}

	int getOrigin()
	{
		return this->a;
	}

	double getCusto()
	{
		return this->custo;
	}
	void setFluxo(double f)
	{
		this->fluxo = f;
	}
	double getFluxo()
	{
		return this->fluxo;
	}

};

class Grafo
{
private:
	int v;
	
	vector<double> demanda;
	vector<double> custoParcial;
	vector<int> depth;

public:
	vector<list<Arco> > adj;
	vector<list<Arco*> > inc;
	Grafo(int v)
	{
		this->v = v;
		adj.resize(v);
		demanda.resize(v);
		depth.resize(v);
		custoParcial.resize(v);
		for(int i = 0; i < custoParcial.size(); i++)
			custoParcial[i] = INFINITY;
	}

	int getDepth(int v)
	{
		return this->depth[v];
	}

	void setDepth(int v, int val)
	{
		this->depth[v] = val;
	}

	void atualizaCusto(int v, double val)
	{
		custoParcial[v] = val;
	}

	void setDemanda(int v, double dem)
	{
		demanda[v] = dem;
	}

	double getDemanda(int v)
	{
		return this->demanda[v];
	}

	Arco novoArco(int a, int b, int custo)
	{
		Arco arco (a, b, custo, 0);//fluxo 0
		adj[a].push_back(arco);

		//Arco arco_inc(a, custo, 0);
		inc[b].push_back(&arco);

		return arco;
	}

	Arco novoArco(int a, int b, int custo, double fluxo)
	{
		Arco arco (a, b, custo, fluxo);
		adj[a].push_back(arco);

		//Arco arco_inc(a, custo, fluxo);
		inc[b].push_back(&arco);

		return arco;
	}

	void addArcosArtificiais(int v)//v vai ser a fonte
	{
		vector<bool> b(this->v, false);
		//for (list<Arco>::iterator arc = adj[v].begin(); arc != adj[v].end(); arc++)
		//	b[arc.getTarget() ] = true;
		
		this->custoParcial[v] = 0;//custo parcial da raiz é nulo
		this->setDepth(v, 0);

		for(auto& arc : this->adj[v])
		{
			b[arc.getTarget()] = true;
			this->custoParcial[arc.getTarget()] = 0;
			arc.setTree(true);
			this->setDepth(arc.getTarget(), 1);
		}

		for(int i = 0; i < b.size(); i++)
			if(b[i] == false)
			{
				Arco a = novoArco(v, i, 0, this->getDemanda(i));
				a.setCustoArt();//custo 'original' zero, por razoes
				a.setTree(true);
				this->custoParcial[i] = 1;
			}

	}

	Arco* procuraCandidato()
	{
		for(int i = 0; i < this->v; i++)
			for(auto& arc : this->adj[i])
				if(!arc.isOnTree() and custoParcial[i]+arc.getCustoIni() < custoParcial[arc.getTarget()])
					return &arc;
		//nao achei, e ai??
		return nullptr;
	}

	int achaCiclo(Arco* e, list<Arco*> ciclo, double* min_flow)//retorna em qual dos lados houve o corte
	{
		int a = e->getOrigin();
		int b = e->getTarget();
		int c;
		int diff_c;
		bool aux;
		*min_flow = INFINITY;
		//list<Arco*> ciclo;

		ciclo.push_back(e);

		if(depth[a] > depth[b]) {c = a; diff_c = depth[a] - depth[b];}
		else if(depth[b] > depth[a]) {c = b; diff_c = depth[b] - depth[a];}
		
		int v_atual = c;//C É TAMBÉM A PONTA DA QUAL SURGIU O MENOR FLUXO, PELO MENOS À PRINCIPIO
		while(diff_c > 0)
		{
			for(auto& arc : adj[v_atual])
				if(arc.isOnTree() and depth[v_atual] > depth[arc.getTarget()])
				{
					ciclo.push_back(&arc);
					aux = true;//nao precisa procurar nos incidentes
					if(c == a and arc.getFluxo() < *min_flow)
					{
						arc.setReverso(true);
						*min_flow = arc.getFluxo();

					}
					v_atual = arc.getTarget();
					break;
				}
			if(!aux)
				for(auto& arc_ptr : inc[v_atual])
					if(arc_ptr->isOnTree() and depth[v_atual] > depth[arc_ptr->getOrigin()])
					{
						ciclo.push_back(arc_ptr);
						if(c == b and arc_ptr->getFluxo() < *min_flow)
						{
							arc_ptr->setReverso(true);
							*min_flow = arc_ptr->getFluxo();
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
				if(arc.isOnTree() and depth[v_atual1] > depth[arc.getTarget()])
				{
					ciclo.push_back(&arc);
					aux = true;//nao precisa procurar nos incidentes
					if(c == a and arc.getFluxo() < *min_flow)
					{
						arc.setReverso(true);
						*min_flow = arc.getFluxo();
						ladoCorte = a;
					}
					v_atual1 = arc.getTarget();
					break;
				}
			if(!aux)
				for(auto& arc_ptr : inc[v_atual1])
					if(arc_ptr->isOnTree() and depth[v_atual1] > depth[arc_ptr->getOrigin()])
					{
						ciclo.push_back(arc_ptr);
						if(c == b and arc_ptr->getFluxo() < *min_flow)
						{
							arc_ptr->setReverso(true);
							*min_flow = arc_ptr->getFluxo();
							ladoCorte = b;
						}
						v_atual1 = arc_ptr->getOrigin();
						break;
					}
			/********************** OUTRO LADO **************************/
			aux = false;
			for(auto& arc : adj[v_atual2])
				if(arc.isOnTree() and depth[v_atual2] > depth[arc.getTarget()])
				{
					ciclo.push_back(&arc);
					aux = true;//nao precisa procurar nos incidentes
					if(c != a and arc.getFluxo() < *min_flow)
					{
						arc.setReverso(true);
					 	*min_flow = arc.getFluxo();
					 	ladoCorte = b;
					}
					v_atual2 = arc.getTarget();
					break;
				}
			if(!aux)
				for(auto& arc_ptr : inc[v_atual2])
					if(arc_ptr->isOnTree() and depth[v_atual2] > depth[arc_ptr->getOrigin()])
					{
						ciclo.push_back(arc_ptr);
						if(c != b and arc_ptr->getFluxo() < *min_flow)
						{
							arc_ptr->setReverso(true);
						 	*min_flow = arc_ptr->getFluxo();
						 	ladoCorte = a;
						}
						v_atual2 = arc_ptr->getOrigin();
						break;
					}
		}
		return ladoCorte;
	}

	void consertaArvore(list<Arco*> ciclo, double min_flow, int ladoCorte)//o primeiro elemento é o arco que entrará
	{
		Arco* arco_saida = nullptr;
		Arco* arco_entra;
		int a, b;
		int v_fixo;

		arco_entra = ciclo.front();
		a = arco_entra->getOrigin();
		b = arco_entra->getTarget();

		/*ATUALIZA OS FLUXOS*/
		list<Arco*>::iterator it;
		for(it = ciclo.begin(); it != ciclo.end(); it++)
		{
			if((*it)->isReverso())
			{
				(*it)->setFluxo((*it)->getFluxo() - min_flow);
				if(arco_saida == nullptr){arco_saida = *it;}
			}
			else
				(*it)->setFluxo((*it)->getFluxo() + min_flow);
		}
		/****** TIRA ARCO DA ARVORE **************/
		arco_saida->setTree(false);

		/********* ATUALIZA AS PROFUNDIDADES E OS CUSTOS PARCIAIS *******/
		v_fixo = arco_entra->getOposto(ladoCorte);
		this->setDepth(ladoCorte, getDepth(v_fixo)+1);
		
		if(ladoCorte == a)//ladoCorte é a bunda
			custoParcial[ladoCorte] = custoParcial[b] - arco_entra->getCusto();
		else//ladoCorte é a cabeça
			custoParcial[ladoCorte] = custoParcial[a] + arco_entra->getCusto();

		this->arrumaDepthCustoP(ladoCorte);//ainda não inclui o novo arco na arvore

		/************ DESMARCO OS REVERSOS *****************/
		for(it = ciclo.begin(); it != ciclo.end(); it++)
			(*it)->setReverso(false);
		
		/****** PONHO O ARCO DE ENTRADA NA ARVORE ******/
		arco_entra->setTree(true);
	}

	void arrumaDepthCustoP(int fix_me)//este nó está com custoParcial e depth correto
	{
		vector<bool> lbl(this->v, false);
		queue<int> fila;
		int v_atual, vizinho;
		bool aux;
		list<Arco>::iterator it1;
		list<Arco*>::iterator it2;

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
					custoParcial[vizinho] = custoParcial[v_atual] + (*it1).getCusto();//arruma custoParcial
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
					custoParcial[vizinho] = custoParcial[v_atual] - (*it2)->getCusto();//arruma custoParcial
					fila.push(vizinho);
					lbl[vizinho] = true;
				}	
			}
		}
	}
};


int main()
{
	int n, fonte, pia, a, b, custo;
	double dem;
	
	cin >> n;
	cin >> fonte;
	cin >> pia;
	cin >> dem;

	Grafo g(n);

	g.setDemanda(fonte, -dem);
	g.setDemanda(pia, dem);

	while(cin >> a >> b >> custo)
	{
		g.novoArco(a, b, custo);
	}

	printf("%f \n", g.getDemanda(fonte));
	cout << g.adj[fonte].front().getTarget();
}