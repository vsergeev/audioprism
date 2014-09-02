PROJECT = spectrogram

################################################################################

SRCS = PulseAudioSource.cpp
SRCS += RealDft.cpp
SRCS += Spectrogram.cpp
SRCS += AudioThread.cpp
SRCS += SpectrogramThread.cpp
SRCS += InterfaceThread.cpp
SRCS += spectrogram.cpp

OBJS = $(patsubst %.cpp,%.o,$(SRCS))

################################################################################

CXX = g++
REMOVE = rm -f

CPPFLAGS = -std=c++11 -W -Wall -Wextra -pedantic
CPPFLAGS += $(shell sdl2-config --cflags)

LDFLAGS = $(shell sdl2-config --libs)
LDFLAGS += -lpulse -lpulse-simple
LDFLAGS += -lfftw3

################################################################################

all: $(PROJECT)

clean:
	$(REMOVE) $(OBJS) $(PROJECT)

################################################################################

$(PROJECT): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

