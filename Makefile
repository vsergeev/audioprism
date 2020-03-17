PROJECT = audioprism

PREFIX ?= /usr
BINDIR = $(PREFIX)/bin

################################################################################

SRC_DIR = src
BUILD_DIR = build

SRCS = $(shell find $(SRC_DIR)/ -type f -name "*.cpp")
OBJS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))

################################################################################

REMOVE = rm -rf

CPPFLAGS += -std=c++11 -W -Wall -Wextra -Wconversion -pedantic -O3 -g -Isrc/
CPPFLAGS += $(shell pkg-config --cflags libpulse libpulse-simple fftw3f sndfile sdl2 SDL2_ttf GraphicsMagick++)

LDFLAGS += $(shell pkg-config --libs libpulse libpulse-simple fftw3f sndfile sdl2 SDL2_ttf GraphicsMagick++)
LDFLAGS +=  -lpthread

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

.PHONY: analyze
analyze:
	clang --analyze -Xanalyzer -analyzer-output=text $(CPPFLAGS) $(SRCS)

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

