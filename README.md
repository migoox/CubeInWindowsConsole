# Cube In Windows Console
CubeInWindowsConsole is an educational purpose small application made in 2022 during holidays, which displays rotating cube in windows console
by accessing console's screen buffer. Implementation is a little bit over-engineered, since the goal was to understand
several stages of simplified OpenGL pipeline.

<img src="https://user-images.githubusercontent.com/56317134/218835687-0a36d9de-abfb-4f9d-86ae-6f4bccab7ead.png" alt="Application screenshot" width=400/>

## Console settings before running the app 
If you want to compile the code and run this application on your own, I recommend using **bitmap font** with equal width and height for your console. On Windows 10 a font can be easily changed in the cmd settings, by navigating "Font" and selecting "Raster Fonts" with  "8x8" size.

<img src="https://user-images.githubusercontent.com/56317134/218835897-1714188c-bc60-431f-bde9-fd20a18f972b.png" alt="Bitmap font" width=300/>

## Implementation details
The fundamental aim was to build everything from scratch, so there is no external math library included. Basic vectors, matrices and operations like multiplication are implemented at the beginning of the main file. 

To implement basic MVP (Model, View, Projection) mechanism, functions returning matrices for these operations where also written. It includes:
```cpp
static mat4 ProjectionMat(float ratio, float fov, float zNear, float zFar);
static mat4 ScaleMat(vec3 scaleVec);
// Rotation is an enum type, can be one of the following: Pitch, Yaw, Roll
static mat4 RotationMat(float angle, Rotation type);
static mat4 TranslationMat(vec3 translation)
```  
For rotation I've decided to use simple **euler angles**.

Objective coordinates of every vertex are first translated using MVP translation **normalized homogeneous coordinates** (in the range $[-1.0, 1.0]$) and then rasterized which produces fragments (in our case pixels - pixel is represented by one char in console).The **Draw function** is responsible for both described stages.

The moment of MVP translation mentioned above is handled by the function 
```cpp
vec3 VertexShader(vec3 inPos);
```
invoked for the each vertex given as input in the Draw function on every frame. As the function's name suggest, this moment is equivalent of OpenGL's vertex shader and can be treated like one (it's handled on CPU not GPU obviously).

After acquaring normalized homogeneous coordinates rasterization process begins. This application supports two types of rasterization - points and lines. 

Function 
```cpp
static void LitPixel(Color* frameBuffer, float x, float y, Color color);
```
allows changing a color in the colorbuffer of the pixel closest to given normalized homogeneous coordinates given as an input. This function is used for lines rasterization as well as for lines rasterization. For lines DDA(Digital Differential Analyzer) algorithm has been used. 

At the end of the main loop of this application new frame buffer replaces console's screen buffer. Described Draw function pipeline can be seen on the image below.

<img src="https://user-images.githubusercontent.com/56317134/218835774-373743dc-3428-4424-9ba8-f17c70373699.png" alt="Bitmap font" width=300/>
