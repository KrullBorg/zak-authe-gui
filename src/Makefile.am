gladedir = $(datadir)/utenti/glade

AM_CPPFLAGS = -I.. \
              $(UTENTI_CFLAGS) \
              -DGLADEDIR=\""$(gladedir)"\"

LIBS = $(UTENTI_LIBS) \
       -export-dynamic

bin_PROGRAMS = utenti

utenti_SOURCES = main.c
