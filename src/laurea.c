/* 
 * Copyright 2014 Paolo Stivanin a.k.a. Polslinux
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 */

#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib/gi18n.h>
#include <locale.h>
#include <stdlib.h>
#include <ctype.h>
#include <libintl.h>
#include <errno.h>

#define VERSION "1.0.0"
#define LOCALE_DIR "/usr/share/locale"
#define PACKAGE "baselaurea"          /* mo file name in LOCALE_DIR */

#define WN 10

/* TODO:
 * - eliminare lo spazio iniziale, fa schifooooo
 * - tener traccia del precedente (hash table o linked list) e fornire nel show_mesasge anche il preb
 */

struct _data
{
	GtkWidget *main_window;
	GtkWidget *cfu_list[WN];
	GtkWidget *voto_list[WN];
};


GtkWidget *do_mainwin (GtkApplication *, struct _data *);
static void calc (GtkWidget *btn, struct _data *);
static void show_message (GtkWidget *, gfloat, gfloat, gfloat);
static void error_dialog (const gchar *, GtkWidget *);
static void startup (GtkApplication *, gpointer );
static void activate (GtkApplication *, struct _data *);
static void about (GSimpleAction *, GVariant *, gpointer);
static void quit (GSimpleAction *, GVariant *, gpointer);
static void show_next (GtkWidget *, GtkWidget *);


gint
main (	int argc,
		char *argv[])
{
	setlocale (LC_ALL, "");
	bindtextdomain (PACKAGE, LOCALE_DIR);
	textdomain (PACKAGE);

	GtkApplication *app;
	gint status;
	struct _data data;
	
	app = gtk_application_new ("org.gtk.baselaurea", G_APPLICATION_FLAGS_NONE);
	g_signal_connect (app, "startup", G_CALLBACK (startup), NULL);
	g_signal_connect (app, "activate", G_CALLBACK (activate), &data);
	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);
	return status;
}


static void
startup (	GtkApplication *application,
			gpointer __attribute__((__unused__)) data)
{
	static const GActionEntry actions[] = {
		{ "about", about },
		{ "quit", quit }
	};
	
	const gchar *quit_accels[2] = { "<Ctrl>Q", NULL };
	
	GMenu *menu, *section;

	g_action_map_add_action_entries (G_ACTION_MAP (application),
					 actions, G_N_ELEMENTS (actions),
					 application);
	
	gtk_application_set_accels_for_action (GTK_APPLICATION (application),
					       "app.quit",
					       quit_accels);
							
	menu = g_menu_new ();
	
	section = g_menu_new ();
	g_menu_append (section, _("About"), "app.about");
	g_menu_append_section (G_MENU (menu), NULL, G_MENU_MODEL (section));
	g_object_unref (section);
	
	section = g_menu_new ();
	g_menu_append (section, _("Quit"),  "app.quit");
	g_menu_append_section (G_MENU (menu), NULL, G_MENU_MODEL (section));
	g_object_unref (section);

	gtk_application_set_app_menu (application, G_MENU_MODEL (menu));
	g_object_unref (menu);
}


static void
activate (	GtkApplication *app,
			struct _data *data)
{
	if (glib_check_version (2, 40, 0) != NULL){
		error_dialog ( _("The required version of GLib is 2.40.0 or greater."), NULL);
		return;
	}
	if (gtk_check_version (3, 12, 0) != NULL){
		error_dialog ( _("The required version of GTK+ is 3.12.0 or greater."), NULL);
		return;
	}
		
	gint i,j;
	GtkWidget *lb[2];
	GtkWidget *grid;
	GtkWidget *bx[WN+1];
	
	data->main_window = do_mainwin (app, data);
	
	lb[0] = gtk_label_new("CFU");
	lb[1] = gtk_label_new("Voto");
	
	for (i=0; i<WN; i++)
	{
		data->cfu_list[i] = gtk_entry_new ();
		gtk_entry_set_max_length (GTK_ENTRY (data->cfu_list[i]), 2);

		data->voto_list[i] = gtk_entry_new ();
		gtk_entry_set_max_length (GTK_ENTRY (data->voto_list[i]), 2);
	}
		
