#include "CW.h"
#include <iostream>

using namespace CW;
int main(){
	Unit u(5.0, 'C');
	u = 6.0;
	std::cout << u.value << std::endl;
	sleep(1000);
	init();
	sleep(5000);
	end();
	return 0;
}
