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

#define VERSION "1.0.1"
#define LOCALE_DIR "/usr/share/locale"
#define PACKAGE "baselaurea" /* mo file name in LOCALE_DIR */

#define WN 10

/* TODO:
 * - aggiungere 20 esami ((10xframe)x2)
 * - calcolo iniziale senza media precedente (tutto a 0 non mi serve visualizzarla)
 */

struct _data
{
	GtkWidget *main_window;
	GtkWidget *cfu_list[WN];
	GtkWidget *voto_list[WN];
	struct _prev
	{
		gfloat p_ma;
		gfloat p_mp;
		gfloat p_bl;
	}prev;
};


GtkWidget *do_mainwin (GtkApplication *, struct _data *);
static void calc (GtkWidget *btn, struct _data *);
static void show_message (GtkWidget *, gfloat, gfloat, gfloat, gfloat, gfloat, gfloat);
static void error_dialog (const gchar *, GtkWidget *);
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
calc (	GtkWidget *btn __attribute__((__unused__)),
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
	
	show_message (data->main_window, data->prev.p_ma, data->prev.p_mp, data->prev.p_bl, m_arit, m_pond, base_laurea);
	
	data->prev.p_ma = m_arit;
	data->prev.p_mp = m_pond;
	data->prev.p_bl = base_laurea;
}


static void
show_message (	GtkWidget *mainwin,
				gfloat p_ma,
				gfloat p_mp,
				gfloat p_bl,
				gfloat ma,
				gfloat mp,
				gfloat bl)
{
	gchar old_message[200];
	gchar message[120];
	gchar *final_message;
	
	g_snprintf (old_message, 200, "(old) Media Aritmetica:\t<b>%.2f</b>\n(old) Media Ponderata:\t<b>%.2f</b>\n(old) Base di Laurea:\t<b>%.2f</b>\n\n", p_ma, p_mp, p_bl);
	g_snprintf (message, 120, "Media Aritmetica:\t<b>%.2f</b>\nMedia Ponderata:\t<b>%.2f</b>\nBase di Laurea:\t<b>%.2f</b>", ma, mp, bl);
	final_message = g_strconcat (old_message, message, NULL);
	
	GtkWidget *dialog;
	dialog = gtk_message_dialog_new (GTK_WINDOW (mainwin),
					GTK_DIALOG_MODAL,
					GTK_MESSAGE_INFO,
					GTK_BUTTONS_OK,
					"%s\n%s", old_message, message);
	
	gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog), final_message);
					
	gtk_dialog_run (GTK_DIALOG (dialog));
	
	gtk_widget_destroy (dialog);
	
	g_free (final_message);
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
	
	gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG (a_dialog), "Base di Laurea");
   
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
