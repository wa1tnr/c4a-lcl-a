CFLAGS = -O3 -m32
srcfiles := $(shell find . -name "*.cpp")
incfiles := $(shell find . -name "*.h")

c4a: $(srcfiles) $(incfiles)
	$(CC) $(CFLAGS) $(srcfiles) -DIS_LINUX=1 -o $@

run: c4a
	./c4a

clean:
	rm -f c4a

test: c4a
	./c4a block-200.fth

bin: c4a
	cp -u -p c4a ~/bin/