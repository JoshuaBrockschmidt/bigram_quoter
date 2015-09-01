NAME=bigram_quoter
CC=g++
RM=rm
SOURCES  := $(wildcard src/*.cpp)
OBJS     := $(SOURCES:.cpp=.o)
INCLUDES := -Iinclude
CPPFLAGS := $(INCLUDES) -std=c++11

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CPPFLAGS) $(OBJS) -o $(NAME)

clean:
	$(RM) $(NAME) $(OBJS)
