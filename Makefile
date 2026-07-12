
.PHONY: all clean target

all: target/zrc

target/stage1: stage1.c target
	$(CC) $< -o $@

target:
	mkdir -p target

target/zrc: target/stage1
	cp $< $@

clean:
	rm -rf target
