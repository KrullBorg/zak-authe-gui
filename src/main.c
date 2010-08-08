/*
 * Copyright (C) 2006 Andrea Zagli <azagli@inwind.it>
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
#include <glade/glade.h>

#include <libsolipa.h>
#include <libaute.h>
#include <libconfi.h>
#include <libgdaobj.h>

#include <libform.h>
#include <libformkey.h>
#include <libformfieldboolean.h>
#include <libformfielddatetime.h>
#include <libformfieldinteger.h>
#include <libformfieldtext.h>
#include <libformwidgetcheck.h>
#include <libformwidgetentry.h>
#include <libformwidgetlabel.h>
#include <libformwidgettextview.h>

enum
{
	COL_STATO,
	COL_COD,
	COL_COGNOME,
	COL_NOME,
	UTENTI_COLS
};

static GtkWidget *w,
                 *tr_utenti,
                 *txt_codice,
                 *txt_password,
                 *txt_conferma_password,
                 *btn_ok_utente;

static GtkListStore *store_utenti;

static GtkTreeSelection *sel_utenti;

static GladeXML *gla_utente;
static GtkWidget *diag_utente;
static Form *frm_utente;

static const SOLIPA *solipa;
static GdaO *gdao;

/* PRIVATE */
static gboolean
crea_tr_utenti ()
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	store_utenti = gtk_list_store_new (UTENTI_COLS,
	                                   G_TYPE_STRING,
	                                   G_TYPE_STRING,
	                                   G_TYPE_STRING,
	                                   G_TYPE_STRING);

	gtk_tree_view_set_model (GTK_TREE_VIEW (tr_utenti), GTK_TREE_MODEL (store_utenti));

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Stato",
                                                     renderer,
                                                     "text", COL_STATO,
                                                     NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tr_utenti), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Codice",
                                                     renderer,
                                                     "text", COL_COD,
                                                     NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tr_utenti), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Cognome",
                                                     renderer,
                                                     "text", COL_COGNOME,
                                                     NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tr_utenti), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Nome",
                                                     renderer,
                                                     "text", COL_NOME,
                                                     NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tr_utenti), column);

	sel_utenti = gtk_tree_view_get_selection (GTK_TREE_VIEW (tr_utenti));
}

static gboolean
riempi_tr_utenti ()
{
	gtk_list_store_clear (store_utenti);

	GdaDataModel *dm = gdao_query (gdao, "SELECT codice, cognome, nome, abilitato, stato "
	                                     "FROM utenti");
	if (dm != NULL)
		{
			GtkTreeIter iter;
			gint row;
			gint rows = gda_data_model_get_n_rows (dm);
			gchar *stato;

			for (row = 0; row < rows; row++)
				{
					if (strcmp (gdao_data_model_get_field_value_stringify_at (dm, row, "stato"), "E") == 0)
						{
							stato = g_strdup ("E");
						}
					else if (!gdao_data_model_get_field_value_boolean_at (dm, row, "abilitato"))
						{
							stato = g_strdup ("D");
						}
					else
						{
							stato = g_strdup ("");
						}
				
					gtk_list_store_append (store_utenti, &iter);
					gtk_list_store_set (store_utenti, &iter,
					                    COL_STATO, stato,
					                    COL_COD, gdao_data_model_get_field_value_stringify_at (dm, row, "codice"),
					                    COL_COGNOME, gdao_data_model_get_field_value_stringify_at (dm, row, "cognome"),
					                    COL_NOME, gdao_data_model_get_field_value_stringify_at (dm, row, "nome"),
					                    -1);
				}
		}
}

