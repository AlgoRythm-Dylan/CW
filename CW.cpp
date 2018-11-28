#include "CW.h"
#include <time.h>
#include <ncurses.h>

namespace CW {

	void init(){
		initscr();
	}

	void end(){
		endwin();
	}

	int screenWidth(){
		int x, y;
		getmaxyx(stdscr, y, x); // Fucking unused variables. Make a coord struct for fucks sake
		return x;
	}

	int screenHeight(){
		int x, y;
		getmaxyx(stdscr, y, x);
		return y;
	}

	// Sleep for n milliseconds and recieve any milliseconds not slept
	long sleep(long milliseconds){
		timespec t1, t2;
		long nanoseconds, seconds;
		seconds = milliseconds / 1000;
		nanoseconds = (milliseconds - (seconds * 1000)) * 1000000;
		t1.tv_sec = seconds;
		t1.tv_nsec = nanoseconds;
		nanosleep(&t1, &t2);
		return (t2.tv_sec * 1000) + (t2.tv_nsec / 1000000);
	}
	
	Box::Box(){
		x = 0;
		y = 0;
		width = 0;
		height = 0;
	}

	Box::Box(int x, int y, int width, int height){
		this->x = x;
		this->y = y;
		this->width = width;
		this->height = height;
	}

	Unit::Unit(){
		value = 0;
		type = 'C';
	}

	Unit::Unit(double value, char type){
		this->value = value;
		this->type = type;
	}

	void Unit::derive(double max){
		if(type == 'C'){
			// Cell calculation. Value is derived already
			derivedValue = value;
		}
		else if(type == '%'){
			// Percentage calculation. Simple, but still must be derived
			derivedValue = (value / 100.0) * max;
		}	
	}

	void Unit::operator=(double value){
		this->value = value;
	}

	namespace Draw {
		
		void point(int x, int y, int character, ColorPair &color){
			mvaddch(y, x, character);
		}

		void line(int x1, int y1, int x2, int y2, ColorPair &color){
			int dx = x2 - x1;
			int dy = y2 - y1;
			int i = x1;
			while(i < x2){
				mvaddch(y1 + dy * (i - x1) / dx, i, 'X');
				i++;
			}
		}

		void rect(int x, int y, int width, int height, ColorPair &color){
			int i = 0, j = 0;
			while(i < height){
				while(j < width){
					mvaddch(y + i, x + j, 'X');
					j++;
				}
				j = 0;
				i++;
			}
		}

		void update(){
			refresh();
		}

	}

	Widget::Widget(){
		inflate(); // Inflate the widget according to the size of stdscr
	}

	void Widget::inflate(){
		// Derive all values, split text into lines, size buffers, etc
		x.derive(screenWidth());
		y.derive(screenHeight());
		width.derive(screenWidth());
		height.derive(screenHeight());
	}

	void Widget::render(){
		// For self-placement
	}

	void Widget::render(const Box &box){
		// Second step of self-placement, but more importantly,
		// allows parent widgets (such as grids) to control layout and size
	}

}
