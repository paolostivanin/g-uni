#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <string.h>
#include "guni.h"


GtkWidget
*do_mainwin (	GtkApplication *app,
				struct _data *data)
{
	static GtkWidget *window = NULL;
	GtkWidget *header_bar;
	GtkWidget *box;
	GtkWidget *btn;
	
	window = gtk_application_window_new(app);
	gtk_window_set_application (GTK_WINDOW (window), GTK_APPLICATION (app));
	gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
	
	gtk_container_set_border_width (GTK_CONTAINER (window), 5);
	
	btn = gtk_button_new_with_label ("Calcolare");
	
	gchar headertext[6 + strlen (VERSION) + 1];
	g_snprintf (headertext, sizeof (headertext), "G-Uni %s", VERSION);
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
