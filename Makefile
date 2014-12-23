PROJECT = audioprism

PREFIX ?= /usr
BINDIR = $(PREFIX)/bin

################################################################################

SRCS = audio/PulseAudioSource.cpp
SRCS += audio/WaveAudioSource.cpp
SRCS += dft/RealDft.cpp
SRCS += image/MagickImageSink.cpp
SRCS += spectrogram/SpectrumRenderer.cpp
SRCS += main/AudioThread.cpp
SRCS += main/SpectrogramThread.cpp
SRCS += main/InterfaceThread.cpp
SRCS += main/main.cpp

SRC_DIR = src
BUILD_DIR = build

SRCS := $(patsubst %.cpp,$(SRC_DIR)/%.cpp,$(SRCS))
OBJS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))

################################################################################

CXX = g++
REMOVE = rm -rf

CPPFLAGS = -std=c++11 -W -Wall -Wextra -Wconversion -pedantic -O3 -g -Isrc/
CPPFLAGS += $(shell sdl2-config --cflags)
CPPFLAGS += $(shell GraphicsMagick++-config --cppflags)

LDFLAGS = $(shell sdl2-config --libs) -lSDL2_ttf
LDFLAGS += -lpulse -lpulse-simple
LDFLAGS += -lfftw3
LDFLAGS += -lsndfile
LDFLAGS += $(shell GraphicsMagick++-config --libs)

################################################################################

.PHONY: all
all: $(PROJECT)

.PHONY: beautiful
beautiful:
	find src \( -name "*.cpp" -o -name "*.hpp" \) | xargs clang-format -i

.PHONY: install
install: $(PROJECT)
	install -D -s -m 0755 $(PROJECT) $(DESTDIR)$(BINDIR)/$(PROJECT)

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(PROJECT)

.PHONY: clean
clean:
	$(REMOVE) $(BUILD_DIR)
	$(REMOVE) $(PROJECT)

################################################################################

$(PROJECT): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) -c $< -o $@

