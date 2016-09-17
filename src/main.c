/*
 * Copyright (C) 2010-2016 Andrea Zagli <azagli@libero.it>
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

#include <libzakauthe/libzakauthe.h>

static GtkBuilder *gtkbuilder;
static gchar *guidir;
static gchar *formdir;
static gchar *guifile;

static GKeyFile *config;

static GtkWidget *w;
static GtkWidget *vbx_body;
static GtkWidget *vbx_body_child;

static void
zak_authe_gui_set_vbx_body_child (GtkWidget *child)
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
zak_authe_gui_on_mnu_help_about_activate (GtkMenuItem *menuitem,
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

	ZakAuthe *aute;
	gchar **aute_params;
	gsize n_aute_params;
	GSList *sl_aute_params;

	gchar *utente;

	guint i;

	gtk_init (&argc, &argv);

	/* leggo la configurazione dal file */
	if (argc == 1)
		{
			g_error ("You need to provide the configuration file on command line.");
		}

	error = NULL;
	config = g_key_file_new ();
	if (!g_key_file_load_from_file (config, argv[1], G_KEY_FILE_NONE, &error))
		{
			g_error ("Unable to load the provided configuration file «%s»: %s.",
					 argv[1], error != NULL && error->message != NULL ? error->message : "no details");
		}

	/* parameters for authetication */
	error = NULL;
	aute_params = g_key_file_get_keys (config, "ZAKAUTHE", &n_aute_params, &error);
	if (aute_params == NULL)
		{
			g_error ("Unable to read configuration for authentication system: %s.",
					 error != NULL && error->message != NULL ? error->message : "no details");
		}

	sl_aute_params = NULL;
	for (i = 0; i < n_aute_params; i++)
		{
			error = NULL;
			sl_aute_params = g_slist_append (sl_aute_params, g_key_file_get_string (config, "ZAKAUTHE", aute_params[i], &error));
		}

	g_strfreev (aute_params);

	/* authentication */
	aute = zak_authe_new ();
	zak_authe_set_config (aute, sl_aute_params);

	while (TRUE)
		{
			utente = zak_authe_authe (aute);

			if (utente == NULL)
				{
					g_warning ("Wrong username or password.");
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

	guifile = g_build_filename (guidir, "zak-authe-gui.ui", NULL);

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

	GtkWidget *wgui = zak_authe_get_management_gui (aute);
	if (wgui != NULL)
		{
			zak_authe_gui_set_vbx_body_child (wgui);
		}
	else
		{
			g_warning ("Unable to get the GtkWidget for the users management from the selected libzakauthe plugin.");
		}

	gtk_widget_show (w);

	gtk_main ();

	return 0;
}
