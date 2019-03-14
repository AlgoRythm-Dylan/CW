#ifndef CW_H
#define CW_H

#include <vector>
#include <string>

namespace CW {

	struct Widget;

	void init();
	void end();
	void loop();
	void loop(int);
	void stopLoop();
	void dispatchEvents();
	void updateScreenSize();
	void setBody(Widget*);

	long sleep(long);

	// Integer coordinate structure. For x/y **OR** width/height
	struct icoord {
		int x, y;
		icoord();
		icoord(const icoord&);
		icoord(int, int);
	};

	struct Box {
		int x, y, width, height;
		Box(); // Empty constructor
		Box(const Box&); // Copy constructor
		Box(int, int, int, int); // x, y, width, height
		void values(int, int, int, int); // Easy way to set all values
	};

	const char UNIT_CELL = 'C'; // Cell
	const char UNIT_PERCENT = '%'; // Percentage of available space
	const char UNIT_AUTO = 'A'; // Automatically fill space as needed (ignores value)

	struct Unit {
		double value, derivedValue;
		char type;
		Unit(); // Empty constructor
		Unit(double, char);
		virtual double derive(double); // Derive given max
		void operator=(double); // Set value using =
	};

	struct CalculatedUnit : Unit {
		Unit *u1, *u2;
		CalculatedUnit();
		CalculatedUnit(Unit*, char, Unit*);
		virtual double derive(double);
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
		short toAttribute();
		void activate();
		static int nextid();
	};

	enum Alignment {
		Start,
		Middle,
		End
	};

	struct Text {
		std::string text;
		std::vector<int> lineBreaks;
		Alignment verticalAlignment, horizontalAlignment;
		Text();
		Text(std::string); // Going to want a copy of the string
		ColorPair color;
		virtual void parseLineBreaks(const icoord&);
		virtual void render(const Box&);
	private:
		int getNextLineBreak(int); // Helper function
	};

	struct CharInfo {
		short attributes;
		int character;
		CharInfo();
		CharInfo(const CharInfo&);
		static void put(int, int, const CharInfo&);
	};

	// Text with colors and other attributes!
	struct RichText : Text {
		// Future plans!
	};

	namespace Draw {
		// Place a single character
		// x, y, char, Color
		void point(int, int, int, const ColorPair&);
		// Draw a line
		// x1, y1, x2, y1, Color
		//void line(int, int, int, int, const ColorPair&);
		// Draw  rect
		// x, y, width, height, color
		void rect(int, int, int, int, const ColorPair&);
		// Overwrite region with default color pair and empty cells
		// x, y, width, height
		void clearRect(int, int, int, int);
		void update();
		void clear();
		int clipCheck(int, int);
	}

	// Event system

	enum EventType {
		WindowResize,
		ParentResize,
		Mouse,
		MouseMove,
		MouseDown,
		MouseUp,
		MouseClick,
		Key,
		Generic,
		Invalid
	};

	struct Event {
		EventType type;
		int x, y;
		int stopped; // Boolean. Is the event stopped/ cancelled?
		Event();
		Event(EventType); // Generic-as-it-gets event
		Event(EventType, int, int); // (Usually) EventType at position x, y
		void stop();
	};

	struct MouseEvent : Event {
		int state; // The mouse state, from ncurses
		int count; // Double, even triple clicks are reported from curses
		MouseEvent();
		MouseEvent(EventType); // Type
		MouseEvent(EventType, int, int); // Type, x, y
		MouseEvent(EventType, int, int, int); // Type, x, y, count
	};

	struct KeyEvent : Event {
		int key;
		KeyEvent();
		KeyEvent(int); // Constructor taking key
	};

	// Abstract class to describe shapes
	struct Shape {
		Box rect;
		virtual int contains(int, int) = 0;
		virtual void setRect(const Box&) = 0;
	};

	// The most basic shape type
	struct Rectangle : Shape {
		virtual int contains(int, int);
		virtual void setRect(const Box&);
	};

	// Layout manager base class
	struct LayoutManager {
		Widget* widget = nullptr;
		virtual void render() = 0;
	};

	// Basic layout manager, perhaps just a bit verbose. No, self-documenting.
	struct AbsoluteLayoutManager : LayoutManager {
		virtual void render();
	};

	enum Orientation {
		Horizontal,
		Vertical
	};

	// More controlling layout manager
	struct StackingLayoutManager : LayoutManager {
		Orientation orientation = Orientation::Vertical;
		Alignment verticalAlignment = Alignment::Start;
		Alignment horizontalAlignment = Alignment::Start;
		virtual void render();
	};

	const char UNIT_GRID = '*'; // Fill available space. Unique to grid layouts	

	struct GridDefinition {
		double value;
		int inflatedValue;
		char type;
		GridDefinition();
		GridDefinition(double, char);
	};

	struct GridChild {
		GridChild();
		GridChild(Widget*, int, int);
		int column, row;
		int colSpan, rowSpan;
		Widget* child;
	};

	struct GridLayoutManager : LayoutManager {
		virtual void render();
		std::vector<GridDefinition> columns, rows;
		std::vector<GridChild> childPositions;
		void addChild(Widget*);
		void addChild(Widget*, int, int);
		Widget* getChildAt(int, int);
		Widget* setChildAt(Widget*, int, int);
		void addColumnDefinition(GridDefinition&);
		void addRowDefinition(GridDefinition&);
		static void inflateDefinitions(std::vector<GridDefinition>&, int);
	};

	struct ScrollBar {
		int visible = 1;
		virtual void render() = 0;
		Box boundingBox;
	};

	struct Widget {
		Unit *x, *y, *width, *height;
		Shape *clipShape = nullptr;
		int usedClip; // Boolean value: did this widget use a clip in the most recent render?
		LayoutManager *layoutManager = nullptr;
		int scrollX, maxScrollX, scrollY, maxScrollY;
		ScrollBar *verticalScrollbar, *horizontalScrollBar;
		int xScrollDisabled = 1, yScrollDisabled = 1;
		int cullChildren = 0, disableCulling = 0;
		Widget();
		virtual int render();
		virtual int render(const Box&);
		virtual void inflate();
		virtual void addChild(Widget*);
		Widget* parent;
		Box boundingBox;
		std::vector<Widget*> children;
		ColorPair color;
		virtual void handleEvent(Event&);
		virtual int contains(int, int);
		// What will be the dimensions of the widget, given some hypothetical available space to render to?
		virtual void setLayoutManager(LayoutManager*);
		void clip();
		void unclip();
		int shouldRender();
	};

	struct Grid : Widget {
		GridLayoutManager *layoutManager = nullptr;
		Grid();
		virtual int render();
		virtual int render(const Box&);
		virtual void inflate();
		virtual void addChild(Widget*);
		virtual void handleEvent(Event&);
	};

	struct Button : Widget {
		Text *text = nullptr;
		Button();
		virtual int render();
		virtual int render(const Box&);
		virtual void inflate();
	};

	// TODO: make double buffered mode
	struct Canvas : Widget {
		CharInfo *buffer = nullptr;
		icoord bufferSize;
		Canvas();
		Canvas(int, int);
		void size(int, int);
		virtual int render();
		virtual int render(const Box &);
		virtual void inflate();
		void clear();
	};

	// Extern variables
	extern std::vector<int> usedColors;
	extern std::vector<int> usedPairs;
	extern ColorPair defaultColorPair;
	extern Color BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE;
	extern int fps, screenWidth, screenHeight, running;
	extern Widget* body;
	extern std::vector<Shape*> clipShapes;

}

#endif
