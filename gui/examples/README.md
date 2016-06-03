AVTK - OpenAV Productions
==========================

This is the repository for AVTK, an Audio / Visual ToolKit.

General Info
------------
AVTK is based on PUGL and Cairo, two great libraries. It minimizes what 
dependencies it has by using these two frameworks. PUGL is a cross platform GL 
abstraction, while Cairo provides fast vector graphic rendering to GL surfaces.

Avtk::UI
--------
Avtk::UI is the main UI class, where the widget logic resides. The UI class wraps
a PUGL view, and manages redrawing and theming of widgets. The UI pointer is passed
to all widgets, which gives it a lot of flexibility to control other elements.

In order to write a custom UI, derive a class `CustomUI` from `Avtk::UI`, and 
create some widgets in its `CustomUI()` constructor. Add the created widgets 
using the `add( Avtk::Widget* )` function, which is inheritited from 
`Avtk::UI`.

The `Avtk::UI` class uses a smart-pointer to manager the `Widget*` instances 
passed in, so we do not need to worry about memory leaks when dealing with 
widgets in this way. 

Widgets
-------
The widget is the main UI element in AVTK. Creating custom widgets is very 
easy: just derive from the widget class, and override the `draw()` function. In 
order to have proper mouse interaction for `drag` and `click` modes, checkout 
`widget.hxx`, which details what `ClickMode`, `DragMode`, and `Scroll` options 
there are.

The `Widget` superclass has `void value(float)` and `float value()` methods, 
for setting and getting the most important value of the widget. As not all 
widgets can be described with one value (think an ADSR or other complex 
widgets), add custom `float getParamX()` and `void setParamX( float )` 
functions. These will need to be explicity controlled by the UI, but there's no
way around that.

Themes
------
Each `Avtk::UI` instance has a `Theme*`, which contains the 
current color values for the `Theme`. Note that line-widths and 
corner-rounded-ness are also contained within the Theme class.

Creating a new Theme can be done by deriving from the `Theme` class, and then 
overriding the `float setColor( cairo_t* cr, USE_CASE uc, float alpha);` 
function. This function sets `USE_CASE` color as the active color for Cairo 
context `cr` when it is called. When the given `USE_CASE` exists in the theme
the custom `setColor()` function returns non-zero: this tells Avtk to not apply
the default color.

If the `setColor()` function returns null, then the default compiled in AVTK 
theme will be used.

Building
--------
To build a static library from the AVTK source:
```
cd avtk
mkdir build
cd build
cmake ..
make
```
The current directory now has two binaries
1. The `avtkDemo` application, just run to see an AVTK UI
2. The `avtkTests` widget tester, run with a command line argument to test
   the specific widget. Examples include `./avtkTests` and pass an argument
   from the following list: `dialog` `dial` `group` `key` `list` `number`

Using
-----
To use AVTK in your own application, it is recommended to *copy* the source
code into your tree. This allows you to link statically to AVTK, which
avoids a lot of issues when embedding plugins UIs. OpenAV does not intend
to support dynamic linking of AVTK, or maintaining a backwards-compatilble
ABI.

After building AVTK, a `libavtk.a` static archive of AVTK is in the build
directory. Statically link your application against this .a file, and
you're ready to run!

License
-------
AVTK is licensed under the 3-clause BSD license. This allows commercial use
of the library.

Contact
-------
If your using AVTK, my curiosity would like an email - this is however in
no way mandatory.

Harry van Haaren    harryhaaren@gmail.com
OpenAV Productions  openavproductions.com
