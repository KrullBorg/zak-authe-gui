guidir = $(datadir)/utenti/gui
formdir = $(datadir)/utenti/form

AM_CPPFLAGS = -I.. \
              $(UTENTI_CFLAGS) \
              -DGUIDIR=\""$(guidir)"\" \
              -DFORMDIR=\""$(formdir)"\"

LIBS = $(UTENTI_LIBS) \
       -export-dynamic

bin_PROGRAMS = utenti

utenti_SOURCES = main.c
