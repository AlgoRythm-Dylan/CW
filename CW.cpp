#include "CW.h"
#include <time.h>
#include <ncurses.h>
#include <stdio.h>
#include <cmath>

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
		// Setup loop
		running = 0;
		updateScreenSize();
		body = new Widget();
		//clipShapes = std::vector<Shape*>();
	}

	void loop(){
		loop(60);
	}

	void loop(int targetfps){
		fps = targetfps;
		int sleepAmount = 1000 / fps;
		running = 1;
		while(running){
			// Get input from the user
			dispatchEvents();
			// Clear the screen
			Draw::clear();
			// Render the body and it's children
			body->render();
			// Actually update the screen (Hopefully done only once per render)
			Draw::update();
			// Wait until next frame
			sleep(sleepAmount);
		}
	}

	// Simple, eh?
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
				Event e(EventType::WindowResize, screenWidth, screenHeight);
				body->handleEvent(e);
			}
			else if(ch == KEY_MOUSE){
				MEVENT me;
				if(getmouse(&me) == OK){
					MouseEvent e;
					e.x = me.x;
					e.y = me.y;
					if(me.bstate & REPORT_MOUSE_POSITION){
						e.type = EventType::MouseMove;
					}
					else if(me.bstate & BUTTON1_PRESSED ||
						me.bstate & BUTTON2_PRESSED ||
						me.bstate & BUTTON3_PRESSED ||
						me.bstate & BUTTON4_PRESSED){
						e.type = EventType::MouseDown;
					}
					else if(me.bstate & BUTTON1_RELEASED ||
						me.bstate & BUTTON2_RELEASED ||
						me.bstate & BUTTON3_RELEASED ||
						me.bstate & BUTTON4_RELEASED){
						e.type = EventType::MouseUp;
					}
					// TODO: Finish
					body->handleEvent(e);
				}
			}
			else if(ch == 'f'){
				stopLoop();
			}
			else{
				KeyEvent e(ch);
				body->handleEvent(e);
			}
			ch = getch();
		}
	}

	void end(){
		// End curses mode
		endwin();
		// Restore to standard mouse reporting
		printf("\033[?10031h\n");
		fflush(stdout);
	}

	void updateScreenSize(){
		getmaxyx(stdscr, screenHeight, screenWidth);
	}

	void setBody(Widget *newBody){
		delete body;
		body = newBody;
		body->inflate();
		body->render();
	}

	// Sleep for n milliseconds and recieve any milliseconds not slept
	long sleep(long milliseconds){
		// Quit if there's nothing to do
		if(!milliseconds){
			return 0;
		}
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

	Text::Text(){
		verticalAlignment = Alignment::Start;
		horizontalAlignment = Alignment::Start;
		text = "";
		lineBreaks = std::vector<int>();
		color = defaultColorPair;
	}

	Text::Text(std::string text){
		verticalAlignment = Alignment::Start;
		horizontalAlignment = Alignment::Start;
		this->text = text;
		lineBreaks = std::vector<int>();
		color = defaultColorPair;
	}

	void Text::parseLineBreaks(const icoord& boundaries){
		// I always have trouble with this algorithm
		lineBreaks.clear();
		int currentPosition = 0;
		int currentPositionInLine = 0;
		while(currentPosition < text.length()){
			if(text[currentPosition] == '\n'){
				// This is an explicit line break. Just do it.
				lineBreaks.push_back(currentPosition);
				currentPosition++;
				currentPositionInLine = 0;
				continue;
			}
			int nextLineBreak = getNextLineBreak(currentPosition);
			int lengthOfNextWord = nextLineBreak - currentPosition;
			if(lengthOfNextWord <= boundaries.x - currentPositionInLine){
				// It fits! Keep going.
				currentPosition += lengthOfNextWord;
				currentPositionInLine += lengthOfNextWord;
				continue;
			}
			else{
				// It doesn't fits :(
				if(lengthOfNextWord > boundaries.x){
					// It doesn't fits because its TOO BIG.
					// Split it up. This is gonna need a loop of it's own.
					// TODO
				}
				else{
					// It will fits on the next line!
					// Add a line break then get going!
					lineBreaks.push_back(currentPosition);
					currentPositionInLine = lengthOfNextWord;
					currentPosition += lengthOfNextWord;
					continue;
				}
			}
		}
		lineBreaks.push_back(text.length());
	}

	int Text::getNextLineBreak(int position){
		position++;
		while(position < text.length()){
			if(text[position] == ' ' || text[position - 1] == ' ' || text[position] == '\n'){
				return position;
			}
			position++;
		}
		return text.length();
	}

	void Text::render(const Box& area){
		int textPosition = 0;
		for(int i = 0; i < lineBreaks.size(); i++){
			int lineLength = lineBreaks[i];
			if(i > 0){
				lineLength -= lineBreaks[i - 1];
			}
			for(int j = 0; j < lineLength; j++){
				Draw::point(area.x + j, area.y + i, text[textPosition], color);
				textPosition++;
			}
		}
	}

	icoord::icoord(){
		x = 0;
		y = 0;
	}

	icoord::icoord(const icoord& othericoord){
		x = othericoord.x;
		y = othericoord.y;
	}

	icoord::icoord(int x, int y){
		this->x = x;
		this->y = y;
	}

	Box::Box(){
		x = 0;
		y = 0;
		width = 0;
		height = 0;
	}

	Box::Box(const Box &otherBox){
		x = otherBox.x;
		y = otherBox.y;
		width = otherBox.width;
		height = otherBox.height;
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
		value = 100.0;
		derivedValue = 0;
		type = UNIT_PERCENT;
	}

	Unit::Unit(double value, char type){
		this->value = value;
		this->type = type;
	}

	void Unit::derive(double max){
		derivedValue = peekDerive(max);
	}

	double Unit::peekDerive(double max){
		if(type == UNIT_CELL){
			// Cell calculation. Value is derived already
			return value;
		}
		else if(type == UNIT_PERCENT){
			// Percentage calculation. Simple, but still must be derived
			return (value / 100.0) * max;
		}
		else{
			return 0;
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

	double CalculatedUnit::peekDerive(double max){
		double a = u1->peekDerive(max);
		double b = u2->peekDerive(max);
		if(type == '-'){
			return a - b;
		}
		else if(type == '+'){
			return a + b;
		}
		else{
			return 0; // Dunno what to do, so here's a 0 for you.
		}
	}

	namespace Draw {
		
		void point(int x, int y, int character, const ColorPair &color){
			if(!clipCheck(x, y)) return;			
			attron(COLOR_PAIR(color.id));
			mvaddch(y, x, character);
		}

		// Not the most efficient algo. Use sparingly.
		/*void line(int x1, int y1, int x2, int y2, const ColorPair &color){
			int dx = x2 - x1;
			int dy = y2 - y1;
			int i = x1;
			while(i < x2){
				
				mvaddch(y1 + dy * (i - x1) / dx, i, ' ');
				i++;
			}
		} Deprecated for the time being */

		void rect(int x, int y, int width, int height, const ColorPair &color){
			if(width < 1 || height < 1){
				return;
			}
			attron(COLOR_PAIR(color.id));
			int i = 0, j = 0;
			while(i < height){
				while(j < width){
					if(!clipCheck(x + j, y + i)){
						j++;
						continue;
					}
					mvaddch(y + i, x + j, 'x');
					j++;
				}
				j = 0;
				i++;
			}
		}

		void clearRect(int x, int y, int width, int height){
			// Hooray for reusable code!
			rect(x, y, width, height, defaultColorPair);
		}

		// These may seem simple, but it keeps CW from directly interfacing with ncurses
		// This really helps with portability
		void update(){
			refresh();
		}

		void clear(){
			erase();
		}

		// Luckily the clipping system is so abstracted away that this code is reusable!
		int clipCheck(int x, int y){	
			int currentClip = 0;
			while(currentClip < clipShapes.size()){
				if(!clipShapes[currentClip]->contains(x, y)){
					return 0; // Can't draw here!
				}
				currentClip++;
			}
			return 1; // All good
		}

	}

	void Rectangle::setRect(const Box& rect){
		// Just copy the box
		this->rect = rect;
	}

	int Rectangle::contains(int x, int y){
		return ((x >= rect.x && x < rect.x + rect.width) && (y >= rect.y && y < rect.y + rect.height));
	}

	void RoundedRectangle::setRadius(double radius){
		setRadius(radius, radius, radius, radius);
	}

	void RoundedRectangle::setRadius(double topLeft, double topRight, double bottomRight, double bottomLeft){
		radiusTopLeft = topLeft;
		radiusTopRight = topRight;
		radiusBottomRight = bottomRight;
		radiusBottomLeft = bottomLeft;
	}

	int RoundedRectangle::contains(int x, int y){
		if(Rectangle::contains(x, y)){
			// Check each of the corners
			int cornerSize = (int) radiusTopLeft * 2;
			// Top right corner
			if(x < rect.x + cornerSize && y < rect.y + cornerSize){
				int midX = rect.x + radiusTopLeft + 1;
				int midY = rect.y + radiusTopLeft + 1;
				int deltaX = x - midX;
				int deltaY = midY - y;
				double angle = atan2(deltaY, deltaX);
				if(inQuadrant(2, angle) && distance(midX, x, midY, y) > radiusTopLeft){
					return 0;
				}
			}
			else if(x >= rect.width - cornerSize && y < rect.y + cornerSize){
				return 0;
			}
			else if(x < rect.x + cornerSize && y >= rect.y + rect.height - cornerSize){
				return 0;
			}
			else if(x >= rect.x + rect.width - cornerSize && y >= rect.y + rect.height - cornerSize){
				return 0;
			}
		}
		return 1;
	}

	// TODO: Accept values larger than 2pi
	int RoundedRectangle::quadrant(double radians){
		if(radians >= 0 && radians <= M_PI_2){
			return 1;
		}
		else if(radians > M_PI_2 && radians <= M_PI){
			return 2;
		}
		else if(radians > M_PI && radians <= M_3PI_2){
			return 3;
		}
		else{
			return 4;
		}
	}

	int RoundedRectangle::inQuadrant(int which, double angle){
		double angleMax = M_PI_2 * which;
		double angleMin = M_PI_2 * (which - 1);
		return angle >= angleMin && angle <= angleMax;
	}
	
	int RoundedRectangle::distance(int x1, int x2, int y1, int y2){
		return (int) sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
	}

	Widget::Widget(){
		parent = nullptr;
		x = new Unit(0, UNIT_CELL);
		y = new Unit(0, UNIT_CELL);
		width = new Unit();
		height = new Unit();
		inflate(); // Inflate the widget according to the size of stdscr
	}

	void Widget::inflate(){
		// Derive all values, split text into lines, size buffers, etc
		if(parent){
			x->derive(parent->width->derivedValue);
			y->derive(parent->height->derivedValue);
			width->derive(parent->width->derivedValue);
			height->derive(parent->height->derivedValue);
		}
		else{
			x->derive(screenWidth);
			y->derive(screenHeight);
			width->derive(screenWidth);
			height->derive(screenHeight);
		}
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
		clip();
		Draw::rect(box.x, box.y, box.width, box.height, color);
		boundingBox = box; // Update current bouding box, for children to render into
		int i = 0;
		while(i < children.size()){
			children[i]->render();
			i++;
		}
		unclip();
	}

	void Widget::addChild(Widget *widget){
		// Add to the child list, so we can send events to this widget and render it
		children.push_back(widget);
		// Tell the new widget who it's new parent is and have it size itself correctly
		widget->parent = this;
		widget->inflate();
	}

	void Widget::handleEvent(Event &e){
		if(e.type == EventType::WindowResize){
			if(!parent){
				// If this is the body widget, there will be no parent,
				// and this widget will need to react to the window size
				inflate();
			}
			int i = 0;
			while(i < children.size() && !e.stopped){
				children[i]->handleEvent(e);
				i++;
			}
		}	
	}

	// Used for event "collisions" - does a point fall within this widget
	int Widget::contains(int x, int y){
		if(x >= boundingBox.x && x <= boundingBox.width){
			if(y >= boundingBox.y && y <= boundingBox.height){
				return 1; // Coords are contained in this widget
			}
		}
		return 0; // Coords lay outside this widget
	}

	icoord Widget::peekSize(const icoord& availableSpace){
		return icoord(0, 0); // For now
	}
	
	void Widget::clip(){
		usedClip = 0;
		if(clipShape){
			usedClip = 1;
			clipShape->setRect(boundingBox);
			clipShapes.push_back(clipShape);
		}
	}

	void Widget::unclip(){
		if(usedClip) clipShapes.pop_back();
	}

	GridChild::GridChild(){
		child = nullptr;
		column = -1;
		row = -1;
	}

	GridChild::GridChild(Widget *child, int column, int row){
		this->child = child;
		this->column = column;
		this->row = row;
	}

	GridDefinition::GridDefinition(){
		value = 0;
		type = UNIT_CELL;
		inflatedValue = 0;
	}

	GridDefinition::GridDefinition(double value, char type){
		this->value = value;
		this->type = type;
		inflatedValue = 0;
	}

	void Grid::render(){
		render(boundingBox);
	}

	void Grid::render(const Box &box){
		clip();
		int startX = box.x, startY = box.y, xOffset = 0, yOffset = 0, column = 0, row = 0;
		// Render columns from left to right
		while(column < columns.size()){
			// Do each row within each column
			yOffset = 0;
			row = 0;
			while(row < rows.size()){
				Widget *child = getChildAt(column, row);
				if(child){
					// If this cell isn't empty, render it!
					Box box(
						startX + xOffset,
						startY + yOffset,
						columns[column].inflatedValue,
						rows[row].inflatedValue
					);
					child->render(box);
				}
				yOffset += rows[row].inflatedValue;
				row++;
			}
			xOffset += columns[column].inflatedValue;
			column++;
		}
		unclip();
	}

	void Grid::addColumnDefinition(GridDefinition &gd){
		columns.push_back(gd);
	}

	void Grid::addRowDefinition(GridDefinition &gd){
		rows.push_back(gd);
	}

	void Grid::handleEvent(Event &e){
		if(e.type == EventType::WindowResize){
			// Just react to resize if this widget is the body
			if(!parent){
				inflate();
			}
		}
	}

	void Grid::inflateDefinitions(std::vector<GridDefinition>& gridTemplate, int availableSpace){
		// Add up all of the definitions that are of type grid
		double totalGridWeight = 0.0;
		int i = 0;
		while(i < gridTemplate.size()){
			if(gridTemplate[i].type == UNIT_GRID){
				totalGridWeight += gridTemplate[i].value;
			}
			else{
				gridTemplate[i].inflatedValue = gridTemplate[i].value;
				availableSpace -= gridTemplate[i].value;
			}
			i++;
		}
		int totalSpace = availableSpace;
		i = 0;
		while(i < gridTemplate.size()){
			if(gridTemplate[i].type == UNIT_GRID){
				double weight = gridTemplate[i].value / totalGridWeight;
				int value = (int) (totalSpace * weight);
				gridTemplate[i].inflatedValue = value;
				availableSpace -= value;
			}
			i++;
		}
		// Lastly, distribute any remaining cells to grid types
		while(availableSpace){
			i = 0;
			while(i < gridTemplate.size() && availableSpace){
				if(gridTemplate[i].type == UNIT_GRID){
					gridTemplate[i].inflatedValue++;
					availableSpace--;
				}
				i++;
			}
		}
	}

	void Grid::inflate(){
		Widget::inflate();
		inflateDefinitions(columns, boundingBox.width);
		inflateDefinitions(rows, boundingBox.height);
	};

	void Grid::addChild(Widget *widget){
		// Add to 0, 0 by default. This is only here for consistency
		setChildAt(widget, 0, 0);
	};

	void Grid::addChild(Widget *widget, int column, int row){
		setChildAt(widget, column, row);
	}

	Widget* Grid::getChildAt(int column, int row){
		int i = 0;
		while(i < childPositions.size()){
			if(childPositions[i].column == column && childPositions[i].row == row){
				return childPositions[i].child;
			}
			i++;
		}
		return NULL;
	}

	// Basically the heart of adding widgets to a grid. Sets a child at
	// row/ column. Returns old widget if there was one.
	Widget* Grid::setChildAt(Widget *widget, int column, int row){
		Widget *oldWidget = getChildAt(column, row);
		if(oldWidget){
			// Find it and replace it
			int i  = 0;
			while(i < childPositions.size()){
				if(childPositions[i].column == column && childPositions[i].row == row){
					// This is it, cheif
					childPositions[i].child = widget;
					return oldWidget;
				}
				i++;
			}
		}
		else{
			// Just add a new one. Simple as that
			childPositions.push_back(GridChild(widget, column, row));
		}
		return oldWidget;
	}

	Event::Event(){
		type = EventType::Invalid;
		x = -1;
		y = -1;
		stopped = 0;
	}

	Event::Event(EventType type){
		this->type = type;
		x = -1;
		y = -1;
		stopped = 0;
	}

	Event::Event(EventType type, int x, int y){
		this->type = type;
		this->x = x;
		this->y = y;
		stopped = 0;
	}

	void Event::stop(){
		stopped = 1;
	}

	MouseEvent::MouseEvent(){
		type = EventType::Mouse;
		count = 0;
		x = -1;
		y = -1;
		stopped = 0;
	}

	MouseEvent::MouseEvent(EventType type){
		this->type = type;
		count = 0;
		x = -1;
		y = -1;
		stopped = 0;
	}

	MouseEvent::MouseEvent(EventType type, int x, int y){
		this->type = type;
		count = 0;
		this->x = x;
		this->y = y;
		stopped = 0;
	}

	MouseEvent::MouseEvent(EventType type, int x, int y, int count){
		this->type = type;
		this->count = count;
		this->x = x;
		this->y = y;
		stopped = 0;
	}

	KeyEvent::KeyEvent(){
		this->type = EventType::Key;
		stopped = 0;
	}

	KeyEvent::KeyEvent(int key){
		this->type = EventType::Key;
		this->key = key;
		stopped = 0;
	}

	// Externs
	std::vector<int> usedColors;
	std::vector<int> usedPairs;
	ColorPair defaultColorPair;
	Color BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE;
	int fps, screenWidth, screenHeight, running;
	Widget *body;
	std::vector<Shape*> clipShapes;

}