	for (i=0; i<WN-1; i++)
	{
		g_signal_connect (data->voto_list[i], "grab-focus", G_CALLBACK (show_next), data->voto_list[i+1]);
		g_signal_connect (data->voto_list[i], "grab-focus", G_CALLBACK (show_next), data->cfu_list[i+1]);
		g_signal_connect (data->voto_list[i], "activate", G_CALLBACK (calc), data);
	}
	
	grid = gtk_grid_new();
	gtk_container_add (GTK_CONTAINER (data->main_window), grid);
	gtk_grid_set_column_homogeneous (GTK_GRID (grid), TRUE);
	gtk_grid_set_row_spacing (GTK_GRID (grid), 10);
	gtk_grid_set_column_spacing (GTK_GRID (grid), 5);
	
	for (i=0; i<WN+1; i++)
		bx[i] = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
		
	gtk_box_pack_start (GTK_BOX(bx[0]), lb[0], TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX(bx[0]), lb[1], TRUE, TRUE, 2);
	
	for(i=1; i<WN+1; i++)
	{
		gtk_box_pack_start (GTK_BOX(bx[i]), data->cfu_list[i-1], TRUE, TRUE, 2);
		gtk_box_pack_start (GTK_BOX(bx[i]), data->voto_list[i-1], TRUE, TRUE, 2);
	}

	for (i=0, j=0; i<WN+1; i++, j+=2)
		gtk_grid_attach (GTK_GRID (grid), bx[i], 0, j, 2, 2);	

	gtk_widget_show_all (data->main_window);
	
	for (i=1; i<WN; i++)
	{
		gtk_widget_hide (data->cfu_list[i]);
		gtk_widget_hide (data->voto_list[i]);
	}
}


static void
show_next (GtkWidget __attribute__((__unused__)) *caller, GtkWidget *showme)
{
	gtk_widget_show (showme);
}


GtkWidget
*do_mainwin (GtkApplication *app, struct _data *data)
{
	static GtkWidget *window = NULL;
	GtkWidget *header_bar;
	GtkWidget *box;
	GtkWidget *btn;
	
	window = gtk_application_window_new(app);
	gtk_window_set_application (GTK_WINDOW (window), GTK_APPLICATION (app));
	gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
	
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	
	btn = gtk_button_new_with_label ("Calcolare");
	
	gchar headertext[12 + strlen (VERSION) + 1];
	g_snprintf (headertext, sizeof (headertext), _("Base Laurea %s"), VERSION);
	headertext[sizeof (headertext)-1] = '\0';

	header_bar = gtk_header_bar_new ();
	gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (header_bar), TRUE);
	gtk_header_bar_set_title (GTK_HEADER_BAR (header_bar), headertext);
	gtk_header_bar_set_has_subtitle (GTK_HEADER_BAR (header_bar), FALSE);
	
	box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_style_context_add_class (gtk_widget_get_style_context (box), "linked");
	gtk_header_bar_pack_end (GTK_HEADER_BAR (header_bar), btn);
		
	gtk_window_set_titlebar (GTK_WINDOW (window), header_bar);
	
	g_signal_connect (btn, "clicked", G_CALLBACK (calc), data);
	
	return window;
}


static void
calc (	GtkWidget __attribute__((__unused__)) *btn,
		struct _data *data)
{	
	gint i, stop = 0, tmp_arit = 0, tmp_pond = 0, cfu_summation = 0;
	gint cfu[WN], voto[WN];
	gfloat m_arit, m_pond, base_laurea;
	
	for (i=0; i<WN; i++)
		if (gtk_entry_get_text_length (GTK_ENTRY (data->cfu_list[i])) >= 1 && gtk_entry_get_text_length (GTK_ENTRY (data->voto_list[i])) >= 1)
			stop += 1;
	