/* CALLBACKS */
void
on_mnu_utenti_nuovo_activate (GtkMenuItem *menuitem,
                              gpointer user_data)
{
	form_clear (frm_utente);

	while (TRUE)
		{
			if (gtk_dialog_run (GTK_DIALOG (diag_utente)) == GTK_RESPONSE_OK)
				{
					if (form_check (frm_utente))
						{
							gchar *password;
							gchar *password_conferma;

							password = g_strdup (gtk_entry_get_text (GTK_ENTRY (txt_password)));
							password_conferma = g_strdup (gtk_entry_get_text (GTK_ENTRY (txt_conferma_password)));

							g_strstrip (password);
							g_strstrip (password_conferma);

							if (strcmp (password, "") != 0
							    && strcmp (password, password_conferma) == 0)
								{
									gchar *sql;

									sql = form_get_sql (frm_utente, FORM_SQL_INSERT);
									if (gdao_execute (gdao, sql) < 0)
										{
											g_warning ("Errore nell'inserimento del nuovo utente.");
										}
									else
										{
											gchar *codice;
											GModule *module;

											codice = g_strdup (gtk_entry_get_text (GTK_ENTRY (txt_codice)));
											g_strstrip (codice);

											/* carico il modulo per l'autenticazione */
											module = aute_plugin_get_module (solipa->confi);
											if (module != NULL)
												{
													gboolean (*crea_utente) (Confi *confi, const gchar *codice, const gchar *password);

													if (!g_module_symbol (module, "crea_utente", (gpointer *)&crea_utente))
														{
															g_warning ("Errore nel caricamento della funzione del plugin di autenticazione.");
														}
													else
														{
															(*crea_utente) (solipa->confi, codice, password);
														}
												}

											riempi_tr_utenti ();
										}
									break;
								}
							else
								{
									GtkWidget *diag;

									diag = gtk_message_dialog_new (GTK_WINDOW (diag_utente),
																								 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
																								 GTK_MESSAGE_WARNING,
																								 GTK_BUTTONS_OK,
																								 "La password o la sua conferma sono vuote o non coincidono.");
									gtk_dialog_run (GTK_DIALOG (diag));
									gtk_widget_destroy (diag);
									gtk_widget_grab_focus (txt_password);
								}
						}
					else
						{
							GtkWidget *diag;

							diag = gtk_message_dialog_new (GTK_WINDOW (diag_utente),
							                               GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
							                               GTK_MESSAGE_WARNING,
							                               GTK_BUTTONS_OK,
							                               "Inserire tutti i campi obbligatori.");
							gtk_dialog_run (GTK_DIALOG (diag));
							gtk_widget_destroy (diag);
						}
				}
			else
				{
					/* GTK_RESPONSE_CANCEL */
					break;
				}
		}
	gtk_widget_hide (diag_utente);
}

void
on_mnu_utenti_modifica_activate (GtkMenuItem *menuitem,
                                 gpointer user_data)
{
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected (sel_utenti, NULL, &iter))
		{
			gchar *codice;
			gchar *sql;
			gchar *stato;
			GdaDataModel *dm;

			gtk_tree_model_get (GTK_TREE_MODEL (store_utenti), &iter,
													COL_COD, &codice,
			                    COL_STATO, &stato,
													-1);

			gtk_entry_set_text (GTK_ENTRY (txt_codice), codice);
			gtk_editable_set_editable (GTK_EDITABLE (txt_codice), FALSE);

			sql = form_get_sql (frm_utente, FORM_SQL_SELECT);
			dm = gdao_query (gdao, sql);
			form_clear (frm_utente);
			form_fill_from_datamodel (frm_utente, dm, 0);

			if (strcmp (stato, "E") == 0)
				{
					gtk_widget_set_sensitive (btn_ok_utente, FALSE);
				}
			else
				{
					gtk_widget_set_sensitive (btn_ok_utente, TRUE);
				}

			while (TRUE)
				{
					if (gtk_dialog_run (GTK_DIALOG (diag_utente)) == GTK_RESPONSE_OK)
						{
							if (form_check (frm_utente))
								{
									gchar *password;
									gchar *password_conferma;

									password = g_strdup (gtk_entry_get_text (GTK_ENTRY (txt_password)));
									password_conferma = g_strdup (gtk_entry_get_text (GTK_ENTRY (txt_conferma_password)));

									g_strstrip (password);
									g_strstrip (password_conferma);

									if (strcmp (password, "") != 0
											&& strcmp (password, password_conferma) == 0)
										{
											gchar *sql;

											sql = form_get_sql (frm_utente, FORM_SQL_UPDATE);
											if (gdao_execute (gdao, sql) < 0)
												{
													g_warning ("Errore nella modifica dell'utente.");
												}
											else
												{
													gchar *codice;
													GModule *module;

													codice = g_strdup (gtk_entry_get_text (GTK_ENTRY (txt_codice)));
													g_strstrip (codice);

													/* carico il modulo per l'autenticazione */
													module = aute_plugin_get_module (solipa->confi);
													if (module != NULL)
														{
															gboolean (*modifica_utente) (Confi *confi, const gchar *codice, const gchar *password);
		
															if (!g_module_symbol (module, "modifica_utente", (gpointer *)&modifica_utente))
																{
																	g_warning ("Errore nel caricamento della funzione del plugin di autenticazione.");
																}
															else
																{
																	(*modifica_utente) (solipa->confi, codice, password);
																}
														}

													riempi_tr_utenti ();
												}
											break;
										}
									else
										{
											GtkWidget *diag;

											diag = gtk_message_dialog_new (GTK_WINDOW (diag_utente),
																										 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
																										 GTK_MESSAGE_WARNING,
																										 GTK_BUTTONS_OK,
																										 "La password o la sua conferma sono vuote o non coincidono.");
											gtk_dialog_run (GTK_DIALOG (diag));
											gtk_widget_destroy (diag);
											gtk_widget_grab_focus (txt_password);
										}
								}
							else
								{
									GtkWidget *diag;

									diag = gtk_message_dialog_new (GTK_WINDOW (diag_utente),
																								 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
																								 GTK_MESSAGE_WARNING,
																								 GTK_BUTTONS_OK,
																								 "Inserire tutti i campi obbligatori.");
									gtk_dialog_run (GTK_DIALOG (diag));
									gtk_widget_destroy (diag);
								}
						}
					else
						{
							/* GTK_RESPONSE_CANCEL */
							break;
						}
				}

			gtk_editable_set_editable (GTK_EDITABLE (txt_codice), TRUE);
			gtk_widget_hide (diag_utente);
		}
	else
		{
			/* TO DO */
		}
}

