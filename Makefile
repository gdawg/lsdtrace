SRC := $(wildcard *.c)
OUT := ${SRC:.c=}

CFLAGS := 
LDFLAGS := -ldtrace

# add warnings... ALL the warnings
CFLAGS += -Wall -Werror -Wextra
# all the speed
CFLAGS += -O3

all: $(OUT)

%: %.c
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(OUT)
