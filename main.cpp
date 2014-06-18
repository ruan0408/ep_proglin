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

	void printArco()
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

	int getTarget()
	{
		return this->b;
	}

	int getOrigin()
	{
		return this->a;
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

	double getCusto(bool ini)
	{
		if(ini) return this->p;
		else return this->custo;
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
		inc.resize(v);
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

	Arco* novoArco(int a, int b, int custo)
	{
		Arco arco (a, b, custo, 0);//arcos originais tem fluxo zero à principio
		Arco* ac;
		
		adj[a].push_back(arco);
		ac = &adj[a].back();

		inc[b].push_back(ac);

		return ac;
	}

	Arco* novoArco(int a, int b, int custo, double fluxo)
	{
		Arco arco (a, b, custo, fluxo);
		Arco* ac;

		adj[a].push_back(arco);
		ac = &adj[a].back();
		
		inc[b].push_back(ac);

		return ac;
	}

	void addArcosArtificiais(int v)//v vai ser a fonte
	{
		vector<bool> b(this->v, false);
		
		this->custoParcial[v] = 0;//custo parcial da raiz é nulo
		this->setDepth(v, 0);
		b[v] = true;//nao quero por arcos para mim mesmo

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
				Arco* a = novoArco(v, i, 0, this->getDemanda(i));//custo 'original' zero, por razoes
				a->setCustoArt();
				a->setTree(true);
				this->custoParcial[i] = 1;
				this->setDepth(i, 1);
			}

	}

	void removeArcosArtificiais(int v)//v acabará por ser sempre a fonte
	{
		list<Arco*>::iterator it;
		list<Arco>::iterator it2;
		/*PRIMEIRO APAGO AS REFERENCIAS*/
		for(int i = 0; i < this->v; i++)
			for(it = inc[i].begin(); it != inc[i].end(); it++)
				if(!(*it)->isOriginal())
					it = inc[i].erase(it);
		/*DEPOIS OS ARCOS DE FATO(só a fonte de arcos artificiais saindo)*/
		for(it2 = adj[v].begin(); it2 != adj[v].end(); it2++)
			if(!(*it2).isOriginal())
				it2 = adj[v].erase(it2);

	}

	Arco* procuraCandidato(bool ini)//ini diz se usaresmos os custos artificiais ou não
	{
		for(int i = 0; i < this->v; i++)
			for(auto& arc : this->adj[i])
			{
				if((!arc.isOnTree()) && ((custoParcial[i]+arc.getCusto(ini)) < custoParcial[arc.getTarget()]))
				{
					return &arc;
				}
			}

		return nullptr;
	}

	int achaCiclo(Arco* e, list<Arco*>& ciclo, double& min_flow)//retorna em qual dos lados houve o corte
	{
		int a = e->getOrigin();
		int b = e->getTarget();
		int c = a;
		int diff_c = 0;
		bool aux;
		//list<Arco>::iterator arc;
		//list<Arco*>::iterator arc_ptr;
		min_flow = INFINITY;
		
		ciclo.push_back(e);
		
		if(depth[a] > depth[b]) {c = a; diff_c = depth[a] - depth[b];}
		else if(depth[b] > depth[a]) {c = b; diff_c = depth[b] - depth[a];}
		
		int v_atual = c;//C É TAMBÉM A PONTA DA QUAL SURGIU O MENOR FLUXO, PELO MENOS À PRINCIPIO
		
		//printf("%d\n", diff_c);
		while(diff_c > 0)
		{
			aux = false;
			for(auto& arc : adj[v_atual])
			{
				if(arc.isOnTree() and depth[v_atual] > depth[arc.getTarget()])
				{	
					ciclo.push_back(&arc);
					aux = true;//nao precisa procurar nos incidentes
					if(c == a and arc.getFluxo() < min_flow)
					{
						arc.setReverso(true);
						min_flow = arc.getFluxo();
					}
					v_atual = arc.getTarget();
					break;
				}
			}
			if(!aux)
				for(auto& arc_ptr : inc[v_atual])
				//for(arc_ptr = inc[v_atual].begin(); arc_ptr != inc[v_atual].end(); arc_ptr++)
				{
					if(arc_ptr->isOnTree() and depth[v_atual] > depth[arc_ptr->getOrigin()])
					{
						ciclo.push_back(arc_ptr);
						if(c == b and arc_ptr->getFluxo() < min_flow)
						{
							arc_ptr->setReverso(true);
							min_flow = arc_ptr->getFluxo();
						}
						v_atual = arc_ptr->getOrigin();
						break;
					}
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
					if(c == a and arc.getFluxo() < min_flow)
					{
						arc.setReverso(true);
						min_flow = arc.getFluxo();
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
						if(c == b and arc_ptr->getFluxo() < min_flow)
						{
							arc_ptr->setReverso(true);
							min_flow = arc_ptr->getFluxo();
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
					if(c != a and arc.getFluxo() < min_flow)
					{
						arc.setReverso(true);
					 	min_flow = arc.getFluxo();
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
						if(c != b and arc_ptr->getFluxo() < min_flow)
						{
							arc_ptr->setReverso(true);
						 	min_flow = arc_ptr->getFluxo();
						 	ladoCorte = b;
						}
						v_atual2 = arc_ptr->getOrigin();
						break;
					}
				}
		}
		return ladoCorte;
	}

	void consertaArvore(list<Arco*>& ciclo, double min_flow, int ladoCorte, bool ini)//o primeiro elemento é o arco que entrará
	{
		Arco* arco_saida = nullptr;
		Arco* arco_entra;
		int a, b;
		int v_fixo;

		arco_entra = ciclo.front();
		a = arco_entra->getOrigin();
		b = arco_entra->getTarget();

		/********** ATUALIZA OS FLUXOS ************/
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
		//printf("ARCO SAIDA: ");arco_saida->printArco();

		/********* ATUALIZA AS PROFUNDIDADES E OS CUSTOS PARCIAIS *******/
		v_fixo = arco_entra->getOposto(ladoCorte);
		this->setDepth(ladoCorte, getDepth(v_fixo)+1);
		
		if(ladoCorte == a)//ladoCorte é a bunda
			custoParcial[ladoCorte] = custoParcial[b] - arco_entra->getCusto(ini);
		else//ladoCorte é a cabeça
			custoParcial[ladoCorte] = custoParcial[a] + arco_entra->getCusto(ini);

		this->arrumaDepthCusto(ladoCorte, ini);//ainda não inclui o novo arco na arvore

		/************ DESMARCO OS REVERSOS *****************/
		for(it = ciclo.begin(); it != ciclo.end(); it++)
			(*it)->setReverso(false);
		
		/****** PONHO O ARCO DE ENTRADA NA ARVORE ******/
		arco_entra->setTree(true);
	}

	void arrumaDepthCusto(int fix_me, bool ini)//este nó está com custoParcial e depth correto
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
					custoParcial[vizinho] = custoParcial[v_atual] + (*it1).getCusto(ini);//arruma custoParcial
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
					custoParcial[vizinho] = custoParcial[v_atual] - (*it2)->getCusto(ini);//arruma custoParcial
					fila.push(vizinho);
					lbl[vizinho] = true;
				}	
			}
		}
	}

	bool inicializa(int fonte)
	{
		addArcosArtificiais(fonte);
		Arco* cand;
		list<Arco*> ciclo;
		double min_flow;
		int ladoCorte;
		
		/*PROCURO SOLUÇÃO INICIAL*/
		while((cand = procuraCandidato(true)) != nullptr)//true pois estamos usando os custos artificiais
		{
			/*for(int i = 0; i< this->v; i++)
				printf("NÓ %d : custo %f, depth %d\n", i, custoParcial[i], depth[i]);
	
			printf("ARCO ENTRADA: ");cand->printArco();*/
			
			ladoCorte = achaCiclo(cand, ciclo, min_flow);//min_flow é alterado la dentro
			/*for(auto& arco : ciclo)
			{
				printf("\t");
				arco->printArco();
			}
			printf("LADO CORTE %d\n", ladoCorte);*/

			consertaArvore(ciclo, min_flow, ladoCorte, true);
			ciclo.clear();
		}
		//for(int i = 0; i< this->v; i++)
		//		printf("NÓ %d : custo %f, depth %d\n", i, custoParcial[i], depth[i]);

		/*VERIFICO SE TODOS OS ARCOS DA ARVORE SÃO ORIGINAIS*/
		list<Arco>::iterator it;
		for(it = adj[fonte].begin(); it != adj[fonte].end(); it++)
			if((*it).isOnTree() and !(*it).isOriginal())
				return false;
		
		/*for(int i = 0; i< this->v; i++)
		{
			printf("CUSTO NÓ %d : %f\n", i, custoParcial[i]);
			printf("DEPTH NÓ %d : %d\n", i, depth[i]);
			for(it = adj[i].begin(); it != adj[i].end(); it++)
				if((*it).isOnTree())
				{
					printf("arco: ");
					(*it).printArco();
					printf("\t custo: %f\n",(*it).getCusto(true));
					printf("\t fluxo: %f\n",(*it).getFluxo());
				}
		}*/
		/*REMOVO ARCOS ARTIFICIAIS*/
		removeArcosArtificiais(fonte);
		
		/*DEIXO DEPTH E CUSTOS INCIAIS PRONTOS*/
		arrumaDepthCusto(fonte, false);

		for(int i = 0; i< this->v; i++)
			printf("NÓ %d : custo %f, depth %d\n", i, custoParcial[i], depth[i]);

		for(int i = 0; i< this->v; i++)
			for(auto& arco : adj[i])
				if(arco.isOnTree())
				{
					printf("arco: ");
					arco.printArco();
					//printf("\t custo: %f\n",(*it).getCusto(true));
					printf("\t fluxo: %f\n", arco.getFluxo());
				}
		

		return true;
	}

	void executaSimplexRede()//o grafo já está com uma arvore factivel
	{
		Arco* cand;
		list<Arco*> ciclo;
		double min_flow;
		int ladoCorte;

		/*PARO QUANDO NÃO HOUVER MAIS CANDIDATOS À ENTRAR NA ÁRVORE*/
		while((cand = procuraCandidato(false)) != nullptr)//false pois estamos usando os custos reais
		{
			ladoCorte = achaCiclo(cand, ciclo, min_flow);//min_flow é alterado la dentro
			consertaArvore(ciclo, min_flow, ladoCorte, false);
			ciclo.clear();
		}
	}

	void mostraResposta(int pia)//imprime o custo para chegar na pia
	{
		printf("%f\n",this->custoParcial[pia]);
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

	if(!g.inicializa(fonte))
	{
		printf("Problema inviável\n");
		return 1;
	}

	g.executaSimplexRede();
	g.mostraResposta(pia);
}