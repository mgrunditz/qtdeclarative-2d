# qtdeclarative-2d
ARM optimized version

This is a opensource effort to bring Quick2 to GPU'less systems.
First out is a ARM only version.

It contains some uglyness , like hardcoded screen resolution in qquickitem.cpp.

Build steps:

1, Configure and build Qt 5.3.2 for you ARM target.
2, Clone this repo in to your Qt tree
3, Run qmake inside the clone from ../qtbase/bin/qmake
4, cross your fingers and type make.


Particles is not supported and qtquickcontrols needs som modifications to work.
