#ifndef CW_H
#define CW_H

#include <vector>

namespace CW {

	void init();
	void end();
	void loop();
	void loop(int);
	void stopLoop();
	void dispatchEvents();

	long sleep(long);

	struct Box {
		int x, y, width, height;
		Box(); // Empty constructor
		Box(int, int, int, int); // x, y, width, height
		void values(int, int, int, int);
	};

	const char UNIT_CELL = 'C'; // Cell
	const char UNIT_PERCENT = '%'; // Percentage of available space
	const char UNIT_AUTO = 'A'; // Automatically fill space as needed (ignores value)

	struct Unit {
		double value, derivedValue;
		char type;
		Unit(); // Empty constructor
		Unit(double, char);
		virtual void derive(double); // Derive given max
		virtual void operator=(double); // Set value using =
	};

	struct CalculatedUnit : Unit {
		Unit *u1, *u2;
		CalculatedUnit();
		CalculatedUnit(Unit*, char, Unit*);
		virtual void derive(double);
	};

	struct Color {
		int id;
		short r, g, b;
		Color(); // Empty constructor
		Color(int); // Construct by color ID
		Color(short, short, short); // Construct by r, g, b
		void activate();
		static int nextid();
	private:
		// Helper functions for interfacing with ncurses.
		static short cv1000to255(short); // Convert 1000-based colors to 255-based colors
		static short cv255to1000(short); // Convert 255-based colors to 1000-based colors
	};

	struct ColorPair {
		int id;
		Color *foreground, *background;
		ColorPair();
		ColorPair(Color&, Color&);
		void activate();
		static int nextid();
	};

	namespace Draw {
		// Place a single character
		// x, y, char, Color
		void point(int, int, int, ColorPair&);
		// Draw a line
		// x1, y1, x2, y1, Color
		void line(int, int, int, int, ColorPair&);
		// Draw  rect
		// x, y, width, height, color
		void rect(int, int, int, int, ColorPair&);
		// Update the screen
		void update();
	}

	// Event system

	const int EVENT_RESIZE = 0;
	const int EVENT_MOUSE = 1;
	const int EVENT_MOUSE_MOVE = 2;
	const int EVENT_MOUSE_DOWN = 3;
	const int EVENT_MOUSE_UP = 4;
	const int EVENT_MOUSE_CLICK = 5;

	struct Event {
		int type;
		int x, y;
		Event();
		Event(int);
		Event(int, int, int);
	};

	struct MouseEvent : Event {
		int state;
		int count;
		MouseEvent();
		MouseEvent(int);
		MouseEvent(int, int, int);
	};

	struct KeyEvent : Event {
		int key;
		KeyEvent();
		KeyEvent(int);
	};

	struct Widget {
		Unit *x, *y, *width, *height;
		Widget();
		virtual void render();
		virtual void render(const Box&);
		virtual void inflate();
		virtual void addChild(Widget*);
		Widget* parent;
		Box boundingBox;
		std::vector<Widget*> children;
		ColorPair color;
		void handleEvent(Event&);
		void handleMouseEvent(MouseEvent&);
		void handleKeyEvent(KeyEvent&);
	};

	const char UNIT_GRID = '*'; // Fill available space. Unique to grid layouts

	struct GridDefinition {
		double value;
		char type;
		GridDefinition();
		GridDefinition(double, char);
	};

	const int GRID_ODD = -1; // Even amount of cells needed to render the grid
	const int GRID_EITHER = 0; // Only one grid region; any amount of cell will do
	const int GRID_EVEN = 1; // Odd amount of cells needed to render the grid

	struct Grid : Widget {
		std::vector<GridDefinition> columns, rows;
		virtual void render();
		virtual void render(const Box&);
		virtual void inflate();
		virtual void addChild(Widget*);
		void addColumnDefinition(GridDefinition&);
		void addRowDefinition(GridDefinition&);
		// Determine if odd or even amount of cells needed to render
		static int oddOrEven(std::vector<GridDefinition>&);
	};

	// Extern variables
	extern std::vector<int> usedColors;
	extern std::vector<int> usedPairs;
	extern ColorPair defaultColorPair;
	extern Color BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE;
	extern int fps, screenWidth, screenHeight, running;
	extern Widget body;

}

#endif
