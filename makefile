# Simple Makefile for rapid-trigger filters
CC      ?= gcc
CFLAGS  ?= -O2 -Wall -Wextra -std=c11
PREFIX  ?= /usr/local
BINDIR  ?= $(PREFIX)/bin

# Default: build the A/D filter (adjust if you prefer another default)
all: movement_rapid_trigger

# Optional: build these only if you have the sources
movement_rapid_trigger: movement_rapid_trigger.c
	$(CC) $(CFLAGS) -o $@ $<

install-movement: movement_rapid_trigger
	install -Dm755 movement_rapid_trigger $(DESTDIR)$(BINDIR)/movement_rapid_trigger

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/movement_rapid_trigger \
	       

clean:
	rm -f movement_rapid_trigger

# Convenience runner (needs root for intercept/uinput)
DEV ?= /dev/input/by-id/usb-Ducky_Akko_Keyboard-event-kbd
run: movement_rapid_trigger
	@echo "Using keyboard device: $(DEV)"
	sudo intercept -g "$(DEV)" | ./movement_rapid_trigger | sudo uinput -d "$(DEV)"

.PHONY: all install install-movement install-global uninstall clean run
