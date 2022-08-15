#include <windows.h>
#include <chrono>
#include <math.h>
#define PI 3.14159265359

enum class Color 
{
	Black = 0, 
	White = 119,
	Red = 68,
	Green = 34, 
	Blue = 17
};

enum class RenderType
{
	Points,
	Lines
};

enum class Rotation
{
	Pitch, // x
	Yaw,   // y
	Roll   // z
};

// basic math
struct vec3
{
	float x, y, z;

	vec3() :
		x(0.f), y(0.f), z(0.f) { }
	vec3(float x, float y, float z) :
		x(x), y(y), z(z) { }

	friend vec3 operator+(const vec3& v1, const vec3& v2) { return vec3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z); }
	friend vec3 operator-(const vec3& v1, const vec3& v2) { return vec3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z); }
	friend vec3 operator*(const float& s, const vec3& v) { return vec3(s * v.x, s * v.y, s * v.z); }
	void Normalize() 
	{ 
		float magnitude = sqrt(x * x + y * y + z * z);
		x /= magnitude;
		y /= magnitude;
		z /= magnitude;
	}
};

	// matrix array indices:
	// | 0   1   2   3  |
	// | 4   5   6   7  |
	// | 8   9   10  11 |
	// | 12  13  14  15 |
struct mat4
{
	float data[16];
	mat4() // id matrix by deafult
	{
		for (unsigned int i = 0; i < 4; i++)
		{
			for (unsigned int j = 0; j < 4; j++)
			{
				if(i == j)
					data[i * 4 + j] = 1.f;
				else
					data[i * 4 + j] = 0.f;
			}
		}
	}
	friend mat4 operator*(const mat4& m1, const mat4& m2)
	{
		mat4 m;
		float sum;
		for (unsigned int i = 0; i < 4; i++)
		{
			for (unsigned int j = 0; j < 4; j++)
			{
				sum = 0.f;
				for (unsigned int k = 0; k < 4; k++)
					sum += m1.data[i * 4 + k] * m2.data[k * 4 + j];
				m.data[i * 4 + j] = sum;
			}
		}
		return m;
	}
};

	// mvp matrices
static mat4 ProjectionMat(float ratio, float fov, float zNear, float zFar)
{
	mat4 m;
	float a = ratio;
	float f = 1.f / tan(fov / 2.f);
	float q = zFar / (zFar - zNear);
	m.data[0] = a * f;	 m.data[1] = 0;		m.data[2] = 0;		m.data[3] = 0;
	m.data[4] = 0;		 m.data[5] = f;		m.data[6] = 0;		m.data[7] = 0;
	m.data[8] = 0;		 m.data[9] = 0;		m.data[10] = q;		m.data[11] = -zNear * q;
	m.data[12] = 0;		 m.data[13] = 0;	m.data[14] = 1;		m.data[15] = 0;

	return m;
}
static mat4 ScaleMat(vec3 scaleVec)
{
	mat4 scale;
	scale.data[0] = scaleVec.x;  scale.data[1] = 0;				scale.data[2] = 0;				scale.data[3] = 0;
	scale.data[4] = 0;			 scale.data[5] = scaleVec.y;	scale.data[6] = 0;				scale.data[7] = 0;
	scale.data[8] = 0;			 scale.data[9] = 0;				scale.data[10] = scaleVec.z;	scale.data[11] = 0;
	scale.data[12] = 0;			 scale.data[13] = 0;			scale.data[14] = 0;				scale.data[15] = 1;
	return scale;
}
static mat4 RotationMat(float angle, Rotation type)
{
	float c = cos(angle);
	float s = sin(angle);

	mat4 rot;
	if (type == Rotation::Pitch)
	{
		rot.data[0] = 1;		 rot.data[1] = 0;		rot.data[2] = 0;		rot.data[3] = 0;
		rot.data[4] = 0;		 rot.data[5] = c;		rot.data[6] = -s;		rot.data[7] = 0;
		rot.data[8] = 0;		 rot.data[9] = s;		rot.data[10] = c;		rot.data[11] = 0;
		rot.data[12] = 0;		 rot.data[13] = 0;		rot.data[14] = 0;		rot.data[15] = 1;
	}
	else if (type == Rotation::Yaw)
	{
		rot.data[0] = c;		 rot.data[1] = 0;		rot.data[2] = s;		rot.data[3] = 0;
		rot.data[4] = 0;		 rot.data[5] = 1;		rot.data[6] = 0;		rot.data[7] = 0;
		rot.data[8] = -s;		 rot.data[9] = 0;		rot.data[10] = c;		rot.data[11] = 0;
		rot.data[12] = 0;		 rot.data[13] = 0;		rot.data[14] = 0;		rot.data[15] = 1;
	}
	else
	{
		rot.data[0] = c;		 rot.data[1] = -s;		rot.data[2] = 0;		rot.data[3] = 0;
		rot.data[4] = s;		 rot.data[5] = c;		rot.data[6] = 0;		rot.data[7] = 0;
		rot.data[8] = 0;		 rot.data[9] = 0;		rot.data[10] = 1;		rot.data[11] = 0;
		rot.data[12] = 0;		 rot.data[13] = 0;		rot.data[14] = 0;		rot.data[15] = 1;
	}
	return rot;
}
static mat4 TranslationMat(vec3 translation)
{
	mat4 trans;
	trans.data[0] = 1;		 trans.data[1] = 0;		trans.data[2] = 0;		trans.data[3] =  translation.x;
	trans.data[4] = 0;		 trans.data[5] = 1;		trans.data[6] = 0;		trans.data[7] =  translation.y;
	trans.data[8] = 0;		 trans.data[9] = 0;		trans.data[10] = 1;		trans.data[11] = translation.z;
	trans.data[12] = 0;     trans.data[13] = 0;		trans.data[14] = 0;		trans.data[15] = 1;
	return trans;
}

