
default:	build

clean:
	rm -rf Makefile objs

build:
	$(MAKE) -f objs/Makefile -j 4

install:
	$(MAKE) -f objs/Makefile install

