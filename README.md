icos
====

An interactive OpenGL [icosahedral sphere-tiling](http://kiwi.atmos.colostate.edu/rr/groupPIX/ross/ross1/ross1.html) demo.

###Build

You'll need the OpenGL Utility Toolkit (GLUT) installed. In Ubuntu, installing freeglut3 and freeglut3-dev seems to do the trick. Then run `make`.

###Run

Run `icos`.

Visibility of [a]xes, [c]entroids, [e]dges, [n]ormals and the [s]phere can be toggled by their respective initial-letter keys. If [f]ixed refinement is enabled, the sphere will not rotate during refinement, unless [g]o is enabled. If ani[m]ate is enabled, lines bisecting the triangle faces will be drawn as an animation; otherwise, they will appear all at once. If [r]efine is set to 2-step, bisection of the triangle faces will occur with the first press of the `>` key, and extension of the new vertices with the second; otherwise, bisection and extension will happen in a single step. The [t]exture key cycles through a series of sphere textures. Other keys should be self-explanatory.

###License

The original contents of this repository are released under the [Apache 2.0](http://www.apache.org/licenses/LICENSE-2.0) license. See the LICENSE file for details. The texture images are from the [Visible Earth](http://visibleearth.nasa.gov) project and are owned by NASA.

