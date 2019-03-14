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
			else if(ch == 'f' || ch == 'q'){
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

	short ColorPair::toAttribute(){
		return COLOR_PAIR(id);
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
		int startX = 0;
		int startY = 0;
		for(int i = 0; i < lineBreaks.size(); i++){
			int lineLength = lineBreaks[i];
			if(i > 0){
				lineLength -= lineBreaks[i - 1];
			}
			// Deal with the alignment
			startX = 0;
			if(horizontalAlignment == Alignment::Middle){
				startX = (area.width - lineLength) / 2;
			}
			else if(horizontalAlignment == Alignment::End){
				startX = area.width - lineLength;
			}
			if(lineBreaks.size() < area.height){
				startY = 0;
				if(verticalAlignment == Alignment::Middle){
					startY = (area.height - lineBreaks.size()) / 2;
				}
				else if(verticalAlignment == Alignment::End){
					startY = area.height - lineBreaks.size();
				}
			}
			for(int j = 0; j < lineLength; j++){
				Draw::point(area.x + j + startX, area.y + i + startY, text[textPosition], color);
				textPosition++;
			}
		}
	}

	CharInfo::CharInfo(){
		attributes = 0;
		character = ' ';
	}

	CharInfo::CharInfo(const CharInfo& otherCharInfo){
		attributes = otherCharInfo.attributes;
		character = otherCharInfo.character;
	}

	void CharInfo::put(int x, int y, const CharInfo& charInfo){
		attron(charInfo.attributes);
		mvaddch(y, x, charInfo.character);
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
		value = 0;
		derivedValue = 0;
		type = UNIT_CELL;
	}

	Unit::Unit(double value, char type){
		this->value = value;
		this->type = type;
	}

	double Unit::derive(double max){
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

	double CalculatedUnit::derive(double max){
		double a = u1->derive(max);
		double b = u2->derive(max);
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
					mvaddch(y + i, x + j, ' ');
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

	void AbsoluteLayoutManager::render(){
		if(!widget) return;
		// Just let them render themselves, good luck!
		for(int i = 0; i < widget->children.size(); i++){
			widget->children[i]->render();
		}
	}

	void StackingLayoutManager::render(){
		if(!widget) return; // Nothing to render to
		if(widget->children.size() == 0) return; // Nothing to render
		// Calculate the width and height of all children
		int totalLayoutWidth = 0;
		int totalLayoutHeight = 0;
		int startX = 0;
		int startY = 0;
		if(orientation == Orientation::Vertical){
			// Stack vertically
			// Sum the heights, find the max of the widths
			totalLayoutWidth = widget->children[0]->boundingBox.width;
			for(int i = 0; i < widget->children.size(); i++){
				if(widget->children[i]->boundingBox.width > totalLayoutWidth){
					totalLayoutWidth = widget->children[i]->boundingBox.width;
				}
				totalLayoutHeight += widget->children[i]->boundingBox.height;
			}
			// According to the alignment, add startY
			if(verticalAlignment == Alignment::Middle &&
			   totalLayoutHeight < widget->boundingBox.height){
				startY = (widget->boundingBox.height - totalLayoutHeight) / 2;
			}
			else if(verticalAlignment == Alignment::End &&
				totalLayoutHeight < widget->boundingBox.height){
				startY = widget->boundingBox.height - totalLayoutHeight;
			}
			int currentY = 0;
			for(int i = 0; i < widget->children.size(); i++){
				Widget *currentWidget = widget->children[i];
				startX = 0;
				if(horizontalAlignment == Alignment::Middle){
					startX = (widget->boundingBox.width - currentWidget->boundingBox.width) / 2;
				}
				else if(horizontalAlignment == Alignment::End){
					startX = widget->boundingBox.width - currentWidget->boundingBox.width;
				}
				currentWidget->render(Box(
					widget->boundingBox.x - currentWidget->scrollX + startX,
					widget->boundingBox.y + currentY - currentWidget->scrollY + startY,
					currentWidget->boundingBox.width,
					currentWidget->boundingBox.height
				));
				currentY += currentWidget->boundingBox.height;
			}
		}
		else if(orientation == Orientation::Horizontal){
			// Stack sideways
			int currentX = 0;
			for(int i = 0; i < widget->children.size(); i++){
				Widget *currentWidget = widget->children[i];
				currentWidget->render(Box(
					widget->boundingBox.x + currentX - currentWidget->scrollX,
					widget->boundingBox.y - currentWidget->scrollY,
					currentWidget->boundingBox.width,
					currentWidget->boundingBox.height
				));
				currentX += currentWidget->boundingBox.width;
			}
		}
	}

	// GRID LAYOUT MANAGER

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

	void GridLayoutManager::addColumnDefinition(GridDefinition &gd){
		columns.push_back(gd);
	}

	void GridLayoutManager::addRowDefinition(GridDefinition &gd){
		rows.push_back(gd);
	}

	void GridLayoutManager::inflateDefinitions(std::vector<GridDefinition>& gridTemplate, int availableSpace){
		// Add up all of the definitions that are of type grid
		if(gridTemplate.size() == 0) return;
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

	void GridLayoutManager::addChild(Widget *widget){
		// Add to 0, 0 by default. This is only here for consistency
		setChildAt(widget, 0, 0);
	};

	void GridLayoutManager::addChild(Widget *widget, int column, int row){
		setChildAt(widget, column, row);
	}

	Widget* GridLayoutManager::getChildAt(int column, int row){
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
	Widget* GridLayoutManager::setChildAt(Widget *widget, int column, int row){
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

	void GridLayoutManager::render(){
		int startX = widget->boundingBox.x;
		int startY = widget->boundingBox.y;
		int xOffset = 0, yOffset = 0;
		int column = 0, row = 0;
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
	}

	Widget::Widget(){
		parent = nullptr;
		x = new Unit();
		y = new Unit();
		width = new Unit(100, UNIT_PERCENT);
		height = new Unit(100, UNIT_PERCENT);
		layoutManager = new AbsoluteLayoutManager();
		layoutManager->widget = this;
		inflate(); // Inflate the widget according to the size of stdscr
	}

	void Widget::inflate(){
		// Derive all values, split text into lines, size buffers, etc
		if(parent){
			boundingBox.x = (int) x->derive(parent->boundingBox.width);
			boundingBox.y = (int) y->derive(parent->boundingBox.height);
			boundingBox.width = (int) width->derive(parent->boundingBox.width);
			boundingBox.height = (int) height->derive(parent->boundingBox.height);
		}
		else{
			boundingBox.x = (int) x->derive(screenWidth);
			boundingBox.y = (int) y->derive(screenHeight);
			boundingBox.width = (int) width->derive(screenWidth);
			boundingBox.height = (int) height->derive(screenHeight);
		}
	}

	int Widget::render(){
		// For self-placement
		return render(boundingBox);
	}

	int Widget::render(const Box &box){
		// Second step of self-placement, but more importantly,
		// allows parent widgets (such as grids) to control layout and size
		boundingBox = box; // Update current bouding box, for children to render into
		if(parent){
			boundingBox.x -= parent->scrollX;
			boundingBox.y -= parent->scrollY;
		}
		if(!shouldRender()) return 0;
		clip();
		Draw::rect(boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height, color);
		layoutManager->render();
		unclip();
		return 1;
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

	void Widget::setLayoutManager(LayoutManager *newManager){
		delete layoutManager;
		layoutManager = newManager;
		layoutManager->widget = this;
	}

	int Widget::shouldRender(){
		// First, see if the culling algo even needs to take place
		if(parent){
			if(!parent->cullChildren || disableCulling){
				return 1;
			}
		}
		if(disableCulling){
			return 1;
		}
		/* Should the widget even render? Yes if:
		 - Widget is at least partially within the screen AND
		 - Widget is at least partially within its parent
		
		Widget:

		A ----------------------------- B
		|                               |
		|                               |
		|                               |
		|                               |
		|                               |
		C ----------------------------- D

		First, check and make sure it is with the screen.
		
		It is NOT within the screen if:
		
		- Dy is less than 0
		- Ay is greater than screen height
		- Dx is less than 0
		- Ax is greater than screen width

		Then check for the same parameters, using the parent bounding box

		Okay, now that it's in english, let's do it in c++ */

		int Dy = boundingBox.y + boundingBox.height;
		int Ay = boundingBox.y;
		int Dx = boundingBox.x + boundingBox.width;
		int Ax = boundingBox.x;
		if(Dy <= 0 || Dx <= 0){
			return 0;
		}
		else if(Ay >= screenHeight || Ax >= screenWidth){
			return 0;
		}
		// Okay, so it fits on the SCREEN, but what about within the parent widget?
		if(parent){
			if(Dy <= parent->boundingBox.y || Dx <= parent->boundingBox.x){
				return 0;
			}
			else if(Ay >= parent->boundingBox.y + parent->boundingBox.height ||
				Ax >= parent->boundingBox.x + parent->boundingBox.width){
				return 0;
			}
		}
		return 1;
	}

	Grid::Grid(){
		layoutManager = new GridLayoutManager();
		layoutManager->widget = this;
	}

	int Grid::render(){
		return render(boundingBox);
	}

	int Grid::render(const Box &box){
		boundingBox = box;
		if(parent){
			boundingBox.x -= parent->scrollX;
			boundingBox.y -= parent->scrollY;
		}
		if(!shouldRender()) return 0;
		clip();
		Draw::rect(boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height, color);
		layoutManager->render();
		unclip();
		return 1;
	}

	void Grid::handleEvent(Event &e){
		if(e.type == EventType::WindowResize){
			// Just react to resize if this widget is the body
			if(!parent){
				inflate();
			}
		}
	}

	void Grid::inflate(){
		Widget::inflate();
		GridLayoutManager::inflateDefinitions(layoutManager->columns, boundingBox.width);
		GridLayoutManager::inflateDefinitions(layoutManager->rows, boundingBox.height);
	};

	void Grid::addChild(Widget* child){
		layoutManager->addChild(child);
	}

	Button::Button(){
		text = new Text("Hello world!");
		text->verticalAlignment = Alignment::Middle;
		text->horizontalAlignment = Alignment::Middle;
	}

	int Button::render(){
		return render(boundingBox);
	}

	int Button::render(const Box& box){
		if(Widget::render(box)){
			text->render(boundingBox);
			return 1;
		}
		return 0;
	}

	void Button::inflate(){
		Widget::inflate();
		text->parseLineBreaks(icoord(boundingBox.width, boundingBox.height));
	}

	Canvas::Canvas(){
		size(10, 10);
	}

	Canvas::Canvas(int bufferWidth, int bufferHeight){
		size(bufferWidth, bufferHeight);
	}

	void Canvas::size(int bufferWidth, int bufferHeight){
		bufferSize.x = bufferWidth;
		bufferSize.y = bufferHeight;
		if(buffer){
			delete[] buffer;
		}
		buffer = new CharInfo[bufferWidth * bufferHeight];
		clear();
	}

	int Canvas::render(){
		return render(boundingBox);
	}

	int Canvas::render(const Box& box){
		boundingBox = box;
		if(parent){
			boundingBox.x -= parent->scrollX;
			boundingBox.y -= parent->scrollY;
		}
		if(!shouldRender()) return 0;
		clip();
		for(int y = 0; y < bufferSize.y; y++){
			for(int x = 0; x < bufferSize.x; x++){
				CharInfo::put(boundingBox.x + x, boundingBox.y + y,
					      buffer[(y * bufferSize.x) + x]);
			}
		}
		unclip();
		return 1;
	}

	void Canvas::inflate(){
		Widget::inflate(); // For x and y
		boundingBox.width = bufferSize.x;
		boundingBox.height = bufferSize.y;
	}

	void Canvas::clear(){
		CharInfo clearChar;
		clearChar.character = ' ';
		clearChar.attributes = color.toAttribute();
		for(int i = 0; i < bufferSize.x * bufferSize.y; i++){
			buffer[i] = clearChar;
		}
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
