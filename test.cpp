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
	body->color = green;
	loop();
	end();
	return 0;
}
