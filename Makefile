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

output.bc: target/stage1 example.zr
	./$< ./example.zr

output.ll: output.bc
	llvm-dis $<

test_output: output.bc
	clang $< -o $@
target:
	mkdir -p target

clean:
	rm -rf target *.bc *.ll test_output
