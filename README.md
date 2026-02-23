# OpenCV-Thermal-FacialRecognition

A real-time facial detection system built in C++ that combines SDL2 for windowing, OpenCV for computer vision, and a lightweight TCP socket server for logging detection events. The webcam feed is rendered in grayscale with a thermal colormap overlay and bounding boxes drawn around detected faces.

## Features

- Real-time webcam capture via V4L2
- Grayscale and thermal imaging rendering modes
- Haar cascade face detection with bounding box overlay
- TCP socket client that logs face detection events to a server
- Detection events logged to a file with timestamps
- SDL2 hardware accelerated rendering

## Dependencies

- SDL2
- OpenCV 4.x
- g++ with C++17 support
- Linux with V4L2 (Video4Linux2)

On Arch Linux:
```bash
sudo pacman -S sdl2 opencv
```

On Ubuntu/Debian:
```bash
sudo apt install libsdl2-dev libopencv-dev
```

## Building

```bash
make
```

This will compile both the camera client (`window`) and the socket server (`server`).

To clean build artifacts:
```bash
make clean
```

## Usage

**1. Start the server first in one terminal:**
```bash
./server
```

**2. Start the camera client in a second terminal:**
```bash
./window
```

The server will log face detection events to `detections.log` in the format:
```
faces=1 timestamp=3521
faces=2 timestamp=3604
```

## Project Structure

```
.
├── window.cpp      # Main application - webcam capture, face detection, SDL rendering, socket client
├── server.c        # TCP socket server - receives and logs detection events
├── Makefile        # Builds both binaries
└── detections.log  # Generated at runtime - face detection event log
```

## Configuration

**Camera device** — change the device path in `window.cpp` if your webcam is not at `/dev/video2`:
```cpp
cv::VideoCapture cap("/dev/video2", cv::CAP_V4L2);
```

**Resolution** — default is 800x600, change in `window.cpp` and make sure the SDL window and texture sizes match:
```cpp
cap.set(cv::CAP_PROP_FRAME_WIDTH, 800);
cap.set(cv::CAP_PROP_FRAME_HEIGHT, 600);
```

**Thermal colormap** — swap the colormap in `window.cpp` to change the visual style:
```cpp
cv::applyColorMap(gray, gray_bgr, cv::COLORMAP_JET);    // classic thermal
cv::applyColorMap(gray, gray_bgr, cv::COLORMAP_INFERNO); // cinematic
cv::applyColorMap(gray, gray_bgr, cv::COLORMAP_HOT);     // realistic thermal
```

**Port** — default is 54321, change the `PORT` define in both `window.cpp` and `server.c` to match.

## How It Works

1. OpenCV captures frames from the webcam via V4L2
2. Each frame is converted to grayscale
3. A Haar cascade classifier scans the frame for faces
4. Bounding boxes are drawn around any detected faces
5. A thermal colormap is applied to the grayscale frame
6. The frame is uploaded to an SDL texture and rendered to the window
7. When faces are detected, a message is sent over TCP to the server with the face count and timestamp
8. The server logs each message to `detections.log`

## Notes

- Make sure to start the server before the camera client or the connection will fail
- The Haar cascade file is loaded from `/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml` — this path may differ on non-Arch systems
- On Wayland/Hyprland set `SDL_VIDEODRIVER=x11` if the window does not appear
