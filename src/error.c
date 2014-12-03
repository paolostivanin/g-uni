#include <gtk/gtk.h>
#include <glib.h>
#include "guni.h"


void
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