// size of the screen (pixels)
static const unsigned int WindowX = 160;
static const unsigned int WindowY = 100;

// win32 api
static HANDLE ConsoleOut;
static CHAR_INFO ConsoleBuffer[WindowX * WindowY];
static void InitConsole();

// "engine"
	// fill frame buffer pixels with specified color
static void Clear(Color* frameBuffer, Color color = Color::Black);

	// modify frame buffer
static void Draw(Color* frameBuffer, vec3* vertexBuffer, unsigned int vertexCount, RenderType type = RenderType::Points, Color color = Color::White);

	// translate frame buffer to console buffer
static void Display(Color* frameBuffer);

	// x and y should be converted from NDS to screen values
static void LitPixel(Color* frameBuffer, float x, float y, Color color = Color::White);

	// vertex shader (called in draw function, before rasterization)
static vec3 VertexShader(vec3 inPos);

	// Model-View-Projection (MVP) matrix, used in vertex shader
static mat4 MVPMat;

int main()
{
	InitConsole();

	// frame buffer stores colors for each pixel
	Color FrameBuffer[WindowX * WindowY];

	//		   7--------6
	//		  /|       /|
	//		 / |      / |
	//		4--------5  | height
	//		|  |     |  |
	//		|  3-----|--2
	//		| /      | / 
	//		|/       |/ width
	//		0--------1
	//		  length

	// length = width = height = x
	float x = 0.5f;

	// vertex buffer (lines)
	vec3 VertexBuffer[] = 
	{
		vec3(0.f, 0.f, 0.f),	// 0 - 1
		vec3(x, 0.f, 0.f),
		vec3(x, 0.f, 0.f),		// 1 - 2
		vec3(x, 0.f, x),
		vec3(x, 0.f, x),		// 2 - 3
		vec3(0.f, 0.f, x),
		vec3(0.f, 0.f, x),		// 3 - 0
		vec3(0.f, 0.f, 0.f),
		vec3(0.f, 0.f, 0.f),	// 0 - 4
		vec3(0.f, x, 0.f),		
		vec3(x, 0.f, 0.f),		// 1 - 5
		vec3(x, x, 0.f),
		vec3(x, 0.f, x),		// 2 - 6
		vec3(x, x, x),
		vec3(0.f, 0.f, x),		// 3 - 7
		vec3(0.f, x, x),
		vec3(0.f, x, 0.f),		// 4 - 5
		vec3(x, x, 0.f),
		vec3(x, x, 0.f),		// 5 - 6
		vec3(x, x, x),
		vec3(x, x, x),			// 6 - 7
		vec3(0.f, x, x),
		vec3(0.f, x, x),		// 7 - 4
		vec3(0.f, x, 0.f)
	};

	// angle info
	float angle = 0.f;
	float angleSpeed = PI / 8.f;

	// delta time
	auto start = std::chrono::steady_clock::now();
	auto end = std::chrono::steady_clock::now();

	while (true)
	{
		// update delta time
		end = std::chrono::steady_clock::now();
		auto dt = float(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()) / 1000000.f;
		start = std::chrono::steady_clock::now();
		
		// update object (cube)
		angle += dt * angleSpeed;

		mat4 proj = ProjectionMat(((float)WindowY) / ((float)WindowX), 55.f, 0.3f, 1000.f);
		mat4 rot = RotationMat(angle, Rotation::Pitch) * RotationMat(angle, Rotation::Yaw) * RotationMat(angle, Rotation::Roll);
		mat4 scale = ScaleMat(vec3(2.f, 2.f, 2.f));
		mat4 origin = TranslationMat(vec3(-x / 2.f, -x / 2.f, -x / 2.f));
		MVPMat = proj * rot * scale * origin;

		// draw
		Clear(FrameBuffer, Color::Blue);
		Draw(FrameBuffer, VertexBuffer, 24, RenderType::Lines, Color::White);
		Draw(FrameBuffer, VertexBuffer, 24, RenderType::Points, Color::Red);
		Display(FrameBuffer);
	}

	SetConsoleTextAttribute(ConsoleOut, int(Color::Black));
	return 0;
}

