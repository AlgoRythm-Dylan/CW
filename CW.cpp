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

}
