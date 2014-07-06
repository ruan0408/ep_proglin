
#include "Network.hpp"

using namespace std;

int Network::getDepth(int v)
{
	return this->depth[v];
}

void Network::setDepth(int v, int val)
{
	this->depth[v] = val;
}

void Network::setParcialCost(int v, double val)
{
	this->parcialCost[v] = val;
}

double Network::getParcialCost(int v)
{
	return this->parcialCost[v];
}

void Network::setDemand(int v, double dem)
{
	this->demand[v] = dem;
}

double Network::getDemand(int v)
{
	return this->demand[v];
}

void Network::addArtificialArcs(int fonte)//vai ser a fonte
{
	vector<bool> label(this->getV(), false);
	
	this->setParcialCost(fonte, 0);//custo parcial da raiz é nulo
	this->setDepth(fonte, 0);
	label[fonte] = true;//nao quero por arcos para mim mesmo

	for(auto& arc : this->neighborsOf(fonte))
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

void Network::removeArtificialArcs(int fonte)//v acabará por ser sempre a fonte
{
	list<Arc*>::iterator it;
	list<Arc>::iterator it2;
	
	/*PRIMEIRO APAGO AS REFERENCIAS*/
	for(int i = 0; i < this->getV(); i++)
		for(it = this->incidentArcsOn(i).begin(); it != this->incidentArcsOn(i).end();)
			if(!(*it)->isOriginal())
				it = this->incidentArcsOn(i).erase(it);
			else
				it++;
	/*DEPOIS OS ARCOS DE FATO(só a fonte de arcos artificiais saindo)*/
	for(it2 = this->neighborsOf(fonte).begin(); it2 != this->neighborsOf(fonte).end();)
		if(!(*it2).isOriginal())
			it2 = this->neighborsOf(fonte).erase(it2);
		else
			it2++;
}

Arc* Network::findEnteringArc(bool ini)//ini diz se usaresmos os custos artificiais ou não
{
	for(int i = 0; i < this->getV(); i++)
		for(auto& arc : this->neighborsOf(i))
			if(!arc.isOnTree() and ((getParcialCost(i)+arc.getCost(ini)) < getParcialCost(arc.getTarget())))
				return &arc;

	return nullptr;
}

int Network::findCycle(Arc* e, list<Arc*>& ciclo, double& min_flow)//retorna em qual dos lados houve o corte
{
	int a = e->getOrigin();
	int b = e->getTarget();
	int diff = 0;		//diferença de profundidade entre as pontas
	int ladoCorte;
	int v1, v2, lado_v1, lado_v2;
	v1 = a; 			//v1 é o maior lado. Escolho arbitrariamente a, à principio.
	lado_v1 = a;
	
	min_flow = INFINITY;
	
	ciclo.push_back(e);
	
	if(getDepth(a) > getDepth(b)) {v1 = a; diff = getDepth(a) - getDepth(b); lado_v1 = a;}
	else if(getDepth(b) > getDepth(a)) {v1 = b; diff = getDepth(b) - getDepth(a); lado_v1 = b;}
	
	while(diff > 0)
	{
		v1 = this->upOnCycle(v1, lado_v1, ladoCorte, e, ciclo, min_flow);
		diff--;
	}

	if(lado_v1 == a) {v2 = b; lado_v2 = b;}
	else {v2 = a; lado_v2 = a;}

	while(v1 != v2)
	{
		v1 = this->upOnCycle(v1, lado_v1, ladoCorte, e, ciclo, min_flow);
		v2 = this->upOnCycle(v2, lado_v2, ladoCorte, e, ciclo, min_flow);
	}
	return ladoCorte;
}

int Network::upOnCycle(int v, int lado_v, int& ladoCorte, Arc* e, list<Arc*>& ciclo, double& min_flow)
{
	int a = e->getOrigin();
	int b = e->getTarget();

	for(auto& arc : this->neighborsOf(v))
		if(arc.isOnTree() and this->getDepth(v) > this->getDepth(arc.getTarget()))
		{
			ciclo.push_back(&arc);				//o arco está no ciclo
			if(lado_v == a)						//o arco é reverso
			{
				arc.setReverse(true);
				if(arc.getFlow() < min_flow)	//o arco é candidato a sair
				{
					min_flow = arc.getFlow();
					ladoCorte = a;
				}
			}
			return arc.getTarget();
		}

	for(auto& arc_ptr : this->incidentArcsOn(v))
		if(arc_ptr->isOnTree() and this->getDepth(v) > getDepth(arc_ptr->getOrigin()))
		{
			ciclo.push_back(arc_ptr);				//o arco está no ciclo
			if(lado_v == b)							//o arco é reverso
			{
				arc_ptr->setReverse(true);			
				if(arc_ptr->getFlow() < min_flow)	//o arco é candidato a sair
				{
					min_flow = arc_ptr->getFlow();
					ladoCorte = b;
				}
			}
			return arc_ptr->getOrigin();
		}
}

void Network::fixTree(list<Arc*>& ciclo, double min_flow, int ladoCorte, bool ini)//o primeiro elemento é o arco que entrará
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

void Network::fixDepthCost(int fix_me, bool ini)//este nó está com custoParcial e depth correto
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
		
		for(it1 = this->neighborsOf(v_atual).begin(); it1 != this->neighborsOf(v_atual).end(); it1++)//olho lista de adjacencia
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
		for(it2 = this->incidentArcsOn(v_atual).begin(); !aux and it2 != this->incidentArcsOn(v_atual).end(); it2++)//olho lista de incidencia
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

Network::Network(fstream& file)
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

int Network::getSource()
{
	return this->source;
}

int Network::getSink()
{
	return this->sink;
}

int Network::getV()
{
	return this->v;
}

Arc* Network::newArc(int a, int b, int custo, double fluxo)
{
	Arc arco (a, b, custo, fluxo);
	Arc* ac;

	this->neighborsOf(a).push_back(arco);
	ac = &this->neighborsOf(a).back();
	
	this->incidentArcsOn(b).push_back(ac);

	return ac;
}

list<Arc>& Network::neighborsOf(int v)
{
	return this->adj[v];
}

list<Arc*>& Network::incidentArcsOn(int v)
{
	return this->inc[v];
}

bool Network::initialize()
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
	for(it = this->neighborsOf(fonte).begin(); it != this->neighborsOf(fonte).end(); it++)
		if((*it).isOnTree() and !(*it).isOriginal())
			return false;
	
	/*REMOVO ARCOS ARTIFICIAIS*/
	removeArtificialArcs(fonte);
	
	/*DEIXO DEPTH E CUSTOS INCIAIS PRONTOS*/
	fixDepthCost(fonte, false);

	return true;
}

void Network::executeSimplex()//o grafo já está com uma arvore factivel
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

void Network::showAnswer(ofstream& saida)//imprime o custo para chegar na pia
{
	int pia = this->getSink();
	saida << "origem destino custo fluxo" << endl;
	for(int i = 0; i < this->getV(); i++)
		for(auto& arco : this->neighborsOf(i))
			saida << arco.getOrigin() << "\t\t" << arco.getTarget() << "\t\t" << arco.getCost(false) << "\t\t" << arco.getFlow() << endl;
	
	saida << "custo total: " << this->getParcialCost(pia)*this->getDemand(this->getSink()) << endl;
}