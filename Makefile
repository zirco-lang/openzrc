CC=clang
CFLAGS= -Iinclude $(CEXTS)


ZRC ?= $(shell which zrc)
ZRCROOT ?= $(shell dirname $(ZRC))/..
ZRCLIB ?= $(ZRCROOT)/include
ZRFLAGS ?= -I${ZRCLIB} -I./include $(ZREXTS)
OUTPUT=main
ifeq ($(OS),Windows_NT)
    OUTPUT = target/zrc.exe
endif

OUTDIR ?= ./target

ZR_SOURCES ?= $(wildcard src/*.zr)
ZR_OUTPUTS ?= $(ZR_SOURCES:src/%.zr=$(OUTDIR)/%.o)
C_SOURCES ?= $(wildcard src/*.c)
C_OUTPUTS ?= $(C_SOURCES:src/%.c=$(OUTDIR)/%.o)

.PHONY: all clean main test interrupt


all: clean $(OUTPUT)
test: CEXTS = -DEXT_INT


interrupt: CEXTS = -DEXT_INT
interrupt: all

$(OUTDIR)/zrc.exe: main
	@cp $(OUTDIR)/zrc $(OUTDIR)/zrc.exe
	@echo "Created exe file"

main: target $(OUTDIR)/zrc

target:
	@mkdir -p $(OUTDIR)

$(OUTDIR)/%.o: src/%.c
	@$(CC) $(CFLAGS) -o $@ -c $<
	@echo "Compiled $@ from $<"

$(OUTDIR)/%.o: src/%.zr
	cp $< tmp.zr
	$(ZRC) --emit object $(ZRFLAGS) -o $@ tmp.zr
	rm tmp.zr
	@echo "Compiled $@ from $<"

$(OUTDIR)/zrc: target $(C_OUTPUTS) $(ZR_OUTPUTS)
	@$(CC) $(CFLAGS) -lc -o $@ $(C_OUTPUTS) $(ZR_OUTPUTS)
	@echo "Compiled: $@ from $(C_OUTPUTS) $(ZR_OUTPUTS)"

clean:
	@rm -rf $(OUTDIR)
	@echo "Cleaned all files!"

