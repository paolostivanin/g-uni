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
#include "guni.h"


/* TODO:
 * - aggiungere 20 esami ((10xframe)x2)
 */


GtkWidget *do_mainwin (GtkApplication *, struct _data *);
static void startup (GtkApplication *,  gpointer);
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
	
	data.prev.first_calc = TRUE;
	data.prev.p_ma = 0.0;
	data.prev.p_mp = 0.0;
	data.prev.p_bl = 0.0;
	
	app = gtk_application_new ("org.gtk.baselaurea", G_APPLICATION_FLAGS_NONE);
	g_signal_connect (app, "startup", G_CALLBACK (startup), NULL);
	g_signal_connect (app, "activate", G_CALLBACK (activate), &data);
	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);
	return status;
}


static void
startup (	GtkApplication *application,
			gpointer user_data __attribute__((__unused__)))
{
	static GActionEntry actions[] = {
		{ "about", about, NULL, NULL, NULL },
		{ "quit", quit, NULL, NULL, NULL }
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
		
		g_signal_connect (data->voto_list[i], "activate", G_CALLBACK (calc), data);
	}
		
	for (i=0; i<WN-1; i++)
	{
		g_signal_connect (data->voto_list[i], "grab-focus", G_CALLBACK (show_next), data->voto_list[i+1]);
		g_signal_connect (data->voto_list[i], "grab-focus", G_CALLBACK (show_next), data->cfu_list[i+1]);
	}
	
	grid = gtk_grid_new();
	gtk_container_add (GTK_CONTAINER (data->main_window), grid);
	gtk_grid_set_column_homogeneous (GTK_GRID (grid), TRUE);
	
	for (i=0; i<WN+1; i++)
	{
		bx[i] = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
		
	}
	
	gtk_box_pack_start (GTK_BOX(bx[0]), lb[0], TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(bx[0]), lb[1], TRUE, TRUE, 0);
	
	for(i=1; i<WN+1; i++)
	{
		gtk_box_pack_start (GTK_BOX(bx[i]), data->cfu_list[i-1], TRUE, TRUE, 0);
		gtk_box_pack_start (GTK_BOX(bx[i]), data->voto_list[i-1], TRUE, TRUE, 0);
	}

	for (i=0, j=0; i<WN+1; i++, j+=2)
		gtk_grid_attach (GTK_GRID (grid), bx[i], 0, j, 1, 1);	

	gtk_widget_show_all (data->main_window);
	
	for (i=1; i<WN; i++)
	{
		gtk_widget_hide (data->cfu_list[i]);
		gtk_widget_hide (data->voto_list[i]);
	}
}


static void
show_next (	GtkWidget *caller __attribute__((__unused__)),
			GtkWidget *showme)
{
	GValue top_margin = G_VALUE_INIT;
	
	if (!G_IS_VALUE (&top_margin))
		g_value_init (&top_margin, G_TYPE_UINT);
			
	g_value_set_uint (&top_margin, 10);
	g_object_set_property (G_OBJECT (showme), "margin-top", &top_margin);
	
	gtk_widget_show (showme);
}


static void
about (	GSimpleAction *action __attribute__((__unused__)), 
		GVariant *parameter __attribute__((__unused__)),
		gpointer user_data)
{
	GtkApplication *app = user_data;
	GtkWindow *win = gtk_application_get_active_window (app);
	
	const gchar *authors[] =
	{
			"Paolo Stivanin <info@paolostivanin.com>",
			NULL,
	};

	GtkWidget *a_dialog = gtk_about_dialog_new ();
	gtk_window_set_transient_for (GTK_WINDOW (a_dialog), GTK_WINDOW (win));
	
	gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG (a_dialog), "G-Uni");
   
	gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (a_dialog), VERSION);
	gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG (a_dialog), "Copyright (C) 2014");
	gtk_about_dialog_set_comments (GTK_ABOUT_DIALOG (a_dialog), _("Calcolare la base di laurea e la media aritmetica e ponderata"));
	gtk_about_dialog_set_license_type (GTK_ABOUT_DIALOG (a_dialog), GTK_LICENSE_GPL_3_0);
	gtk_about_dialog_set_website (GTK_ABOUT_DIALOG (a_dialog), "http://paolostivanin.com");
	gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG (a_dialog), authors);

	gtk_dialog_run(GTK_DIALOG (a_dialog));
	gtk_widget_destroy (a_dialog);
}


static void
quit (	GSimpleAction *action __attribute__((__unused__)),
		GVariant *parameter __attribute__((__unused__)),
		gpointer user_data)
{
	GtkApplication *app = user_data;
	g_application_quit (G_APPLICATION (app));
}
