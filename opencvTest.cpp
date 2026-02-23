#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <opencv2/opencv.hpp>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t ui8;
typedef uint16_t ui16;
typedef uint32_t ui32;
typedef uint64_t ui64;

int main(int argc, char* const argv[]){
	printf("Starting Main...\n");
	fflush(stdout);
	cv::VideoCapture cap(0);
	if(!cap.isOpened()) {
		printf("Error: could not open camera\n");
		return 1;
	}

	// set resolution
	cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

	cv::Mat frame;

	while(1){
		cap >> frame; // output frame

		if (frame.empty()){
			printf("Error: empty frame\n");
			break;
		}

		cv::imshow("Webcam", frame); // cv2.imshow() equivalent
		if (cv::waitKey(1) == 'q') break;
	}

	cap.release();
	cv::destroyAllWindows();
	
	return 0;
}
