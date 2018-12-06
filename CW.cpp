#include "CW.h"
#include <time.h>
#include <ncurses.h>
#include <stdio.h>

namespace CW {

	void init(){
		// Interface with ncurses
		initscr();
		start_color();
		use_default_colors();
		keypad(stdscr, 1);
		cbreak();
		noecho();
		nodelay(stdscr, 1);
		mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
		// This will need to be played with and will be different from
		// emulator to emulator and system to system
		mouseinterval(75);
		// Some raw stuff
		printf("\033[?1003h\n");
		fflush(stdout);
		// Set up the ASCII colors
		BLACK = Color(0);
		RED = Color(1);
		GREEN = Color(2);
		YELLOW = Color(3);
		BLUE = Color(4);
		MAGENTA = Color(5);
		CYAN = Color(6);
		WHITE = Color(7);
		// Make the cursor invisible initially
		curs_set(0);
		// Set up the body
		body.width->value = 100.0;
		body.width->type = UNIT_PERCENT;
		body.height->value = 100.0;
		body.height->type = UNIT_PERCENT;
		// Setup loop
		running = 0;
	}

	void loop(){
		loop(30);
	}

	void loop(int targetfps){
		fps = targetfps;
		int sleepAmount = 1000 / fps;
		running = 1;
		while(running){
			dispatchEvents();
			body.render();
			sleep(sleepAmount);
		}
	}

	void stopLoop(){
		running = false;
	}

	void dispatchEvents(){
		int ch = getch();
		while(ch != ERR){
			if(ch == KEY_RESIZE){
				// Generate and dispatch a resize event
				// Can only get screen size at current time afaik
				updateScreenSize();
				ResizeEvent e(screenWidth, screenHeight);
				body.handleEvent(e);
			}
			else if(ch == KEY_MOUSE){
				MEVENT me;
				if(getmouse(&me) == OK){
					MouseEvent e;
					e.x = me.x;
					e.y = me.y;
					if(me.bstate & REPORT_MOUSE_MOVEMENT){
						e.type = EVENT_MOUSE_MOVE;
					}
					else if(me.bstate & BUTTON1_PRESSED ||
						me.bstate & BUTTON2_PRESSED ||
						me.bstate & BUTTON3_PRESSED ||
						me.bstate & BUTTON4_PRESSED){
						e.type = EVENT_MOUSE_DOWN;
					}
					else if(me.bstate & BUTTON1_RELEASED ||
						me.bstate & BUTTON2_RELEASED ||
						me.bstate & BUTTON3_RELEASED ||
						me.bstate & BUTTON4_RELEASED){
						e.type = EVENT_MOUSE_UP;
					}
					// TODO: Finish
					body.handleMouseEvent(e);
				}
			}
			else if(ch == 'f'){
				stopLoop();
			}
			else{
				KeyEvent e(ch);
				body.handleKeyEvent(e);
			}
			ch = getch();
		}
	}

	void end(){
		printf("\033[?1000h");
		fflush(stdout);
		endwin();
	}

