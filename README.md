# CW

*CW is a toolkit for creating faux graphical widgets in the terminal, complete with
many complex features. The goal is to be on par with HTML, WPF, or Qt, as much as
the terminal allows. Windows support is planned but not likely for some time.*

# Tech Demo 1

This is probably what you're here for: the tech demo. It's very easy to run the demo.
Make sure you have g++ and ncurses installed. On apt-based linux distros, use the
`libncurses5-dev` package; `sudo apt install libncurses5-dev`. With pacman, you can
simply use the `ncurses` package, which is often already installed on your system;
`pacman -S ncurses`.

As of now, and as planned for the forseeable future, ncurses is the **only** dependency

### Running the demo

**IMPORTANT NOTE: THIS PROGRAM MODIFIES THE INPUT MODE OF YOUR TERMINAL TO COLLECT MOUSE
MOVEMENT, IF AVAILABLE. THIS SHOULD BE RETURNED TO NORMAL AFTER EXECUTION OF THE PROGRAM,
BUT IF IT ISN'T, THIS IS NOT PERMANENT. RESTARTING THE TERMINAL WILL FIX THE ISSUE.**

(If you don't see a bunch of garbage when you move your mouse, then there is no issue!)

Navigate to the directory with the code, and execute the command `./build.sh && ./CWTest`.

`build.sh` is a very simple command which just executes g++ and outputs the binary to
the executable file `./CWTest`. You *may* need to `sudo chmod +x` both of these files,
but you probably will not need to.

You should see some colorful boxes with a blank border. If you do, **congratulations**,
CW is running on your machine!

The demo will continue to run as long as you let it. It will respond to resize events.
Press "q" to quit.

### What's in the demo?

A lot, but not much. When you run the tech demo, You will see three colorful boxes; one
large, green box (the shade of green depends on your terminal emulator), one small red
box, and one tiny yellow box.

In short, it's the main widget, which has two children. One is a 3x3 red box, and one is
a 1x1 yellow box. The main widget has clipping enabled, so children that try to render
outside of the main widget will not be able to. The main widget is then scrolled on it's
y axis to hide one cell of the top (red 3x3) widget. The widget displays as 3x2 in the
demo because of this (The top is clipped). The main widget has a stacking layout manager
applied to it, which means that the widgets are stacked on top of each other (or,
optionally, side-by-side). Technology demonstrated here:

- Abstract widget rendering
- Hierarchical widget rendering (parents/ children)
- Abstract layout management
- Abstract clipping
- Input collection
- Color manager
- Abstracted terminal interaction (For Windows support, later on; See the `Draw` namespace)

In even greater detail:

**The big green one**

The big green one is the main widget. After HTML, I have named this widget the `body`.
This widget is created automatically and a pointer to it can be accessed under
`CW::body`.

The body has the dimensions of the size of the screen, minus two cells (They aren't
pixels, so I have opted for the term "cells"). The body is then placed at (1, 1).
This is to give it a 1 cell "border" where absolutely nothing is rendered. If you have
a terminal emulator with transparency enabled, you will see your desktop.

Here are some important lines from the source code used to achieve this:

*please note that all source code assumes the use of `using namespace CW;`*

    body->color = green;
    
    body->x->value = 1;
    body->y->value = 1;
    
    // Reads as "100% - 2cells (px)"
    CalculatedUnit *bodySize = new CalculatedUnit(new Unit(100, UNIT_PERCENT), '-', new Unit(2, UNIT_CELL));
    
    // Destroy the basic Units that the body uses for width and height, and replace them
    delete body->width;
    delete body->height;
    body->width = bodySize;
    body->height = bodySize;
    
    // Re-inflate the body after size changes
    body->inflate();
    
**The small red one**

While it renders as 3x2, this red widget is a 3x3 basic widget. More on it later.

**The tiny yellow one**

This is just a basic yellow widget, with 1 cell width and 1 cell height.

**The layout**

One of the more impressive items here is the abstract layout that you can apply to
any widget. A **layout** is a subclass of the **LayoutManager** abstract class,
which manages the size and positioning of child widgets. The default is an absolute
layout manager, which lets the widgets just place themselves. This is the equivalent
of having *no* layout manager. The layout used in this tech demo is a stacking
layout: this is very similar to the way HTML works by default, or the way a
`StackPanel` works in WPF. Items are stacked either side-to-side or on top of each
other. Here's the code used to achieve that:

    StackingLayoutManager *layout = new StackingLayoutManager();
    layout->orientation = Orientation::Vertical;
    body->setLayoutManager(layout);
    
Really, quite simple.

**Clipping**

Something I am personally proud of is the abstract clipping system used in the demo.
The clipping system is based on one abstract class: `Shape`. Simply enough, every time CW is
asked to place a pixel, it first checks if that pixel is within the shape. This shape
can be any logically-expressable shape. Even patterns, like checkerboards are easily
implemented. The default is no clipping: any child outside the boundaries of it's parent
will be rendered. The only available shape in this tech demo is a `Rectangle`, which
is applied to the body:

    Rectangle *bodyClipper = new Rectangle();
    body->clipShape = bodyClipper;
    
Shapes are stacked, so if you have one shape applied for clipping, and then a child widget
has yet another shape, both shapes are evaluated before placing any pixels.

This is why the red 3x3 widget is actually rendered as a 3x2 widget: the first cell is clipped

**Scrolling**

The feature which was added most recently before the tech demo was put together is scrolling.
Each widget allows for scrolling in both axis. The body of the tech demo is scrolled by 1
cell in the Y axis, making the small red widget overflow out of the top of the body. This
is why the widget is clipped.

    body->scrollY = 1;
    
I encourage you to play around with `test.cpp`. Of course, the code is a bit fragile, and
there's no documentation, but you should be able to play around with most of the items in
the test file to get a feel for the technology.

-Algo
