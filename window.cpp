#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <opencv2/opencv.hpp>

#define PORT 54321

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t ui8;
typedef uint16_t ui16;
typedef uint32_t ui32;
typedef uint64_t ui64;

typedef float f32;
typedef double f64;

int main(int argc, char* const argv[]){
	printf("Starting Main...\n");
	fflush(stdout);
	
	// socket setup 
	int client_fd;
	struct sockaddr_in serv_addr;
	if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("Socket creation error\n");
		return -1;
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0){
		printf("Invalid address\n");
		return -1;
	}

	if (connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		printf("Connection failed - is server running?\n");
		return -1;
	}
	printf("Connected to the server on port %d\n", PORT);
	
	// SDL setup
	if (SDL_Init(SDL_INIT_VIDEO) != 0){
		printf("error initializing SDL: %s\n", SDL_GetError());
		return 1;
	}
	
	// OpenCV setup
	cv::VideoCapture cap("/dev/video2", cv::CAP_V4L2);

	if(!cap.isOpened()) {
		printf("Error: could not open camera\n");
		return 1;
	}

	// set resolution
	cap.set(cv::CAP_PROP_FRAME_WIDTH, 800);
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, 600);

	// Face Cascade setup
	cv::CascadeClassifier face_cascade;
	if (!face_cascade.load("/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml")){
		printf("Error: could not load face cascade\n");
		return 1;
	}
	printf("Face cascade loaded\n");


	// SDL Window/Renderer/Texture
	SDL_Window* win = SDL_CreateWindow("SDL Webcam", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);

	if (!win){
		printf("error creating window: %s\n", SDL_GetError());
		return 1;
	}
	printf("Window Created\n");

	SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer){
		printf("error creating renderer: %s\n", SDL_GetError());
		return 1;
	}

	// Bridge between SDL and OpenCV
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGR24, SDL_TEXTUREACCESS_STREAMING, 800, 600);
	if (!texture){
		printf("error creating texture: %s\n", SDL_GetError());
		return 1;
	}
	printf("Window Created %p\n", (void*)win);
	
	cv::Mat frame;
	
	int running = 1;
	SDL_Event event;
	while(running){
		 while(SDL_PollEvent(&event)){
			 if (event.type == SDL_QUIT) running = 0;
		 }

		 // grab frame
		 cap >> frame;
		 if (frame.empty()){
			 printf("empty frame, skipping\n");
			 continue;
		 }
		 
		 // 1. Convert to Grayscale
		 cv::Mat gray;
		 cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

		 // 2. Face detection
		 std::vector<cv::Rect> faces;
		 face_cascade.detectMultiScale(gray, faces, 1.1, 4, 0, cv::Size(30,30));

		 // 3. Apply Sobel to draw rectangles on detected faces
		 for (auto& face : faces){
			 // sobel steps
			 cv::Mat face_roi = gray(face);
			 cv::Mat sobel_x, sobel_y, sobel;
			 cv::Sobel(face_roi, sobel_x, CV_8U, 1, 0);
			 cv::Sobel(face_roi, sobel_y, CV_8U, 0, 1);
			 cv::addWeighted(sobel_x, 0.5, sobel_y, 0.5, 0, sobel);
			 sobel.copyTo(face_roi); // write sobel edges back into gray frame
			 cv::rectangle(gray, face, cv::Scalar(255), 2); // box around face 
		 }

		 // 4. Transmit facial recognition data
		 if (faces.size() > 0){
			 char msg[256];
			 snprintf(msg, sizeof(msg), "faces:=%zu timestamp=%lu\n", faces.size(), SDL_GetTicks());
			 send(client_fd, msg, strlen(msg),0);  // socket send
			 printf("Sent: %s", msg);
		 }

		 // 5. Convert back to BGR for SDL
		 cv::Mat gray_bgr;
		 cv::applyColorMap(gray, gray_bgr, cv::COLORMAP_JET);



		 //printf("Frame size: %d x %d\n", frame.cols, frame.rows);
		 //fflush(stdout);

		 // frames data in SDL format
		 SDL_UpdateTexture(texture, NULL, gray_bgr.data, gray_bgr.cols * 3);
		 SDL_RenderClear(renderer);
		 SDL_RenderCopy(renderer, texture, NULL, NULL);
		 SDL_RenderPresent(renderer);
	}


	close(client_fd);
	cap.release();	
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}
