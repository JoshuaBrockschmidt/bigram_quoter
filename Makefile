NAME=bigram_quoter
CC=g++
RM=rm
SOURCES  := $(wildcard src/*.cpp)
OBJS     := $(SOURCES:.cpp=.o)
INCLUDES := -Iinclude
WARNFLAGS   := -Wall -Wextra -Wshadow -Wcast-align -Wwrite-strings -Winline
WARNFLAGS   += -Wno-attributes -Wno-deprecated-declarations
WARNFLAGS   += -Wno-div-by-zero -Wno-endif-labels -Wfloat-equal
WARNFLAGS   += -Wformat=2 -Wno-format-extra-args -Winit-self
WARNFLAGS   += -Winvalid-pch -Wmissing-format-attribute
WARNFLAGS   += -Wmissing-include-dirs -Wno-multichar -Wshadow
WARNFLAGS   += -Wno-sign-compare -Wswitch
WARNFLAGS   += -Wno-pragmas -Wno-unused-but-set-parameter
WARNFLAGS   += -Wno-unused-but-set-variable -Wno-unused-result
WARNFLAGS   += -Wwrite-strings -Wdisabled-optimization -Wpointer-arith
CPPFLAGS := $(INCLUDES) $(WARNFLAGS) -std=c++11

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CPPFLAGS) $(OBJS) -o $(NAME)

src/argparser.o: include/showhelp.h

include/showhelp.h: help
# Use sed to transform plaintext helpfile
# into escaped version for printing.
# Thanks to esr for the trick.
	sed <help >include/showhelp.h \
		-e 's/\\/\\\\/g' -e 's/"/\\"/g' \
		-e 's/.*/std::cerr << "&" << std::endl;/g'

debug: CPPFLAGS += -g
debug: all

clean:
	$(RM) $(NAME) $(OBJS)
