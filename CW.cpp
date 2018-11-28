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

	void Unit::operator=(double value){
		this->value = value;
	}

	namespace Draw {
		
		void point(int x, int y, int character, ColorPair &color){
			mvaddch(y, x, character);
		}

		void rect(int x, int y, int width, int height, ColorPair &color){
			int i = 0, j = 0;
			while(i < height){
				while(j < width){
					mvaddch(i, j, 'X');
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

}
