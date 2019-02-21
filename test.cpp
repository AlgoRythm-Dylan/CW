#include "CW.h"
#include <iostream>

using namespace CW;
int main(){
	init();
	/*ColorPair green(RED, GREEN), red(GREEN, RED), yellow(BLUE, YELLOW), blue(YELLOW, BLUE);
	Grid g;
	Widget w1, w2, w3, w4;
	w1.color = green;
	w2.color = red;
	w3.color = yellow;
	w4.color = blue;
	g.addChild(&w1, 0, 0);
	g.addChild(&w2, 1, 0);
	g.addChild(&w3, 0, 1);
	g.addChild(&w4, 1, 1);
	GridDefinition r1(2, UNIT_GRID), r2(1, UNIT_GRID);
	GridDefinition c1(3, UNIT_GRID), c2(1, UNIT_GRID);
	g.addRowDefinition(r1);
	g.addRowDefinition(r2);
	g.addColumnDefinition(c1);
	g.addColumnDefinition(c2);
	setBody(&g);
	loop();*/
	ColorPair textColor(BLUE, WHITE);
	Text t("This is sample text! Nothing else to it!");
	/* EXPECTED OUTPUT

		"This is "
		"sample "
		"text! "
		"Nothing "
		"else to "
		"it!"

	*/
	t.color = textColor;
	int textWidth = 10;
	int textHeight = 20;
	icoord ic(textWidth, textHeight);
	Box b(0, 1, textWidth, textHeight);
	t.parseLineBreaks(ic);
	t.render(b);
	Draw::update();
	sleep(20000);
	end();
	//std::cout << t.bakedLineBreaks.size() << std::endl;
	return 0;
}
