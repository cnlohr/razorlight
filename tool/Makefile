all : razorlight.exe razorlight

CFLAGS:= -O0 -g $(shell pkg-config --cflags gtkmm-2.4 ) -Ihttp
LDFLAGS:=$(shell pkg-config --libs gtkmm-2.4 ) -g -lX11 -lm
CC:=gcc

WINCFLAGS:=-Os -Ihttp
WINLDFLAGS:=-lgdi32 -lws2_32 -ldwmapi -s  #-Wl,-subsystem,windows
WINCC:=i686-w64-mingw32-gcc

C_FILES:=myip.c razorlight.c tray.c os_generic.c http/cnhttp.c http/http_bsd.c http/mfs.c http/sha1.c custom_http.c urlopen.c parameters.c util.c windowworks.c led_manage.c ledsender.c

http/mfs_data.h:
	$(CC) -o http/mfsmaker http/mfsmaker.c
	cd http && ./mfsmaker ../page mpfs.dat

razorlight : $(C_FILES) http/mfs_data.h
	$(CC) $(CFLAGS) -o $@ $(C_FILES) $(LDFLAGS)

razorlight.exe : $(C_FILES) http/mfs_data.h
	$(WINCC) $(WINCFLAGS) -o $@ $(C_FILES) $(WINLDFLAGS)

clean :
	rm -rf *.o *~ razorlight.exe razorlight http/mfs_data.h http/mfsmaker mpfs.dat

