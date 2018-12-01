#ifndef CW_H
#define CW_H

#include <vector>

namespace CW {

	void init();
	void end();
	int screenWidth(); // Functions for now. extern ints in the future when event system is implemented
	int screenHeight();

	long sleep(long);

	struct Box {
		int x, y, width, height;
		Box(); // Empty constructor
		Box(int, int, int, int); // x, y, width, height
		void values(int, int, int, int);
	};

	const char UNIT_CELL = 'C';
	const char UNIT_PERCENT = '%';

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

	struct Widget {
		Unit *x, *y, *width, *height;
		Widget();
		virtual void render();
		virtual void render(const Box&);
		virtual void inflate();
		Widget* parent;
		Box boundingBox;
		std::vector<Widget*> children;
		ColorPair color;
	};

	// Extern variables
	extern std::vector<int> usedColors;
	extern std::vector<int> usedPairs;
	extern ColorPair defaultColorPair;
	extern Color BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE;

}

#endif
