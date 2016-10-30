all : ambilight.exe

CFLAGS:=-fdata-sections -Wl,--gc-sections -Os -s 

ambilight.exe : ambilight.c os_generic.c
	i686-w64-mingw32-gcc $(CFLAGS) -o $@ $^ -lgdi32
	cp $@ /win/pub/
#	wine upx $@

#direct2d.exe : direct2d.c os_generic.c
#	i686-w64-mingw32-gcc -Os -s -o $@ $^ -lgdi32
	

clean :
	rm -rf *.exe *~
