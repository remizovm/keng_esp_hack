all: keng_esp_hack

keng_esp_hack: esp.o
	gcc esp.o -o keng_esp_hack -lpsapi

esp.o: esp.c
	gcc -c esp.c

clean:
	rm -rf *.o keng_esp_hack.exe
