//Nome: Ruan de Menezes Costa
//NUSP: 7990929

#include "Network.hpp"

using namespace std;

int main(int argc, char* argv[])
{
	fstream entrada;
	ofstream saida("saida.dat");
	
	string nome_entra = "";
	nome_entra += argv[1];
	
	entrada.open(nome_entra);
	
	Network net(entrada);
	
	if(!net.initialize())
		saida << "Problema inviável" << endl;
	else
	{
		net.executeSimplex();
		net.showAnswer(saida);
	}

	entrada.close();
	saida.close();
}
