CC = mpicc

TARGET = build/mandelbrot
SRC = src/main.c

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p build
	$(CC) -O3 $(SRC) -o $(TARGET)

clean:
	rm -rf build

.PHONY: all clean
