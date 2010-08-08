/*
 * Copyright (C) 2010 Andrea Zagli <azagli@libero.it>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include <gtk/gtk.h>

#include <libaute.h>

static GtkBuilder *gtkbuilder;
static gchar *guidir;
static gchar *formdir;
static gchar *guifile;

static GKeyFile *config;

static GtkWidget *w;
static GtkWidget *vbx_body;
static GtkWidget *vbx_body_child;

static void
utenti_set_vbx_body_child (GtkWidget *child)
{
	if (GTK_IS_WIDGET (vbx_body_child))
		{
			gtk_container_remove (GTK_CONTAINER (vbx_body), vbx_body_child);
			gtk_widget_destroy (vbx_body_child);
		}

	vbx_body_child = child;
	gtk_box_pack_start (GTK_BOX (vbx_body), vbx_body_child, TRUE, TRUE, 0);
}

G_MODULE_EXPORT void
utenti_on_mnu_help_about_activate (GtkMenuItem *menuitem,
                            gpointer user_data)
{
	GError *error;
	GtkWidget *diag;

	error = NULL;
	gtk_builder_add_objects_from_file (gtkbuilder, guifile,
	                                   g_strsplit_set ("dlg_about", "|", -1),
	                                   &error);
	if (error != NULL)
		{
			g_error ("Errore: %s.", error->message);
		}

	diag = GTK_WIDGET (gtk_builder_get_object (gtkbuilder, "dlg_about"));

	gtk_dialog_run (GTK_DIALOG (diag));
	gtk_widget_destroy (diag);
}

int
main (int argc, char **argv)
{
	GError *error;

	Aute *aute;
	gchar **aute_params;
	gsize n_aute_params;
	GSList *sl_aute_params;

	gchar *utente;

	gchar *cnc_string;

	guint i;

	gtk_init (&argc, &argv);

	/* leggo la configurazione dal file */
	if (argc == 1)
		{
			g_error ("Occorre passare a riga di comando il file di configurazione.");
		}

	error = NULL;
	config = g_key_file_new ();
	if (!g_key_file_load_from_file (config, argv[1], G_KEY_FILE_NONE, &error))
		{
			g_error ("Impossibile caricare il file di configurazione specificato: %s.", argv[1]);
		}

	/* leggo i parametri per l'autenticazione */
	error = NULL;
	aute_params = g_key_file_get_keys (config, "AUTE", &n_aute_params, &error);
	if (aute_params == NULL)
		{
			g_error ("Impossibile leggere la configurazione per il sistema di autenticazione.");
		}

	sl_aute_params = NULL;
	for (i = 0; i < n_aute_params; i++)
		{
			error = NULL;
			sl_aute_params = g_slist_append (sl_aute_params, g_key_file_get_string (config, "AUTE", aute_params[i], &error));
		}

	g_strfreev (aute_params);

	/* autenticazione */
	aute = aute_new ();
	aute_set_config (aute, sl_aute_params);

	while (TRUE)
		{
			utente = aute_autentica (aute);

			if (utente == NULL)
				{
					g_warning ("Nome utente o password non validi.");
				}
			else if (g_strcmp0 (utente, "") == 0)
				{
					return 0;				
				}
			else
				{
					break;
				}
		}

#ifdef G_OS_WIN32

#undef GUIDIR
	guidir = g_build_filename (g_win32_get_package_installation_directory_of_module (NULL), "share", PACKAGE, "gui", NULL);

#undef FORMDIR
	formdir = g_build_filename (g_win32_get_package_installation_directory_of_module (NULL), "share", PACKAGE, "form", NULL);

#else

	guidir = g_strdup (GUIDIR);
	formdir = g_strdup (FORMDIR);

#endif

	guifile = g_build_filename (guidir, "utenti.gui", NULL);

	gtkbuilder = gtk_builder_new ();

	error = NULL;
	gtk_builder_add_objects_from_file (gtkbuilder, guifile,
	                                   g_strsplit_set ("w_main", "|", -1),
	                                   &error);
	if (error != NULL)
		{
			g_error ("Errore: %s", error->message);
		}

	gtk_builder_connect_signals (gtkbuilder, NULL);

	w = GTK_WIDGET (gtk_builder_get_object (gtkbuilder, "w_main"));

	vbx_body = GTK_WIDGET (gtk_builder_get_object (gtkbuilder, "vbx_body"));

	vbx_body_child = NULL;

	gtk_widget_show (w);

	gtk_main ();

	return 0;
}
