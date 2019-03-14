#include "CW.h"
#include <iostream>

using namespace CW;

int main(){
	// Init and create some colors to use
	init();
	ColorPair green(RED, GREEN), red(GREEN, RED), yellow(BLUE, YELLOW), blue(YELLOW, BLUE);
	ColorPair white(BLACK, WHITE), black(WHITE, BLACK);

	body->color = green;

	// Replace CW::body with a grid
	Grid* grid = new Grid();
	delete body;
	body = grid;

	GridDefinition r(1, UNIT_GRID), c1(1, UNIT_GRID), c2(20, UNIT_CELL);

	grid->layoutManager->addRowDefinition(r);
	grid->layoutManager->addColumnDefinition(c1);
	grid->layoutManager->addColumnDefinition(c2);

	Widget *canvasHolder = new Widget();
	canvasHolder->color = black;
	StackingLayoutManager *canvasHolderLayout = new StackingLayoutManager();
	canvasHolderLayout->orientation = Orientation::Horizontal;
	canvasHolderLayout->verticalAlignment = Alignment::Middle;
	canvasHolderLayout->horizontalAlignment = Alignment::Middle;
	canvasHolder->setLayoutManager(canvasHolderLayout);
	grid->layoutManager->addChild(canvasHolder, 0, 0);

	Canvas *canvas = new Canvas(100, 25);
	canvas->color = white;
	canvas->clear();
	canvasHolder->addChild(canvas);

	grid->color = white;
	grid->inflate();

	// Start the graphics loop
	loop();
	end(); // End when "f" or "q" key is pressed
	return 0;
}
