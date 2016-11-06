# entrypoint

Lightweight entry point for games.

### Why ?

- [SDL](https://www.libsdl.org/) is too heavy.
- [GLFW](http://www.glfw.org/) is way too OpenGL oriented.
- [TIGR](https://bitbucket.org/rmitton/tigr/src) is not developing :(

So let's take TIGR as a baseline, offload all rendering part to something like [bgfx](https://github.com/bkaradzic/bgfx) and add iOS/Android support.

### Supported platforms

- Windows
- macOS
- iOS
- Emscripten (a bit rough)

### TODO

- GamePad input
- Mouse wheel
- iOS keyboard (at least partially)
- MessageBox/allert
- Android
- X11
- Wayland
- Small casual OpenGL renderer

### Additional

This library very loosely based on TIGR.

	This is free and unencumbered software released into the public domain.
	
	Our intent is that anyone is free to copy and use this software,
	for any purpose, in any form, and by any means.
	
	The authors dedicate any and all copyright interest in the software
	to the public domain, at their own expense for the betterment of mankind.
	
	The software is provided "as is", without any kind of warranty, including
	any implied warranty. If it breaks, you get to keep both pieces.
