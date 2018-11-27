#ifndef CW_H
#define CW_H

namespace CW {

	void init();
	void end();

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
		double value;
		char type;
		Unit(); // Empty constructor
		Unit(double, char); // value, type
		void operator =(double);
		double derive();
	};

	struct ColorPair {
		int id;
		Color *color1, *color2;
	};

	namespace Draw {
		// Place a single character
		// x, y, char, Color
		void place(int, int, int, ColorPair);
		// Draw a line
		// x1, y1, x2, y1, Color
		void line(int, int, int, int, ColorPair);
		// Draw  rect
		// x, y, width, height, color
		void rect(int, int, int, int, ColorPair);
	}

	struct Widget {
		Unit x, y, width, height;
	};

}



#endif
