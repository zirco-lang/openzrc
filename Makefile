CC=clang
CFLAGS=-g `llvm-config --cflags`
LD=clang++
LDFLAGS=`llvm-config --cxxflags --ldflags --libs core executionengine mcjit interpreter analysis native bitwriter --system-libs`
.PHONY: all clean target

all: target target/zrc

target/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

target/zrc: target/stage1
	cp $< $@

target/stage1: target/stage1.o
	$(LD) $< $(LDFLAGS) -o $@

clean:
	rm -rf target
