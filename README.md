Very basic implementation of a Wayland client using XDG shell, displays a resizable semi-tranparent black window.

To build and run:
```
$ git clone https://github.com/OnatBulut/Wayland-XDG-Shell-Example.git
$ cd Wayland-XDG-Shell-Example
$ mkdir build && cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Release -G Ninja
$ cmake --build .
$ ./Wayland_XDG_Shell_Example
```

Dependencies
```
wayland
```

Build dependencies:
```
cmake
ninja
```
