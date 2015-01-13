#ifndef GUNI_H_INCLUDED
#define GUNI_H_INCLUDED

#define VERSION "1.0.3"
#define LOCALE_DIR "/usr/share/locale"
#define PACKAGE "baselaurea" /* mo file name in LOCALE_DIR */

#define WN 10

struct _data
{
	GtkWidget *main_window;
	GtkWidget *cfu_list[WN];
	GtkWidget *voto_list[WN];
	struct _prev
	{
		gboolean first_calc;
		gfloat p_ma;
		gfloat p_mp;
		gfloat p_bl;
	}prev;
};
extern struct _data data;


void calc (GtkWidget *btn, struct _data *);
void error_dialog (const gchar *, GtkWidget *);

#endif
