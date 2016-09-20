CFLAGS+=-g -Igen `pkg-config --cflags gtk+-3.0 cairo`
LDFLAGS+=-g `pkg-config --libs gtk+-3.0 cairo` -lm

main: main.o

main.o: gen/ui.glade.xml.h

D2H=./data_to_header_string/pack

check_d2h:
	cd data_to_header_string && ninja

$(D2H): check_d2h

gen/%.h: % $(D2H) | gen
	name=$* $(D2H) $< >$@.temp
	mv $@.temp $@

%.o: src/%.c
	gcc $(CFLAGS) -c -o $@ $<

gen:
	mkdir $@

.PHONY: check_d2h
