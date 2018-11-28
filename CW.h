#ifndef CW_H
#define CW_H

#include <vector>

namespace CW {

	void init();
	void end();
	int screenWidth(); // Functions for now. extern ints in the future when event system is implemented
	int screenHeight();

	long sleep(long);

	struct Color {
		int id;
		Color(); // Empty constructor
		Color(int); // Construct by color ID
		Color(short, short, short); // Construct by r, g, b
	};

	struct Box {
		int x, y, width, height;
		Box(); // Empty constructor
		Box(int, int, int, int); // x, y, width, height
	};

	const char UNIT_CELL = 'C';
	const char UNIT_PERCENT = '%';

	struct Unit {
		double value, derivedValue;
		char type;
		Unit(); // Empty constructor
		Unit(double, char); // value, type
		void operator =(double);
		void derive(double); // Derive given max
	};

	struct ColorPair {
		int id;
		Color *color1, *color2;
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

	// Forwards declaration for a class which references itself
	struct Widget;

	struct Widget {
		Unit x, y, width, height;
		Widget();
		void render();
		void render(const Box&);
		void inflate();
		Widget* parent;
		std::vector<Widget*> children;
	};

}



#endif
