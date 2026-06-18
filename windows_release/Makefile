# OS-Level Anti-Detect System Makefile

CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pthread
CXXFLAGS += -I.
TARGET = antidetect
SOURCES = core/main.cpp core/system_fingerprint.cpp core/advanced_fingerprint.cpp core/android_spoof.cpp
HEADERS = core/system_fingerprint.h core/advanced_fingerprint.h core/android_spoof.h

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/
	@echo "Installed to /usr/local/bin/$(TARGET)"

uninstall:
	rm -f /usr/local/bin/$(TARGET)

.PHONY: all clean install uninstall
