CC=clang
CFLAGS=-g `llvm-config --cflags`
LD=clang++
LDFLAGS=`llvm-config --cxxflags --ldflags --libs core executionengine mcjit interpreter analysis native bitwriter --system-libs`

STAGE1OBJ = target/s1_stage1.o target/s1_lexer.o target/s1_parser.o target/s1_codegen.o

.PHONY: all clean target

all: target target/zrc

target/s1_%.o: stage1/%.c
	$(CC) $(CFLAGS) -c $< -o $@

target/zrc: target/stage1
	cp $< $@

target/stage1: $(STAGE1OBJ)
	$(LD) $(STAGE1OBJ) $(LDFLAGS) -o $@

output.bc: clean target target/stage1 example.zr
	./target/stage1 ./example.zr

output.ll: output.bc
	llvm-dis $<

a.out: output.bc
	clang $< -o $@
target:
	mkdir -p target

clean:
	rm -rf target *.bc *.ll a.out
