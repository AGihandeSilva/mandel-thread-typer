# mandel-thread-typer
A multi-threaded mandelbrot fractal generator with selectable thread and numeric type configurations

This application is based on the Qt Mandelbrot Example code available in the Qt Core documentation, and also (optionally) uses the Boost libraries (Version 1.69.0)

![Alt text](./snapshots/MandelThreadTyperScreenGrab.jpg?raw=true "Screen Shot")

Why make yet another Mandelbrot demo?
==================================

This was in part a nostalgic trip back to the days my Dad's old 8-bit computer was kept busy (by me) for minutes, hours, sometimes days making these pretty pictures (I was hoping to do things a bit more quickly this time around). 
I also have developed it with the hope it might be a useful tool for trying out some multi-threaded algorithmic ideas and playing around with numeric data types, in C++14, with some quick visual feedback to help explore those ideas.
Nowadays desktop PCs seem to have more and more cores available, so it seems a shame not to use them sometimes....

Development setups (64-bit only)
=======================

Windows
----------
Windows 10

Qt 5.12.0 + Microsoft Visual Studio Community 2017 (Version 15.4.1)

(Qt Creator 4.8.1 and CMake flows can also be used with the VS compiler as an alternative)

Linux
-------
Ubuntu 18.0.4.1

Qt 5.12.0  + gcc 7.3.0 + Qt Creator 4.8.1

(CMake flow can also be used as an alternative)

Optional requirements
---------------------
For (optional) higher precision type support, the multiprecision library in Boost (I used Version 1.69.0) is additionally required.

Other
===============
The maximum number of threads is currently hard-coded to 10 (more than that should be possible, but that's never been tested).
Other than that, the application should detect the number of CPU cores (taking into account possible multithreading) available and adjust the possible range of worker thread counts accordingly, to leave one or two free for other tasks whilst rendering is underway.

To run the generated executable on Windows, you may need to copy Qt library DLLs into the build output directory.

TODOs
===============
Try it on something with more cores

Document/make more visible some UI features (resize, region select, + and - keys)

Code tidy (alas, always true)

Better handling of the GUI zoom/region select for very small regions (currently precision is lost)

More intelligent sharing of tasks between threads?

HW acceleration? (GPU? FPGA?)







