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
	grid->addChild(canvasHolder, 0, 0);

	Canvas *canvas = new Canvas(80, 25);
	canvas->color = white;
	canvas->clear();
	canvasHolder->addChild(canvas);

	MouseEventListener *canvasListener = new MouseEventListener();
	canvasListener->handle = [&white, canvas](MouseEvent *event){
		CharInfo info;
		info.character = 'x';
		info.attributes = white.toAttribute();
		icoord relativeCoords = event->relativeTo(canvas);
		canvas->point(relativeCoords.x, relativeCoords.y, info);
	};
	canvas->mouseEventListeners.push_back(canvasListener);

	Button *clearButton = new Button();
	//grid->addChild(clearButton, 1, 0);
	clearButton->height->value = 3;
	clearButton->height->type = UNIT_CELL;
	clearButton->color = blue;

	Widget *controlsHolder = new Widget();
	StackingLayoutManager *controlsLayout = new StackingLayoutManager;
	controlsHolder->setLayoutManager(controlsLayout);
	controlsHolder->color = yellow;
	grid->addChild(controlsHolder, 1, 0);
	controlsHolder->addChild(clearButton);

	grid->color = white;
	grid->inflate();

	// Start the graphics loop
	loop();
	end(); // End when "f" or "q" key is pressed

	return 0;
}
