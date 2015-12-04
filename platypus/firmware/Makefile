# memos: $@ file name of the target of the rule
#	 $^ all dependencies of this rule
#	 $< the first dependency
#	 .PRECIOUS targets marked with this are not deleted when make is killed

CXX=g++
CFLAGS=-g -Wall -std=c++0x -Iinclude -IGrLib/grlib -ILcdDriver -lmraa# -I/usr/include/mraa -lupm-i2clcd -I/usr/include/upm
COPTS=-pthread
LOPTS=-pthread

SOURCES = src/platypus.cpp \
					src/imu_edison.cpp \
					src/display_edison.cpp \
					src/mcu_edison.cpp \
					src/batgauge_edison.cpp \
					src/ldc_edison.cpp \
					LcdDriver/HAL_MSP_EXP430FR5969_Sharp96x96.c \
					LcdDriver/Sharp96x96.c \
					GrLib/fonts/fontfixed6x8.c \
					GrLib/grlib/context.c \
					GrLib/grlib/display.c \
					GrLib/grlib/line.c \
					GrLib/grlib/circle.c \
					GrLib/grlib/rectangle.c \
					GrLib/grlib/string.c

MAIN_BINARIES=$(addprefix bin/,$(basename $(notdir $(wildcard src/*Main.cpp))))
OBJECTS = $(subst src/,build/,$(subst .cpp,.o,$(SOURCES)))


.PRECIOUS: build/%.o

all: compile

compile: dirs $(SOURCES) $(MAIN_BINARIES)

bin/%Main: $(OBJECTS) build/%Main.o
	$(CXX) $(CFLAGS) -o $@ $^ $(LOPTS)

build/%.o: src/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@ $(COPTS)



dirs:
	mkdir -p bin build

checkstyle:
	python2 cpplint.py --filter=-runtime/references,-whitespace/comments,-readability/streams --linelength=200 *.cpp

.PHONY:
	clean

clean:
	rm -rf bin build
	rm -f *~
	rm -f */*~
