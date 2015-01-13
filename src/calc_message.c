#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <errno.h>
#include "guni.h"


static void show_message (GtkWidget *, struct _data *, gfloat, gfloat, gfloat);


void
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
			gtk_entry_set_icon_from_icon_name (GTK_ENTRY (data->cfu_list[i]), GTK_ENTRY_ICON_SECONDARY, "dialog-warning-symbolic");
			error_dialog ("Il numero di CFU deve essere compreso fra 1 e 15", data->main_window);
			return;
		}

		if (voto[i] < 18 || voto[i] > 30)
		{
			gtk_entry_set_icon_from_icon_name (GTK_ENTRY (data->voto_list[i]), GTK_ENTRY_ICON_SECONDARY, "dialog-warning-symbolic");
			
			error_dialog ("Il voto deve essere compreso fra 18 e 30", data->main_window);
			
			return;
		}		
	}
	
	for (i=0; i<stop; i++)
	{
		if (gtk_entry_get_icon_pixbuf (GTK_ENTRY (data->cfu_list[i]), GTK_ENTRY_ICON_SECONDARY) != NULL)
			gtk_entry_set_icon_from_icon_name (GTK_ENTRY (data->cfu_list[i]), GTK_ENTRY_ICON_SECONDARY, NULL);
		
		if (gtk_entry_get_icon_pixbuf (GTK_ENTRY (data->voto_list[i]), GTK_ENTRY_ICON_SECONDARY) != NULL)
			gtk_entry_set_icon_from_icon_name (GTK_ENTRY (data->voto_list[i]), GTK_ENTRY_ICON_SECONDARY, NULL);
			
		tmp_arit += voto[i];
		tmp_pond = tmp_pond + (cfu[i]*voto[i]);
		cfu_summation += cfu[i];
	}

	m_arit = (gfloat)tmp_arit / stop;
	m_pond = (gfloat)tmp_pond/cfu_summation;
	base_laurea = (m_pond * 110)/30;
	
	show_message (data->main_window, data, m_arit, m_pond, base_laurea);
	
	if (data->prev.first_calc)
		data->prev.first_calc = FALSE;
		
	data->prev.p_ma = m_arit;
	data->prev.p_mp = m_pond;
	data->prev.p_bl = base_laurea;
}


static void
show_message (	GtkWidget *mainwin,
				struct _data *data,
				gfloat ma,
				gfloat mp,
				gfloat bl)
{
	gchar old_message[200];
	gchar message[120];
	gchar *final_message;
	
	if (!data->prev.first_calc)
		g_snprintf (old_message, 200, "(old) Media Aritmetica:\t<b>%.2f</b>\n(old) Media Ponderata:\t<b>%.2f</b>\n(old) Base di Laurea:\t<b>%.2f</b>\n\n", data->prev.p_ma, data->prev.p_mp, data->prev.p_bl);
	
	g_snprintf (message, 120, "Media Aritmetica:\t<b>%.2f</b>\nMedia Ponderata:\t<b>%.2f</b>\nBase di Laurea:\t<b>%.2f</b>", ma, mp, bl);
	
	if (!data->prev.first_calc)
		final_message = g_strconcat (old_message, message, NULL);
	else
		final_message = g_strconcat (message, NULL);
	
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