	for (i=0; i<stop; i++)
	{
		cfu[i] = (gint)g_ascii_strtoll (gtk_entry_get_text (GTK_ENTRY (data->cfu_list[i])), NULL, 10);
		voto[i] = (gint)g_ascii_strtoll (gtk_entry_get_text (GTK_ENTRY (data->voto_list[i])), NULL, 10);
		
		if (cfu[i] < 1 || cfu[i] > 15)
		{
			error_dialog ("Il numero di CFU deve essere compreso fra 1 e 15", data->main_window);
			return;
		}

		if (voto[i] < 18 || voto[i] > 30)
		{
			error_dialog ("Il voto deve essere compreso fra 18 e 30", data->main_window);
			return;
		}		
			
	}
	
	for (i=0; i<stop; i++)
	{
		tmp_arit += voto[i];
		tmp_pond = tmp_pond + (cfu[i]*voto[i]);
		cfu_summation += cfu[i];
	}

	m_arit = (gfloat)tmp_arit / stop;
	m_pond = (gfloat)tmp_pond/cfu_summation;
	base_laurea = (m_pond * 110)/30;
	show_message (data->main_window, m_arit, m_pond, base_laurea);
}


static void
show_message (	GtkWidget *mainwin,
				gfloat ma,
				gfloat mp,
				gfloat bl)
{
	gchar message[120];
	g_snprintf (message, 120, "Media Aritmetica:\t<b>%.2f</b>\nMedia Ponderata:\t<b>%.2f</b>\nBase di Laurea:\t<b>%.2f</b>", ma, mp, bl);
	
	GtkWidget *dialog;
	dialog = gtk_message_dialog_new (GTK_WINDOW (mainwin),
					GTK_DIALOG_MODAL,
					GTK_MESSAGE_INFO,
					GTK_BUTTONS_OK,
					"%s", message);
	
	gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog), message);
					
	gtk_dialog_run (GTK_DIALOG (dialog));
	
	gtk_widget_destroy (dialog);
}


static void
error_dialog (	const gchar *message,
				GtkWidget *mainwin)
{
	GtkWidget *dialog;
	dialog = gtk_message_dialog_new (GTK_WINDOW (mainwin),
					GTK_DIALOG_MODAL,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_OK,
					"%s", message);
					
	gtk_dialog_run (GTK_DIALOG (dialog));
	
	gtk_widget_destroy (dialog);
}


static void
about (	GSimpleAction __attribute__((__unused__)) *action, 
		GVariant __attribute__((__unused__)) *parameter,
		gpointer __attribute__((__unused__)) data)
{
        const gchar *authors[] =
        {
                "Paolo Stivanin <info@paolostivanin.com>",
                NULL,
        };
	
        GtkWidget *a_dialog = gtk_about_dialog_new ();
        gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG (a_dialog), "Base di Laurea");
       
        gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (a_dialog), VERSION);
        gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG (a_dialog), "Copyright (C) 2014");
        gtk_about_dialog_set_comments (GTK_ABOUT_DIALOG (a_dialog), _("Calcolare la base di laurea e la media aritmetica e ponderata"));
        gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(a_dialog),
					"This program is free software: you can redistribute it and/or modify it under the terms"
					" of the GNU General Public License as published by the Free Software Foundation, either version 3 of"
					" the License, or (at your option) any later version.\n"
					"This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even"
					" the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. "
					"See the GNU General Public License for more details.\n"
					"You should have received a copy of the GNU General Public License along with this program."
					"\nIf not, see http://www.gnu.org/licenses\n\nPolCrypt is Copyright (C) 2014 by Paolo Stivanin.\n");
        gtk_about_dialog_set_wrap_license (GTK_ABOUT_DIALOG (a_dialog), TRUE);
        gtk_about_dialog_set_website (GTK_ABOUT_DIALOG (a_dialog), "http://paolostivanin.com");
        gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG (a_dialog), authors);

        gtk_dialog_run(GTK_DIALOG (a_dialog));
        gtk_widget_destroy (a_dialog);
}


static void
quit (	GSimpleAction __attribute__((__unused__)) *action,
		GVariant __attribute__((__unused__)) *parameter,
		gpointer app)
{
	g_application_quit (G_APPLICATION (app));
}
