#include "CW.h"

using namespace CW;

int main(){

	init();
	ColorPair green (GREEN, GREEN);
	body->color = green;
	body->width->value = 50.0;
	body->inflate();

	ColorPair purple (MAGENTA, MAGENTA);
	Widget w;
	w.color = purple;
	w.height->value = 50.0;
	body->addChild(&w);

	KeyEventListenerPtr keyListener = std::make_shared<KeyEventListener>();
	keyListener->handle = [](KeyEvent &event){
		if(event.key == 'a') stopLoop();
	};

	body->onKeyEvent(keyListener);

	loop();
	end();	

}
