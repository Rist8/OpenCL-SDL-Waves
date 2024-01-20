#include <CL/opencl.hpp>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <SDL.h>
#include <math.h>

static std::string kernel_code;
std::ifstream fin("src.cl", std::ifstream::binary);
cl::Device default_device;
cl::Context context;
cl::Kernel Block_UpdateSpeed;
cl::Buffer buffer_A;
cl::CommandQueue queue;
std::ofstream fout("logs.txt");

struct Block {
	Block(float w = 1, float h = 0) :weight(w), speed(0), height(h) {}
	float weight, speed, height, empty = 0;
};

class Wave {
public:
	Wave(unsigned w = 50, unsigned h = 50) :width(w), height(h) { Init(); }

	~Wave() {
		delete[] wave;
	}

	void Update() {
		//UpdateSpeed
		//Write from wave to buffer_A
		clock_t start = clock();
		queue.enqueueWriteBuffer(buffer_A, CL_TRUE, 0, sizeof(Block) * width * height, wave);
		Block_UpdateSpeed.setArg(0, width);
		Block_UpdateSpeed.setArg(1, buffer_A);
		//Start calculations(NDRange is number of threads)
		for (int i = 0; i < 20; ++i) {
			queue.enqueueNDRangeKernel(Block_UpdateSpeed, cl::NullRange, cl::NDRange(width * height), cl::NullRange);
		}
		queue.enqueueReadBuffer(buffer_A, CL_TRUE, 0, sizeof(Block) * width * height, wave);
		queue.finish();
		fout << double(clock() - start) / CLOCKS_PER_SEC << '\n';
	}

	void ImpulseCircle(int x, int y, int r) {
		for (int i = x - r; i <= x + r; ++i)
			for (int j = y - r; j <= y + r; ++j)
				if (pow(i - x, 2) + pow(j - y, 2) <= r * r)
					wave[i + j * width].height = 10;
	}

	void ImpulseRectangle(int x, int y, int a, int b) {
		for (int i = x; i < x + a; ++i)
			for (int j = y; j < y + b; ++j)
				if (i == x || j == y || i == x + a - 1 || j == y + b - 1)
					wave[i + j * width].height = (1);
	}

	void ObjectFilledCircle(int x, int y, int r) {
		for (int i = x - r; i <= x + r; ++i)
			for (int j = y - r; j <= y + r; ++j)
				if (pow(i - x, 2) + pow(j - y, 2) <= r * r)
					wave[i + j * width].weight = (INFINITY);
	}

	void ObjectFilledRectangle(int x, int y, int a, int b) {
		for (int i = x; i < x + a; ++i)
			for (int j = y; j < y + b; ++j)
				wave[i + j * width].weight = float(INFINITY);
	}

	void ObjectRectangle(int x, int y, int a, int b) {
		for (int i = x; i < x + a; ++i)
			for (int j = y; j < y + b; ++j)
				if (i == x || j == y || i == x + a - 1 || j == y + b - 1)
					wave[i + j * width].weight = (INFINITY);
	}

	void ImpulseDot(int x, int y) {
		wave[x + y * width].height = (10);
	}

	inline float operator()(const int x) { return wave[x].weight; }
	float operator[](size_t i) { return wave[i].height; }

private:
	void Init() {
		buffer_A = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(Block) * width * height);
		queue = cl::CommandQueue(context, default_device);
		wave = new Block[width * height];
		for (int j = 0; j < height; ++j)
			wave[j * width].weight = INFINITY, wave[width - 1 + j * width].weight = INFINITY;
		for (int j = 0; j < width; ++j)
			wave[j].weight = INFINITY, wave[j + (height - 1) * width].weight = INFINITY;
	}
	Block* wave;
	unsigned width, height;
};

int main(int argc, char* argv[])
 {
	if (fin) {
		// get length of file:
		fin.seekg(0, std::ios::end);
		kernel_code.reserve(fin.tellg());
		fin.seekg(0, std::ios::beg);
		kernel_code.assign((std::istreambuf_iterator<char>(fin)),
			std::istreambuf_iterator<char>());
		fin.close();
	}

	std::vector<cl::Platform> all_platforms;
	cl::Platform::get(&all_platforms);
	if (all_platforms.size() == 0) {
		std::cout << " No platforms found. Check OpenCL installation!\n";
		exit(1);
	}
	cl::Platform default_platform = all_platforms[0];
	std::vector<cl::Device> all_devices;
	default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
	if (all_devices.size() == 0) {
		std::cout << " No devices found. Check OpenCL installation!\n";
		exit(1);
	}
	default_device = all_devices[0];
	context = cl::Context({ default_device });
	cl::Program::Sources sources;
	sources.push_back({ kernel_code.c_str(),kernel_code.length() });

	cl::Program program(context, sources);
	if (program.build({ default_device }) != CL_SUCCESS) {
		std::cout << " Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\n";
		exit(1);
	}
	Block_UpdateSpeed = cl::Kernel(program, "Block_UpdateSpeed");

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
		return 1;
	SDL_Window* window = SDL_CreateWindow("WAVRS", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1920, 1080, SDL_WINDOW_FULLSCREEN_DESKTOP);
	if (window == NULL)
		return 1;
	SDL_Surface* window_surface = SDL_GetWindowSurface(window);
	uint32_t* screen = (uint32_t*)window_surface->pixels;

	int width = window_surface->w;
	int height = window_surface->h;

	Wave map(width, height);
	map.ImpulseCircle(width / 2 - 200, height / 2, 100);
	map.ImpulseCircle(width / 2 + 200, height / 2, 100);
	//map.ImpulseDot(0, 0);
	//map.ImpulseDot(50, 30);
	SDL_Event event;

	while (1) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) { exit(0); }
		}
		clock_t start = clock();
		for (int i = 0; i < width * height; i++) {
			uint8_t index = std::min(abs((int)map[i]) * 100, 255);
			uint32_t pixel = SDL_MapRGBA(window_surface->format, index, index, index, 255);
			if (map(i) == INFINITY)
				pixel = SDL_MapRGBA(window_surface->format, 255, 0, 0, 255);
			screen[i] = pixel;
		}
		fout << "screen " << double(clock() - start) / CLOCKS_PER_SEC << '\n';
		SDL_UpdateWindowSurface(window);
		map.Update();
	}
}