void
on_mnu_utenti_elimina_activate (GtkMenuItem *menuitem,
                                gpointer user_data)
{
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected (sel_utenti, NULL, &iter))
		{
			GtkWidget *diag = gtk_message_dialog_new (GTK_WINDOW (w),
			                                          GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			                                          GTK_MESSAGE_QUESTION,
			                                          GTK_BUTTONS_YES_NO,
			                                          "Sicuri di voler eliminare l'utente selezionato?");
			if (gtk_dialog_run (GTK_DIALOG (diag)) == GTK_RESPONSE_YES)
				{
					gchar *sql;
					gchar *codice;

					gtk_tree_model_get (GTK_TREE_MODEL (store_utenti), &iter,
					                    COL_COD, &codice,
					                    -1);
				
					sql = g_strdup_printf ("UPDATE utenti "
					                       "SET stato = 'E' "
					                       "WHERE codice = '%s'",
					                       codice);
					if (gdao_execute (gdao, sql) < 0)
						{
							g_warning ("Errore nell'eliminazione dell'utente.");
						}
					else
						{
							GModule *module;

							/* carico il modulo per l'autenticazione */
							module = aute_plugin_get_module (solipa->confi);
							if (module != NULL)
								{
									gboolean (*elimina_utente) (Confi *confi, const gchar *codice);

									if (!g_module_symbol (module, "elimina_utente", (gpointer *)&elimina_utente))
										{
											g_warning ("Errore nel caricamento della funzione del plugin di autenticazione.");
										}
									else
										{
											(*elimina_utente) (solipa->confi, codice);
										}
								}

							riempi_tr_utenti ();
						}
				}
			gtk_widget_destroy (diag);
		}
	else
		{
			/* TO DO */
		}
}

void
on_mnu_help_about_activate (GtkMenuItem *menuitem,
                            gpointer user_data)
{
	GladeXML *gla_about = glade_xml_new (GLADEDIR "/utenti.glade", "diag_about", NULL);
	GtkWidget *diag = glade_xml_get_widget (gla_about, "diag_about");

	gtk_dialog_run (GTK_DIALOG (diag));
	gtk_widget_destroy (diag);
}

