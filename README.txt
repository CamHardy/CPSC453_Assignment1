Alright so basic operation is easy.

Up/down: increase/decrease the number of levels rendered
Left/right: next/previous scene
Number keys 1-5: jump to a scene
Scene 1: Squares and Triangles
Scene 2: Archimede’s Spiral
Scene 3: Sierpinski Triangle
Scene 4: Barnsley’s Fern
Scene 5: Heighway Dragon Curve

Press escape to close the render window.

*PLEASE NOTE*
My laptop only supports OpenGL version 3.3, it shouldn’t cause any problems. On the chance it does, change the 3 on line 535 and the 3 on line 536 to 4 and 1, respectively. Also my IDE required a full path declaration for the shaders, I’ve changed them back to what I think(?) they were originally, but if you get shader errors that’s the issue. Check lines 194 and 196 and make sure they match your local file paths.

Built on a Mac running OS X El Capitan 10.11.1 using Xcode version 7.3.1