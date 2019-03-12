#include "CW.h"
#include <iostream>
#include <cmath>

using namespace CW;

int main(){
	init();
	ColorPair green(RED, GREEN), red(GREEN, RED), yellow(BLUE, YELLOW), blue(YELLOW, BLUE);
	body->color = green;

	StackingLayoutManager *layout = new StackingLayoutManager();
	layout->orientation = Orientation::Vertical;
	body->setLayoutManager(layout);

	Widget *w1 = new Widget();
	w1->color = red;
	w1->height->value = 3;
	w1->height->type = UNIT_CELL;
	w1->width->value = 3;
	w1->width->type = UNIT_CELL;

	
	Widget *w2 = new Widget();
	w2->color = yellow;
	w2->height->value = 1;
	w2->height->type = UNIT_CELL;
	w2->width->value = 1;
	w2->width->type = UNIT_CELL;

	body->addChild(w1);
	body->addChild(w2);

	loop();
	end();
	return 0;
}