	int updateScreenSize(){
		getmaxyx(stdscr, screenHeight, screenWidth);
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
	
	Color::Color(){
		id = -1; // Default color id
	}

	Color::Color(int id){
		this->id = id;
	}

	Color::Color(short r, short g, short b){
		id = -2; // Invalid color id (Flag to assign one)
		this->r = r;
		this->g = g;
		this->b = b;
	}

	void Color::activate(){
		if(id == -2){
			id = nextid();
		}
		init_color(id, 
			cv255to1000(r),
			cv255to1000(g),
			cv255to1000(b)
		);
	}

	short Color::cv255to1000(short num){
		return (short)(1000.0 * ((double) num / 255.0));
	}

	short Color::cv1000to255(short num){
		return (short)(255.0 * ((double) num / 1000.0 ));
	}

	ColorPair::ColorPair(){
		id = 0; // Default color pair id
	}

	ColorPair::ColorPair(Color& foreground, Color& background){
		id = -1; // Invalid color pair id (Flag to assign one)
		this->foreground = &foreground;
		this->background = &background;
		activate();
	}

	int Color::nextid(){
		int i = 8; // Start after the 8 ASCII colors
		while(i < COLORS){
			int j = 0;
			while(j < usedColors.size()){
				if(usedColors[j] == i){
					i++;
					continue;
				}
				j++;
			}
			usedColors.push_back(i);
			return i;
		}
		return -1;
	}

	void ColorPair::activate(){
		if(id == -1){
			id = nextid();
		}
		init_pair(id, foreground->id, background->id);
	}

	int ColorPair::nextid(){
		int i = 1; // Start at 1, which is the first available color pair
		// Find the smallest avaialable number, reserve it, and return it
		while(i < COLOR_PAIRS){
			int j = 0;
			while(j < usedPairs.size()){
				if(usedPairs[j] == i){
					i++;
					continue; // The pair is in the list, and thus, is in use
				}
				j++;
			}
			usedPairs.push_back(i); // Reserve this color pair
			return i; // The pair was not in the list, and is available
		}
		return -1; // No more color pairs available :(
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

	void Box::values(int x, int y, int width, int height){
		this->x = x;
		this->y = y;
		this->width = width;
		this->height = height;
	}

	Unit::Unit(){
		value = 0;
		derivedValue = 0;
		type = UNIT_CELL;
	}

	Unit::Unit(double value, char type){
		this->value = value;
		this->type = type;
	}

	void Unit::derive(double max){
		if(type == UNIT_CELL){
			// Cell calculation. Value is derived already
			derivedValue = value;
		}
		else if(type == UNIT_PERCENT){
			// Percentage calculation. Simple, but still must be derived
			derivedValue = (value / 100.0) * max;
		}	
	}

	void Unit::operator=(double value){
		this->value = value;
	}

	CalculatedUnit::CalculatedUnit(){
		u1 = new Unit();
		u2 = new Unit();
		type = '-';
	}

	CalculatedUnit::CalculatedUnit(Unit *u1, char type, Unit *u2){
		this->u1 = u1;
		this->type = type;
		this->u2 = u2;
	}

	void CalculatedUnit::derive(double max){
		u1->derive(max);
		u2->derive(max);
		if(type == '-'){
			derivedValue = u1->derivedValue - u2->derivedValue;
		}
		else if(type == '+'){
			derivedValue = u1->derivedValue + u2->derivedValue;
		}
		else{
			derivedValue = 0; // Dunno what to do, so here's a 0 for you.
		}
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
				mvaddch(y1 + dy * (i - x1) / dx, i, ' ');
				i++;
			}
		}

		void rect(int x, int y, int width, int height, ColorPair &color){
			attron(COLOR_PAIR(color.id));
			int i = 0, j = 0;
			while(i < height){
				while(j < width){
					mvaddch(y + i, x + j, ' ');
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
		x = new Unit();
		y = new Unit();
		width = new Unit();
		height = new Unit();
		inflate(); // Inflate the widget according to the size of stdscr
	}

	void Widget::inflate(){
		// Derive all values, split text into lines, size buffers, etc
		x->derive(screenWidth);
		y->derive(screenHeight);
		width->derive(screenWidth);
		height->derive(screenHeight);
		// Update the bounding box
		boundingBox.values(
				(int) x->derivedValue,
				(int) y->derivedValue,
				(int) width->derivedValue,
			       	(int) height->derivedValue
		);
	}

	void Widget::render(){
		// For self-placement
		render(boundingBox);
	}

	void Widget::render(const Box &box){
		// Second step of self-placement, but more importantly,
		// allows parent widgets (such as grids) to control layout and size
		Draw::rect(box.x, box.y, box.width, box.height, color);
		Draw::update();
	}

	void Widget::addChild(Widget *widget){
		children.push_back(widget);
	}

	GridDefinition::GridDefinition(){
		value = 0;
		type = UNIT_CELL;
	}

	GridDefinition::GridDefinition(double value, char type){
		this->value = value;
		this->type = type;
	}

	void Grid::render(){
		render(boundingBox);
	}

	void Grid::render(const Box &box){

	};

	void Grid::addColumnDefinition(GridDefinition &gd){
		columns.push_back(gd);
	}

	void Grid::addRowDefinition(GridDefinition &gd){
		rows.push_back(gd);
	}

	int Grid::oddOrEven(std::vector<GridDefinition>& gridTemplate){
		if(gridTemplate.size() <= 1){
			// There is either nothing or one thing to render, so we do not need
			// a specific number of cells to render this grid
			return GRID_EITHER;
		}
		// Another condition for GRID_EITHER is if all of the GridDefinitions are of
		// type cell. In other words, the grid is fixed size. This is common for row heights
		int i = 0, allFixedSize = 1;
		while(i < gridTemplate.size()){
			if(gridTemplate[i].type == UNIT_GRID){
				allFixedSize = 0;
				break;
			}
			i++;
		}
		if(allFixedSize){
			return GRID_EITHER;
		}
		// What we know at this point:
		// * More than one item to render
		// * Not all items are fixed size
		// Now comes the difficult part: determine odd or even!
		i = 0;
		double fixedDimension = 0.0, gridDimension = 0.0;
		while(i < gridTemplate.size()){
			if(gridTemplate[i].type == UNIT_CELL){
				fixedDimension += gridTemplate[i].value;
			}
			else if(gridTemplate[i].type == UNIT_GRID){
				gridDimension += gridTemplate[i].value;
			}
			i++;
		}
		int gridOdd = 0, fixedOdd = 0;
		if((int) gridDimension % 2 != 0){
			gridOdd = 1;
		}
		if((int) fixedDimension % 2 != 0){
			fixedOdd = 1;
		}
		if(fixedOdd || gridOdd){
			return GRID_ODD;
		}
		else{
			return GRID_EVEN;
		}
	}

	void Grid::inflate(){};
	void Grid::addChild(Widget *widget){};

	Event::Event(){
		type = -1; // Invalid type
		x = -1;
		y = -1;
	}

	Event(int type){
		this->type = type;
		x = -1;
		y = -1;
	}

	Event(int type, int x, int y){
		this->type = type;
		this->x = x;
		this->y = y;
	}

	MouseEvent::MouseEvent(){
		type = EVENT_MOUSE;
		count = 0;
		x = -1;
		y = -1;
	}

	MouseEvent::MouseEvent(int type){
		this->type = type;
		count = 0;
		x = -1;
		y = -1;
	}

	MouseEvent::MouseEvent(int type, int x, int y){
		this->type = type;
		count = 0;
		this->x = x;
		this->y = y;
	}

	MouseEvent::MouseEvent(int type, int x, int y, int count){
		this->type = type;
		this->count = count;
		this->x = x;
		this->y = y;
	}

	KeyEvent::KeyEvent(){
		this->type = EVENT_KEY;
	}

	KeyEvent::KeyEvent(int key){
		this->type = EVENT_KEY;
		this->key = key;
	}

	// Externs
	std::vector<int> usedColors;
	std::vector<int> usedPairs;
	ColorPair defaultColorPair;
	Color BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE;
	int fps, screenWidth, screenHeight, running;
	Widget body;

}
