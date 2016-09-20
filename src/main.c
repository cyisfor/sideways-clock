#include "ui.glade.xml.h"

#include <cairo/cairo.h>
#include <gtk/gtk.h>
#include <math.h>

//#include <gdk.h>

#define MARGIN 5.0


gboolean supports_alpha = FALSE;
static void screen_changed(GtkWidget *widget, GdkScreen *old_screen, gpointer userdata)
{
    /* To check if the display supports alpha channels, get the visual */
    GdkScreen *screen = gtk_widget_get_screen(widget);
    GdkVisual *visual = gdk_screen_get_rgba_visual(screen);

    if (!visual)
    {
        printf("Your screen does not support alpha channels!\n");
        visual = gdk_screen_get_system_visual(screen);
        supports_alpha = FALSE;
    }
    else
    {
        printf("Your screen supports alpha channels!\n");
        supports_alpha = TRUE;
    }

    gtk_widget_set_visual(widget, visual);
}

int main(int argc, char *argv[])
{
	gtk_init(&argc,&argv);
	GtkBuilder* b = gtk_builder_new_from_string
		(ui_glade_xml,ui_glade_xml_length);
	GtkWidget* top = GTK_WIDGET(gtk_builder_get_object(b,"top"));
	const char* strtime;
	const char* timeformat = "%k:%m";
	void new_hour(struct tm *curtime_tm) {
		char buf[0x100];
		strftime(buf,0x100,"%a %l %P",curtime_tm);
		gtk_widget_set_tooltip_text(top,buf);
	}
	gint last_hour = -1;
	void gettime()
	{
		time_t curtime;
		struct tm *curtime_tm;
		static char timebuf[0x100];
		curtime = time(NULL);
		curtime_tm = localtime(&curtime);
		if(last_hour != curtime_tm->tm_hour) {
			last_hour = curtime_tm->tm_hour;
			new_hour(curtime_tm);
		}
		strftime(timebuf, 49, timeformat, curtime_tm);
		strtime = timebuf;
		while(*strtime==' ') ++strtime;
	}
	gtk_window_move(GTK_WINDOW(top),0,100);
	void maybe_resize(gint width, gint height) {
		static int old_height, old_width, gotit = 0;
		if(gotit == 0) {
			gotit = 1;
		} else {
			if(old_height == height && old_width == width) return;
		} 
		old_height = height;
		old_width = width;
		gtk_window_resize(GTK_WINDOW(top), width, height);
	}

	cairo_surface_t* surface = gdk_window_create_similar_surface
		(gtk_widget_get_window (widget),
		 CAIRO_CONTENT_COLOR,
		 gtk_widget_get_allocated_width (widget),
		 gtk_widget_get_allocated_height (widget));
	
	cairo_font_slant_t slant = CAIRO_FONT_SLANT_NORMAL;
	cairo_font_weight_t weight = CAIRO_FONT_WEIGHT_BOLD;
	int size = 18;
	cairo_text_extents_t te;

	void size_to_font(cairo_t* cairo) {
		cairo_select_font_face(cairo,
			 "serif",
			 slant,
			 weight);
		cairo_set_font_size(cairo,size);
		cairo_text_extents (cairo, strtime, &te);
		// note the height of the text is the width of the window.
		maybe_resize(
			 te.height + MARGIN*2,
			 te.width + te.x_bearing + MARGIN*2);
	}

	gboolean draw_it(GtkWidget    *widget,
               cairo_t *cairo,
               gpointer      user_data) {
		size_to_font(cairo);

		cairo_matrix_t tf;
		{
			cairo_matrix_init_translate(&tf, MARGIN,MARGIN);
			cairo_matrix_rotate(&tf, M_PI / 2);
		}
		cairo_transform(cairo, &tf);

		cairo_set_source_rgba(cairo,1,0,1,1);
		cairo_rectangle(cairo,0,0,
										te.height+MARGIN*2,
										te.width+te.x_bearing+MARGIN*2);
		cairo_stroke(cairo);

		cairo_set_source_rgba(cairo, 0,1,1,1);
    cairo_move_to (cairo,0,0);
		cairo_show_text(cairo, strtime);
		cairo_fill(cairo);
		return FALSE;
	}

	g_signal_connect(top,"draw",G_CALLBACK(draw_it),NULL);

	gboolean update_clock(gpointer udata) {
		gettime();
		gtk_widget_queue_draw(GTK_WIDGET(top));
		return G_SOURCE_CONTINUE;
	}
	// set the initial size...
	size_to_font(gdk_cairo_create(gtk_widget_get_window(top)));
	
	g_timeout_add(100,update_clock,NULL);

	g_signal_connect(top,"destroy",gtk_main_quit,NULL);
	g_signal_connect(top,"screen-changed", G_CALLBACK(screen_changed), NULL);
	screen_changed(top,NULL,NULL);
	gtk_widget_show_all(top);
	gtk_main();
	return 0;
}
