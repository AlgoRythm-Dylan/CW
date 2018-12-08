#include "CW.h"
#include <iostream>

using namespace CW;
int main(){
	init();
	ColorPair green(RED, GREEN), red(GREEN, RED);
	Grid g;
	Widget w1, w2;
	w1.color = green;
	w2.color = red;
	g.addChild(&w1);
	g.addChild(&w2);
	GridDefinition r(1, UNIT_GRID);
	GridDefinition c1(1, UNIT_GRID), c2(1, UNIT_GRID);
	g.addRowDefinition(r);
	g.addColumnDefinition(c1);
	g.addColumnDefinition(c2);
	body.addChild(&g);
	body.inflate();
	g.inflate();
	//g.render();
	loop();
	end();
	std::cout << g.boundingBox.width << std::endl;
	return 0;
}