int
main (int argc, char **argv)
{
	GladeXML *gla_main;
	gchar *provider_id;
	gchar *cnc_string;
	FormField *ffld;
	FormWidget *fwdg;
	FormKey *fkey;

	gtk_init (&argc, &argv);

	solipa = solipa_init (&argc, &argv);
	if (solipa == NULL)
		{
			return 0;
		}

	provider_id = confi_path_get_value (solipa->confi, "utenti/db/provider_id");
	cnc_string = confi_path_get_value (solipa->confi, "utenti/db/cnc_string");

	gdao = gdao_new_from_string (NULL, provider_id, cnc_string);

	gla_main = glade_xml_new (GLADEDIR "/utenti.glade", "w_main", NULL);
	glade_xml_signal_autoconnect (gla_main);

	w = glade_xml_get_widget (gla_main, "w_main");
	tr_utenti = glade_xml_get_widget (gla_main, "tr_utenti");

	gla_utente = glade_xml_new (GLADEDIR "/utenti.glade", "diag_utente", NULL);
	diag_utente = glade_xml_get_widget (gla_utente, "diag_utente");
	btn_ok_utente = glade_xml_get_widget (gla_utente, "btn_ok");

	/* creo l'oggetto Form per diag_utente */
	frm_utente = form_new ();

	fkey = form_key_new ();
	g_object_set (G_OBJECT (frm_utente),
	              "key", fkey,
								"table", "utenti",
	              NULL);

	txt_codice = glade_xml_get_widget (gla_utente, "txt_codice");
	txt_password = glade_xml_get_widget (gla_utente, "txt_password");
	txt_conferma_password = glade_xml_get_widget (gla_utente, "txt_conferma_password");

	fwdg = form_widget_entry_new ();
	form_widget_set_from_glade (fwdg, gla_utente, "txt_codice");

	ffld = form_field_text_new ();
	g_object_set (G_OBJECT (ffld),
	              "field", "codice",
	              "form-widget", fwdg,
								NULL);
	form_add_field (frm_utente, ffld);
	form_key_add_field (fkey, ffld);

	fwdg = form_widget_entry_new ();
	form_widget_set_from_glade (fwdg, gla_utente, "txt_id_personale");

	ffld = form_field_integer_new ();
	g_object_set (G_OBJECT (ffld),
	              "field", "id_personale",
	              "form-widget", fwdg,
								NULL);
	form_add_field (frm_utente, ffld);

	fwdg = form_widget_entry_new ();
	form_widget_set_from_glade (fwdg, gla_utente, "txt_cognome");

	ffld = form_field_text_new ();
	g_object_set (G_OBJECT (ffld),
	              "field", "cognome",
	              "form-widget", fwdg,
								NULL);
	form_add_field (frm_utente, ffld);

	fwdg = form_widget_entry_new ();
	form_widget_set_from_glade (fwdg, gla_utente, "txt_nome");

	ffld = form_field_text_new ();
	g_object_set (G_OBJECT (ffld),
	              "field", "nome",
	              "form-widget", fwdg,
								NULL);
	form_add_field (frm_utente, ffld);

	fwdg = form_widget_entry_new ();
	form_widget_set_from_glade (fwdg, gla_utente, "txt_password");

	ffld = form_field_text_new ();
	g_object_set (G_OBJECT (ffld),
	              "form-widget", fwdg,
								NULL);
	form_add_field (frm_utente, ffld);

	fwdg = form_widget_entry_new ();
	form_widget_set_from_glade (fwdg, gla_utente, "txt_conferma_password");

	ffld = form_field_text_new ();
	g_object_set (G_OBJECT (ffld),
	              "form-widget", fwdg,
								NULL);
	form_add_field (frm_utente, ffld);

	fwdg = form_widget_check_new ();
	form_widget_set_from_glade (fwdg, gla_utente, "chk_abilitato");

	ffld = form_field_boolean_new ();
	g_object_set (G_OBJECT (ffld),
	              "field", "abilitato",
	              "form-widget", fwdg,
								"default", TRUE,
								NULL);
	form_add_field (frm_utente, ffld);

	fwdg = form_widget_label_new ();
	form_widget_set_from_glade (fwdg, gla_utente, "lbl_ultimo_accesso");

	ffld = form_field_datetime_new ();
	g_object_set (G_OBJECT (ffld),
	              "field", "ultimo_accesso",
	              "form-widget", fwdg,
								NULL);
	form_add_field (frm_utente, ffld);

	fwdg = form_widget_textview_new ();
	form_widget_set_from_glade (fwdg, gla_utente, "txtv_descrizione");

	ffld = form_field_text_new ();
	g_object_set (G_OBJECT (ffld),
	              "field", "descrizione",
	              "form-widget", fwdg,
								NULL);
	form_add_field (frm_utente, ffld);

	crea_tr_utenti ();
	if (gdao != NULL)
		{
			riempi_tr_utenti ();
		}

	gtk_main ();

	return 0;
}