void InitConsole()
{
	// get console handle
	ConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);

	// fill console buffer with 219 char
	for (int y = 0; y < WindowY; ++y)
	{
		for (int x = 0; x < WindowX; ++x)
		{
			ConsoleBuffer[x + WindowX * y].Char.AsciiChar = (unsigned char)219;
		}
	}
}

void Clear(Color* frameBuffer, Color color)
{
	for (unsigned int i = 0; i < WindowY; i++)
	{
		for (unsigned int j = 0; j < WindowX; j++)
		{
			frameBuffer[i * WindowX + j] = color;
		}
	}
}

void Draw(Color* frameBuffer, vec3* vertexBuffer, unsigned int vertexCount, RenderType type, Color color)
{
	if (type == RenderType::Points)
	{
		for (int i = 0; i < vertexCount; i++)
		{
			// apply vertex shader and get current vertex
			vec3 v = VertexShader(vertexBuffer[i]);

			// vertex NDS to screen conversion
			v.x = WindowX * (v.x + 1.f) / 2.f;
			v.y = WindowY * (1.f - v.y) / 2.f;

			// draw point
			LitPixel(frameBuffer, v.x, v.y, color);
		}
	}
	else // Type::Lines
	{
		for (int i = 0; i < vertexCount - (vertexCount % 2); i += 2)
		{
			// apply vertex shader and get current vertex
			vec3 v1 = VertexShader(vertexBuffer[i]);
			vec3 v2 = VertexShader(vertexBuffer[i + 1]);

			// vertex NDS to screen conversion
			v1.x = WindowX * (v1.x + 1.f) / 2.f;
			v1.y = WindowY * (1.f - v1.y) / 2.f;
			v2.x = WindowX * (v2.x + 1.f) / 2.f;
			v2.y = WindowY * (1.f - v2.y) / 2.f;

			// prepare delta x and delta y
			float dx = v2.x - v1.x, dy = v2.y - v1.y;
			float step = 0.f;

			if (abs(dx) >= abs(dy))
				step = abs(dx);
			else
				step = abs(dy);

			dx = dx / step;
			dy = dy / step;

			// starting point
			float x = v1.x, y = v1.y;

			// iteration counter 
			int iterations = 0;

			// DDA (Digital differental analyzer) algorithm
			while (step >= float(iterations))
			{
				LitPixel(frameBuffer, x, y, color);
				x = x + dx;
				y = y + dy;
				iterations++;
			}
		}

		// if last vertex has no pair it will be drawn as a point
		if (vertexCount % 2 != 0)
		{
			// apply vertex shader and get current vertex
			vec3 v = VertexShader(vertexBuffer[vertexCount - 1]);

			// vertex NDS to screen conversion
			v.x = WindowX * (v.x + 1.f) / 2.f;
			v.y = WindowY * (1.f - v.y) / 2.f;

			// draw point
			LitPixel(frameBuffer, v.x, v.y, color);
		}
	}
}

void Display(Color* frameBuffer)
{
	// copy frame buffer colors to console buffer
	for (unsigned int i = 0; i < WindowY; i++)
	{
		for (unsigned int j = 0; j < WindowX; j++)
		{
			ConsoleBuffer[i * WindowX + j].Attributes = int(frameBuffer[i * WindowX + j]);
		}
	}
	
	// prepare console drawing position info 
	COORD characterBufferSize = { WindowX, WindowY };
	COORD characterPosition = { 0, 0 };
	SMALL_RECT consoleWriteArea = { 0, 0, WindowX - 1, WindowY - 1 };

	// apply console buffer
	WriteConsoleOutput(ConsoleOut, ConsoleBuffer, characterBufferSize, characterPosition, &consoleWriteArea);
}

void LitPixel(Color* frameBuffer, float x, float y, Color color)
{
	int posX = int(x), posY = int(y);

	if (x > 0.f)
		posX = int(x) - 1;
	if (y > 0.f)
		posY = int(y) - 1;

	if (posX >= WindowX || posX < 0 || posY >= WindowY || posY < 0)
		return;

	frameBuffer[posY * WindowX + posX] = color;
}

vec3 VertexShader(vec3 inPos)
{
	vec3 outPos;

	// outPos = (x, y, z, 1.f)
	// matrix and vector multiplication:  MVPMat * outPos
	outPos.x = MVPMat.data[0]  * inPos.x + MVPMat.data[1]  * inPos.y + MVPMat.data[2]  * inPos.z + MVPMat.data[3]  * 1.f;
	outPos.y = MVPMat.data[4]  * inPos.x + MVPMat.data[5]  * inPos.y + MVPMat.data[6]  * inPos.z + MVPMat.data[7]  * 1.f;
	outPos.z = MVPMat.data[8]  * inPos.x + MVPMat.data[9]  * inPos.y + MVPMat.data[10] * inPos.z + MVPMat.data[11] * 1.f;

	return outPos;
}