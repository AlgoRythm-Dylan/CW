#include "CW.h"
#include <iostream>

using namespace CW;
int main(){
	Widget w;
	delete w.width;
	Unit u1 = Unit(100, '%');
	Unit u2 = Unit(10, 'C');
	w.width = new CalculatedUnit(&u1, '-', &u2);
	*w.height = 10.0;
	init();
	Color devRantLightPurple = Color(64, 65, 90);
	devRantLightPurple.activate();
	w.color = ColorPair(WHITE, devRantLightPurple);
	w.inflate();
	w.render();
	sleep(5000);
	end();
	return 0;
}
