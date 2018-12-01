#include "CW.h"
#include <iostream>

using namespace CW;
int main(){
	Grid g;
	GridDefinition dg1(1, '*');
	GridDefinition dg2(2, '*');
	g.addColumnDefinition(dg1);
	g.addColumnDefinition(dg2);
	init();
	//sleep(25000);
	end();
	std::cout << Grid::oddOrEven(g.columns) << std::endl;
	return 0;
}
