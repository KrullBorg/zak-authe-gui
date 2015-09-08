guidir = $(datadir)/$(PKGNAME)/gui
formdir = $(datadir)/$(PKGNAME)/form

AM_CPPFLAGS = -I.. \
              $(ZAK_AUTHE_GUI_CFLAGS) \
              -DGUIDIR=\""$(guidir)"\" \
              -DFORMDIR=\""$(formdir)"\"

LIBS = $(ZAK_AUTHE_GUI_LIBS) \
       -export-dynamic

bin_PROGRAMS = zak-authe-gui

zak_authe_gui_SOURCES = main.c
