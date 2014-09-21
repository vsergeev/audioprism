PROJECT = spectrogram

################################################################################

SRCS = PulseAudioSource.cpp
SRCS += RealDft.cpp
SRCS += Spectrogram.cpp
SRCS += AudioThread.cpp
SRCS += SpectrogramThread.cpp
SRCS += InterfaceThread.cpp
SRCS += WaveAudioSource.cpp
SRCS += MagickImageSink.cpp
SRCS += main.cpp

SRC_DIR = src
BUILD_DIR = build

SRCS := $(patsubst %.cpp,$(SRC_DIR)/%.cpp,$(SRCS))
OBJS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))

################################################################################

CXX = g++
REMOVE = rm -rf

CPPFLAGS = -std=c++11 -W -Wall -Wextra -pedantic -O3 -g
CPPFLAGS += $(shell sdl2-config --cflags)
CPPFLAGS += $(shell GraphicsMagick++-config --cppflags)

LDFLAGS = $(shell sdl2-config --libs) -lSDL2_ttf
LDFLAGS += -lpulse -lpulse-simple
LDFLAGS += -lfftw3
LDFLAGS += -lsndfile
LDFLAGS += $(shell GraphicsMagick++-config --libs)

################################################################################

all: $(PROJECT)

clean:
	$(REMOVE) $(BUILD_DIR)
	$(REMOVE) $(PROJECT)

################################################################################

$(PROJECT): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) -c $< -o $@

