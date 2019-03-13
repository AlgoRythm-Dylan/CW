#include "CW.h"
#include <iostream>

using namespace CW;

int main(){
	// Init and create some colors to use
	init();
	ColorPair green(RED, GREEN), red(GREEN, RED), yellow(BLUE, YELLOW), blue(YELLOW, BLUE);
	body->color = green;

	StackingLayoutManager *layout = new StackingLayoutManager();
	layout->orientation = Orientation::Vertical;
	body->setLayoutManager(layout);

	// Move the body widget to (1, 1)
	body->x->value = 10;
	body->y->value = 10;
	
	// Reads as "100% - 2cells (px)"
	CalculatedUnit *bodySize = new CalculatedUnit(new Unit(100, UNIT_PERCENT), '-', new Unit(20, UNIT_CELL));

	// Destroy the basic Units that the body uses for width and height, and replace them
	delete body->width;
	delete body->height;
	body->width = bodySize;
	body->height = bodySize;

	// Re-inflate the body after size changes
	body->inflate();

	// Create the clipper for the body. This will hide overflowing widgets.
	Rectangle *bodyClipper = new Rectangle();
	//body->clipShape = bodyClipper;

	// Create some simple widgets to use for testing
	// A 3x3 red widget
	Widget *w1 = new Widget();
	w1->color = red;
	w1->height->value = 3;
	w1->height->type = UNIT_CELL;
	w1->width->value = 3;
	w1->width->type = UNIT_CELL;
	
	// And a 1x1 yellow widget
	Widget *w2 = new Widget();
	w2->color = yellow;
	w2->height->value = 2;
	w2->height->type = UNIT_CELL;
	w2->width->value = 2;
	w2->width->type = UNIT_CELL;

	// Add the widgets to the body
	body->addChild(w1);
	body->addChild(w2);

	// Scroll the body down one cell
	body->scrollY = -8;
	body->scrollX = 0;

	// Start the graphics loop
	loop();
	end(); // End when "f" or "q" key is pressed
	return 0;
}
