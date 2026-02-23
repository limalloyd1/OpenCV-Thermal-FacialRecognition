CC := g++

CFLAGS := $(shell sdl2-config --cflags) -I/usr/include/opencv4 -Wall -ggdb3 --std=c++17
LDFLAGS := $(shell sdl2-config --libs) -lopencv_core -lopencv_videoio -lopencv_imgproc -lopencv_objdetect
SRCS := window.cpp

OBJS := $(SRCS:.cpp=.o)
EXEC := window

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(EXEC) $(OBJS)

.PHONY: all clean
