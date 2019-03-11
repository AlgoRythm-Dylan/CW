#include "CW.h"
#include <iostream>
#include <cmath>

using namespace CW;

// To test if the clipping thing works
struct EndlessVoid : Shape {
	int contains(int x, int y){
		return 0;
	}
	void setRect(const Box&){
		// Nope
	}
};

int main(){
	init();
	ColorPair green(RED, GREEN), red(GREEN, RED), yellow(BLUE, YELLOW), blue(YELLOW, BLUE);
	Grid g;
	Widget w1, w2, w3, w4;
	w1.color = green;
	w2.color = red;
	w3.color = yellow;
	w4.color = blue;
	EndlessVoid *testClipper = new EndlessVoid();
	RoundedRectangle *otherTestClipper = new RoundedRectangle();
	otherTestClipper->setRadius(3);
	w4.clipShape = testClipper;
	w1.clipShape = otherTestClipper;
	g.addChild(&w1, 0, 0);
	g.addChild(&w2, 1, 0);
	//g.addChild(&w3, 0, 1);
	g.addChild(&w4, 1, 1);
	GridDefinition r1(2, UNIT_GRID), r2(1, UNIT_GRID);
	GridDefinition c1(3, UNIT_GRID), c2(1, UNIT_GRID);
	g.addRowDefinition(r1);
	g.addRowDefinition(r2);
	g.addColumnDefinition(c1);
	g.addColumnDefinition(c2);
	setBody(&g);
	loop();
	end();
	// Some math test stuff
	//icoord pointOne(3, 3);
	//icoord pointTwo(2, 2);
	/*

		. . .
 		. 2 .
		. . 1

	*/
	// What is the angle between these two?
	// Expected output: 135*, or 3pi/4 (2.35619)
	//double deltaX = pointTwo.x - pointOne.x;
	//double deltaY = pointOne.y - pointTwo.y;
	//std::cout << atan2(deltaY, deltaX) << std::endl;
	// This will be used to *yawn* make a rounded rectangle clip area 
	return 0;
}
