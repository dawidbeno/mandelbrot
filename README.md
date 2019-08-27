# Rendering Mandelbrot fractal with SEE and AVX

[Slovak](README.sk.md) version

Application renders Mandelbrot fractar using special processor extensions SSE and AVX to greatly increase rendering speed. User can how to render the image - using special extension as well as size of rendered image. There is also option to launch test to measure redndering speed of images with different sizes and with use of extensions and without them.

![Alt text](images/mandelbrot.png?raw=true "Mandelbrot fractal")

### SIMD
The Flynn classification is probably the best known classification of parallel systems, created in 1966. Systems are split
from two points of view - instruction flow and data flow. This classification includes four main types of parallel systems:


|                 | Single Instruction | Multiple Instruction |
| --------------- | ------------------ | -------------------- |
|   Single Data   |        SISD        |        MISD          |
|  Multiple Data  |        SIMD        |        MIMD          |



## Installation
Application does not require any special installation. Only to download codes and compile with command:
```
make
```
or command:
```
gcc -Wall -Wextra -O3 -msse2 -mavx mandel.c mandel_sse2.c mandel_avx.c -o mandelEXE
```

## Launch
App launches with Bash script:
```
./mandel [-h] [-B] [-S] [-A] [-t] [-w] [-xy num num] [-c]
```
### Options
- **-h** 
	* Shows help.
- **-B**
	* Renders image without using special extensions (Basic state).
- **-S**
	* Redners image with SEE extension.
- **-A**
	* Renders image with AVX extension.
- **-t**
	* Performs a test, which shows average rendering time of images without using special extensions, with SSE and with AVX. Image is rendered 10 times for each type and then average time is calculated. Testing is performed for multiple sizes of image (N x N) with side of 500, 1000, 2000, 3000 and 4000 pixels.
- **-w**
	* Saves the image in the application folder. Name of image is fractal_[Basic / SSE / AVX].png. If this option is used in combination -t option, only last (the biggest one) image from test will be stored.
- **-xy num num**
	* The width and height of the image to be rendered. The **-xy** switch have to be followed by two numbers separated by space.
- -c
	* Cleans the application folder - deletes all rendered images and compiled files.

## Xcode project
Directory **xcode_mandel_aps** contains Xcode project for Macos system.

## License
This project was created as semestral work in FIIT STU within subject Computer architecture. December 2018.

