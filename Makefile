PREFIX	:= $(DESTDIR)/usr/local
INCDIR	:= $(PREFIX)/include

SRCDIR	:= ./source
HEADERS	:= $(shell find -wholename "$(SRCDIR)/*.hpp" && find -wholename "$(SRCDIR)/*.h")
INCLUDE	:= $(HEADERS:$(SRCDIR)/%=$(INCDIR)/%)
INCDIRS	:= $(shell dirname $(INCLUDE))

default:
	@echo This is a pure header library, so there is nothing to build. You can still install/uninstall.

install: $(INCLUDE)
	@echo Install complete!

$(INCDIR)/%.h: $(SRCDIR)/%.h
	@mkdir -p $(@D)
	cp $< $@

$(INCDIR)/%.hpp: $(SRCDIR)/%.hpp
	@mkdir -p $(@D)
	cp $< $@

uninstall:
	-rm $(INCLUDE)
	@rmdir -p $(INCDIRS) 2> /dev/null || true
	@echo Uninstall complete!

.PHONY : install uninstall
