/*
FRACTAL GENERATOR GUI:  Written by F. Sullivan and B. Young. 2018
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <math.h>
#include <complex.h>
#include <X11/Xlib.h>
#include <gtk-3.0/gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdk.h>
#include <cairo.h>

typedef cairo_t *graphics_context;

double complex (*f) (double complex) = NULL;

//Data entry fields

  GtkWidget *x_entry;
  GtkWidget *y_entry;
  GtkWidget *func_entry;
  GtkWidget *xmin_entry;
  GtkWidget *xmax_entry;
  GtkWidget *ymin_entry;
  GtkWidget *ymax_entry;
  GtkWidget *pix_entry;
  GtkWidget *thr_entry;
  GtkWidget *iter_entry;
  GtkWidget *hoff1_entry;
  GtkWidget *hoff2_entry;
  GtkWidget *file_name;

//Data structs


typedef struct cnum {
  double re;
  double im;
} cnum;

typedef struct PlotData {
  int set_flg;
  int man_flg;
  int maxw;
  int maxh;
  double x;
  double y;
  double xmin;
  double xmax;
  double ymin;
  double ymax;
  int pix;
  int pix_h;
  int pix_w;
  int ctrl_w;
  int ctrl_h;
  double thr;
  int iter;
  double h_off;
  int cyc_flg;
  double cyc_off;
  int rev_flg;
  double sat;
  int axes_flg;
  int grid_flg;
  char *fun;
  GtkWidget *display;
  GtkWidget *text;
} PlotData;

typedef struct fracdat {
  int c;
  double x;
  double y;
} fracdat;

typedef struct colors {
  double r;
  double g;
  double b;
} colors;


//Translate pixels to points in the complex plane.

double xx(double x, int WIDTH, double Re_Min, double Re_Max) {
  return Re_Min + x/WIDTH*(Re_Max - Re_Min);
}

double yy(double y, int HEIGHT, double Im_Min, double Im_Max) {
  return Im_Max + y/HEIGHT*(Im_Min - Im_Max);
}

//Translate x and y to pixel labels.

int ii(double x, int WIDTH, double Re_Min, double Re_Max) {
  double tmp = WIDTH*(x - Re_Min)/(Re_Max - Re_Min)+1;
  int i = (int) tmp;
  return i;
}

int jj(double y, int HEIGHT, double Im_Min, double Im_Max) {
  double tmp = HEIGHT*(y - Im_Max)/(Im_Min - Im_Max)+1;
  int j = (int) tmp;
  return j;
}

//Fractal Generator


fracdat frac_gen(int flg, int man, int Max_Iter, double Threshold, double xc, double yc, double cx, double cy, double complex (*f) (double complex)) {

  fracdat ret = {0,0,0};
  int k = 0;
  double x = 0;
  double y = 0;
  double complex z = 0 + 0*I;
  double complex zc = xc + yc*I;
  double complex cz = cx + cy*I;

  switch (man) {
  case 0: //Default Sets
    switch(flg) {
    case 0: //Mandelbrot Set
      while (x*x + y*y < Threshold*Threshold && k < Max_Iter) {
	double xtmp = x;
	x = x*x - y*y + xc;
	y = 2*xtmp*y + yc;
	k++;
      }
      ret.x = x;
      ret.y = y;
      ret.c = k;
      break;
      
    case 1://Julia Set
      x = xc;
      y = yc;

      while (x*x + y*y < Threshold*Threshold && k < Max_Iter) {
	double xtmp = x;
	x = x*x - y*y + cx;
	y = 2*xtmp*y + cy;
	k++;
      }
      ret.x = x;
      ret.y = y;
      ret.c = k;
      break;
    }
    break;

  case 1: //Manual Entry
  switch(flg) {
      case 0: //Mandelbrot Set
      
      while (cabs(z) < Threshold && k < Max_Iter) {
	z = (*f)(z) + zc;
	k++;
    }
    ret.c = k;
    ret.x = creal(z);
    ret.y = cimag(z);
    break;

  case 1: //Julia Set
    z = xc + yc*I;
    while (cabs(z) < Threshold && k < Max_Iter) {
      z = (*f)(z) + cz;
      k++;
    }
    ret.c = k;
    ret.x = creal(z);
    ret.y = cimag(z);
    break;
  }
  break;
 }
  return ret;
}

//Pixel color functions

colors hsv2rgb(double h, double s, double v) {
  colors rgb = {0,0,0};
  double M = floor(h/360);
  h = h - M*360;
  double  H = h/60;
  double i = floor(H);
  double f = H - i;
  double p = v*(1-s);
  double q = v*(1-f*s);
  double t = v*(1-(1-f)*s);
  
  if (H < 1){
    rgb.r = v;
    rgb.g = t;
    rgb.b = p;
  } else if (H < 2){
    rgb.r = q;
    rgb.g = v;
    rgb.b = p;
  } else if (H < 3){
    rgb.r = p;
    rgb.g = v;
    rgb.b = t;
  } else if (H < 4){
    rgb.r = p;
    rgb.g = q;
    rgb.b = v;
  } else if (H < 5){
    rgb.r = t;
    rgb.g = p;
    rgb.b = v;
  } else {
    rgb.r = v;
    rgb.g = p;
    rgb.b = q;
  }
  return rgb;
}

void colorizer(int cyc_flg, int rev_flg, int Max_Iter, double hoff1, double hoff2, double sat, int i, int j, int cnt, double x, double y, double *r, double *g, double *b){
  double m = cnt  - log(log(x*x + y*y)/(2*log(2)));
  double h = 360*m/Max_Iter+ hoff1;
  double s = sat;
  double v = 0;

  switch (cyc_flg) {

  case 0: //Hue Cycler OFF
    switch (rev_flg) {
    case 0: //Cyler OFF and Reverser OFF
      if (cnt < Max_Iter) {v = 0.75;}
      break;
    case 1: //Cycler OFF and Reverser ON
      if (cnt == Max_Iter) {h=hoff1; v=0.8;} else {v = m/Max_Iter;}
      break;
    }
    break;

  case 1: //Hue Cycler ON
    switch (rev_flg) {
    case 0: //Cyler ON and Reverser OFF
      h = h + (i+j)*hoff2;
      if (cnt < Max_Iter) {v = 0.75;}
      break;
    case 1: //Cycler ON and Reverser ON
      if (cnt == Max_Iter) {h=(i+j)*hoff2+hoff1;v=0.8;} else {v = m/Max_Iter;}
      break;
    }

    break;
    
  }
  colors rgb = {0,0,0};
  rgb = hsv2rgb(h,s,v);
  *r = rgb.r;
  *g = rgb.g;
  *b = rgb.b;
}

//Callback to exit program

void destroy(GtkWidget *widget, gpointer data) {
  gtk_main_quit();
}


//Window resets

void m_window_reset(GtkWidget *widget, gpointer dat){
  gtk_entry_set_text(GTK_ENTRY(xmin_entry), "-2");
  gtk_entry_set_text(GTK_ENTRY(xmax_entry), "1");
  gtk_entry_set_text(GTK_ENTRY(ymin_entry), "-1");
  gtk_entry_set_text(GTK_ENTRY(ymax_entry), "1");
}

void j_window_reset(GtkWidget *widget, gpointer dat){
  gtk_entry_set_text(GTK_ENTRY(xmin_entry), "-1.5");
  gtk_entry_set_text(GTK_ENTRY(xmax_entry), "1.5");
  gtk_entry_set_text(GTK_ENTRY(ymin_entry), "-1");
  gtk_entry_set_text(GTK_ENTRY(ymax_entry), "1");
  }

//Print to file

void print2file(GtkWidget *widget, gpointer datap){
  PlotData *data = datap;
  
  if (!data->display || !gtk_widget_get_window(data->display)) return;

  GdkWindow *gdk_window = gtk_widget_get_window(GTK_WIDGET(data->display));

  cairo_surface_t *surf = cairo_image_surface_create(CAIRO_FORMAT_RGB24,data->pix_w,data->pix_h);
  cairo_t *cpy = cairo_create(surf);
  gdk_cairo_set_source_window(cpy,gdk_window,0,0);
  cairo_paint(cpy);
  
  char c[64];
  char *name = gtk_editable_get_chars(GTK_ENTRY(file_name),0,-1);
  snprintf(c,sizeof(c),"./%s.png",name);
  g_free(name);
  cairo_surface_write_to_png(surf,c);
  
  cairo_destroy(cpy);
  cairo_surface_destroy(surf);
}


//Toggle sensitivity of Julia Set options

void julia_option_off(GtkWidget *widget, gpointer data) {
	gtk_widget_set_sensitive(x_entry, FALSE);
	gtk_widget_set_sensitive(y_entry, FALSE);
}

void julia_option_on(GtkWidget *widget, gpointer data) {
	gtk_widget_set_sensitive(x_entry, TRUE);
	gtk_widget_set_sensitive(y_entry, TRUE);
}

//Toggle sensitivity of Hue Cycler Offset

void cycler_option_off(GtkWidget *widget, gpointer data) {
	gtk_widget_set_sensitive(hoff2_entry, FALSE);
}

void cycler_option_on(GtkWidget *widget, gpointer data) {
	gtk_widget_set_sensitive(hoff2_entry, TRUE);
}

//Function to remove display window.

void remover(GtkWidget *widget, gpointer datap) {
  PlotData *data = datap;
  gtk_widget_destroy(data->display);
   }

void h_remover(GtkWidget *widget, gpointer datap) {
  GtkWidget *data = datap;
  gtk_widget_destroy(data);
   }

//Help Window Callback

void help(GtkWidget *widget, gpointer data) {
  GtkWidget *h_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(h_window), 600, 800);
  
  GdkPixbuf *icon;
  GtkWidget *text_view;
  GtkWidget *vbox;
  GtkTextBuffer *buffer =  gtk_text_buffer_new (NULL);

  gtk_text_buffer_create_tag(buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
  gtk_text_buffer_create_tag(buffer, "large", "weight", PANGO_WEIGHT_BOLD,"size",  15 * PANGO_SCALE ,NULL);

  vbox = gtk_vbox_new(FALSE, 0);
  

  text_view = gtk_text_view_new_with_buffer (buffer);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text_view), GTK_WRAP_WORD);
  gtk_box_pack_start(GTK_BOX(vbox), text_view, TRUE, TRUE, 0);

  GtkTextIter iter;
  gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);

  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,"Manual Entry:\n\n", -1,"large");
  gtk_text_buffer_insert(buffer, &iter, "Functions must be entered using the usual arithmetic operations (all multiplication must be explicit).  Grouping sympols are limited to parentheses, and the only recognized variable name is z.  Only the functions included in the complex.h library are currently supported.\n\n", -1);
  
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,"Constants:\n",-1,"bold");
  gtk_text_buffer_insert(buffer,&iter,"i must be entered as I.\n",-1);
  gtk_text_buffer_insert(buffer,&iter,"M_PI = 3.14159...\n",-1);
  gtk_text_buffer_insert(buffer,&iter,"M_E = 2.71828...\n\n",-1);

  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,"Basic Operations:\n",-1,"bold");
  gtk_text_buffer_insert(buffer,&iter,"|z| = cabs(z)\n",-1);
  gtk_text_buffer_insert(buffer,&iter,"Arg(z) = carg(z)\n",-1);
  gtk_text_buffer_insert(buffer,&iter,"Re(z) = creal(z)\n",-1);
  gtk_text_buffer_insert(buffer,&iter,"Im(z) = cimag(z)\n",-1);
  gtk_text_buffer_insert(buffer,&iter,"conjugate of z = conj(z)\n\n",-1);

  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,"Powers:\n",-1,"bold");
  gtk_text_buffer_insert(buffer,&iter,"z^p = cpow(z,p)  (NOTE:  Explicitly typing out the multiplication may lead to faster rendering for small integer powers.)\n\n",-1);

  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,"Exponential and Logarithmic Functions:\n",-1,"bold");
  gtk_text_buffer_insert(buffer,&iter,"e^z = cexp(z)\n",-1);
  gtk_text_buffer_insert(buffer,&iter,"Log(z) = clog(z)\n\n",-1);
  
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,"Circular Trigonometric Functions: \n",-1,"bold");
  gtk_text_buffer_insert(buffer,&iter,"sin(z) = csin(z)\n",-1);
  gtk_text_buffer_insert(buffer,&iter,"cos(z) = ccos(z)\n",-1);
  gtk_text_buffer_insert(buffer,&iter,"tan(z) = ctan(z)\n",-1);
  gtk_text_buffer_insert(buffer,&iter,"arcsin(z) = casin(z)\n",-1);
  gtk_text_buffer_insert(buffer,&iter,"arccos(z) = cacos(z)\n",-1);
  gtk_text_buffer_insert(buffer,&iter,"arctan(z) = catan(z)\n\n",-1);

  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,"Hyperbolic Trigonometric Functions: \n",-1,"bold");
  gtk_text_buffer_insert(buffer,&iter,"sinh(z) = csinh(z)\n",-1);
  gtk_text_buffer_insert(buffer,&iter,"cosh(z) = ccosh(z)\n",-1);
  gtk_text_buffer_insert(buffer,&iter,"tanh(z) = ctanh(z)\n",-1);
  gtk_text_buffer_insert(buffer,&iter,"arsinh(z) = casinh(z)\n",-1);
  gtk_text_buffer_insert(buffer,&iter,"arcosh(z) = cacosh(z)\n",-1);
  gtk_text_buffer_insert(buffer,&iter,"artanh(z) = catanh(z)\n\n",-1);
  
  icon = gdk_pixbuf_new_from_file("./mand.jpeg",NULL);
  gtk_window_set_title (GTK_WINDOW (h_window), "Fractals Help");
  gtk_window_set_icon(GTK_WINDOW(h_window),icon);
  gtk_container_add(GTK_CONTAINER(h_window), vbox);
  gtk_widget_show_all(h_window);
  g_signal_connect(G_OBJECT(h_window), "destroy",G_CALLBACK(h_remover) , h_window);

}


//Functions to toggle flags in rendering data.

void man_toggle(GtkWidget *widget, gpointer datap){
  PlotData *data = datap;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(widget))) {
    data->man_flg = 1;
    gtk_widget_set_sensitive(func_entry, TRUE);
  } else {
    data->man_flg = 0;
    gtk_widget_set_sensitive(func_entry, FALSE);
  }
}

void set_toggle(GtkWidget *widget, gpointer datap) {
  PlotData *data = datap;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(widget))){
          data->set_flg = 1;
  }
  else {
          data->set_flg = 0;
  }
}

void cyc_toggle(GtkWidget *widget, gpointer datap) {
  PlotData *data = datap;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(widget))){
          data->cyc_flg = 1;
  }
  else {
          data->cyc_flg = 0;
  }
}

void rev_toggle(GtkWidget *widget, gpointer datap) {
  PlotData *data = datap;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(widget))){
          data->rev_flg = 1;
  }
  else {
          data->rev_flg = 0;
  }
}

void grid_toggle(GtkWidget *widget,gpointer datap) {
  PlotData *data = datap;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(widget))){
    if(data->axes_flg == 0){
          data->grid_flg = 1;
    }
  }
  else {
          data->grid_flg = 0;
  }
}

void axes_toggle(GtkWidget *widget,gpointer datap) {
  PlotData *data = datap;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(widget))) {
          data->axes_flg = 1;
	  data->grid_flg = 0;
  }
  else {
          data->axes_flg = 0;
  }
}

//Rendering data update function.

void update_data (GtkWidget *widget, gpointer datap) {
  PlotData *data = datap;
  
  g_free(data->fun);
  data->fun = gtk_editable_get_chars(GTK_ENTRY(func_entry),0,-1);
  data->x = atof(gtk_entry_get_text(GTK_ENTRY(x_entry)));
  data->y = atof(gtk_entry_get_text(GTK_ENTRY(y_entry)));
  data->xmin = atof(gtk_entry_get_text(GTK_ENTRY(xmin_entry)));
  data->xmax = atof(gtk_entry_get_text(GTK_ENTRY(xmax_entry)));	
  data->ymin = atof(gtk_entry_get_text(GTK_ENTRY(ymin_entry)));
  data->ymax = atof(gtk_entry_get_text(GTK_ENTRY(ymax_entry)));
  data->pix = atoi(gtk_entry_get_text(GTK_ENTRY(pix_entry)));
  data->thr = atof(gtk_entry_get_text(GTK_ENTRY(thr_entry)));
  data->iter = atoi(gtk_entry_get_text(GTK_ENTRY(iter_entry)));
  data->h_off = atof(gtk_entry_get_text(GTK_ENTRY(hoff1_entry)));
  data->cyc_off = atof(gtk_entry_get_text(GTK_ENTRY(hoff2_entry)));

  
  if (data->xmin > data->xmax) {
    data->xmin = atof(gtk_entry_get_text(GTK_ENTRY(xmax_entry)));
    data->xmax = atof(gtk_entry_get_text(GTK_ENTRY(xmin_entry)));
    
    char *tmp = g_strdup_printf("%g",data->xmax);
    gtk_entry_set_text(GTK_ENTRY(xmax_entry), tmp);
    g_free(tmp);
    
    tmp = g_strdup_printf("%g",data->xmin);
    gtk_entry_set_text(GTK_ENTRY(xmin_entry), tmp);
    g_free(tmp);
  }

  if (data->xmin == data->xmax) {
    data->xmax += 1;
    char *tmp = g_strdup_printf("%g",data->xmax);
    gtk_entry_set_text(GTK_ENTRY(xmax_entry), tmp);
    g_free(tmp);
  }

  if (data->ymin > data->ymax) {
    data->ymin = atof(gtk_entry_get_text(GTK_ENTRY(ymax_entry)));
    data->ymax = atof(gtk_entry_get_text(GTK_ENTRY(ymin_entry)));
    
    char *tmp = g_strdup_printf("%g",data->ymax);
    gtk_entry_set_text(GTK_ENTRY(ymax_entry), tmp);
    g_free(tmp);
    
    tmp = g_strdup_printf("%g",data->ymin);
    gtk_entry_set_text(GTK_ENTRY(ymin_entry), tmp);
    g_free(tmp);
  }

  if (data->ymin == data->ymax) {
    data->ymax += 1;
    
    char *tmp = g_strdup_printf("%g",data->ymax);
    gtk_entry_set_text(GTK_ENTRY(ymax_entry), tmp);
    g_free(tmp);
  }

  if (data->pix <= 0) {
    data->pix = 800;
    
    char *tmp = g_strdup_printf("%d",data->pix);
    gtk_entry_set_text(GTK_ENTRY(pix_entry), tmp);
    g_free(tmp);
  }

  if (data->thr <= 0) {
    data->thr = 2.0;
    
    char *tmp = g_strdup_printf("%g",data->thr);
    gtk_entry_set_text(GTK_ENTRY(thr_entry), tmp);
    g_free(tmp);
  }

  if (data->iter <= 0) {
    data->iter = 75;
    
    char *tmp = g_strdup_printf("%d",data->iter);
    gtk_entry_set_text(GTK_ENTRY(iter_entry), tmp);
    g_free(tmp);
  }
}

void sscale_moved(GtkRange *scale, gpointer datap) {
  PlotData *data = datap;
  double s = gtk_range_get_value(scale);
  data->sat = s;
}

//drawing functions

gboolean draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
  PlotData *dat = data;
  
  int width, height;
  GtkWidget *window = gtk_widget_get_toplevel(widget);

  FILE *src = fopen("function.c", "w");
  if (!src) {
    perror("Could not create function.c");
    return FALSE;
	}
	
  fprintf(src, "#include <math.h>\n");
  fprintf(src,"#include <complex.h>\n");
  fprintf(src, "%s\n", "double complex func(double complex z) {");
  fprintf(src, "  return %s;\n", dat->fun);
  fprintf(src, "}\n");
  fclose(src);
  
  int ret = system("gcc -fPIC -shared -o function.so function.c -lm");
  if (ret != 0) {
    fprintf(stderr, "Compilation of function.c failed.\n");
    return FALSE;
	}
	
  void *handle = dlopen("./function.so", RTLD_NOW);
  if (handle == NULL) {
    fprintf(stderr, "%s\n", dlerror());
    exit(1);
  }
  f = dlsym(handle, "func");
  if (f == NULL) {
    fprintf(stderr, "%s\n", dlerror());
    return FALSE;
  }
  
  gtk_window_get_size(GTK_WINDOW(window), &width, &height);
  draw_window(cr, width, height, data, f);

  dlclose(handle);
  unlink("function.c");
  unlink("function.so");
  return FALSE;
}

void draw_window(graphics_context gc, int width, int height, void *d,  double complex (*f) (double complex)) {
  PlotData *data = d;
  fracdat frac = {0,0,0};
  char c[256];
  double r = 0;
  double g = 0;
  double b = 0;
  
  cairo_new_path(gc);
   for (double j = 0; j <= data->pix_h; j++) {
     for (double i = 0; i <= data->pix_w; i++) {
       frac = frac_gen(data->set_flg,data->man_flg, data->iter, data->thr, xx(i,data->pix_w,data->xmin,data->xmax),yy(j,data->pix_h,data->ymin,data->ymax),data->x,data->y,f);
       colorizer(data->cyc_flg, data->rev_flg, data->iter, data->h_off, data->cyc_off, data->sat, i, j, frac.c, frac.x, frac.y, &r, &g, &b);
       cairo_rectangle(gc,i,j,1,1);
       cairo_set_source_rgb(gc,r,g,b);
       cairo_fill(gc);
     }
   }
   cairo_close_path(gc);
   cairo_stroke(gc);

   if (data->grid_flg == 1) {
     
     int xn = 5;
     int yn = 5;
     
     double num = data->xmax - data->xmin+1;
     num = num - floor(num+0.5);
     while (num >= 0.000001) {
       num = num * 10;
       xn += 1;
       num - (int) num;
     }
     if (xn < 3) {xn = 3;}

     num = data->ymax - data->ymin+1;
     num = num - floor(num+0.5);
     while (num >= 0.000001) {
       num = num * 10;
       yn += 1;
       num - (int) num;
       }
     if (yn < 3) {yn = 3;}
     
     int dx = 1;
     int dy = 1;
     double xpr = 0;
     double ypr = 0;
     for (int l = 1; l < xn; l++){dx *=10;}
     for (int l = 1; l < yn; l++){dy *=10;}
     
     double incrx = (data->xmax - data->xmin)/10;
     double incry = (data->ymax - data->ymin)/10;
     double x = data->xmin;
     double y = data->ymin;
     cairo_new_path(gc);
     cairo_set_source_rgb(gc,1,1,1);
     for (int l  = 0; l <= 10; l++) {
       cairo_move_to(gc, ii(x, data->pix_w, data->xmin, data->xmax), 0);
       cairo_line_to(gc, ii(x, data->pix_w, data->xmin, data->xmax), data->pix_h);
       	 x = x + incrx;
     }
     cairo_close_path(gc);
     cairo_stroke(gc);
     
     cairo_new_path(gc);
     cairo_set_source_rgb(gc,1,1,1);
     x = data->xmin;
       for (int l = 0; l <= 9; l++) {
       cairo_move_to(gc,ii(x, data->pix_w, data->xmin, data->xmax)+5,data->pix_h/2+20);
       xpr = floor(dx*x+0.5)/dx;
       snprintf(c,sizeof(c),"%g",xpr);
       cairo_show_text(gc,c);
       	 x = x + incrx;
     }
       cairo_move_to(gc,ii(data->xmax, data->pix_w, data->xmin, data->xmax)-20,data->pix_h/2+20);
       snprintf(c,sizeof(c),"%g",data->xmax);
       cairo_show_text(gc,c);
     cairo_close_path(gc);
     cairo_stroke(gc);
     
     cairo_new_path(gc);
     cairo_set_source_rgb(gc,1,1,1);
     for (int l = 0; l <= 10; l++) {
       cairo_move_to(gc,0,jj(y, data->pix_h, data->ymin, data->ymax));
       cairo_line_to(gc,data->pix_w,jj(y, data->pix_h, data->ymin, data->ymax));
       y = y + incry;
     }
     cairo_close_path(gc);
     cairo_stroke(gc);
     
     cairo_new_path(gc);
     cairo_set_source_rgb(gc,1,1,1);
     y = data->ymax;
     for (int l = 0; l <= 9; l++) {
       cairo_move_to(gc,data->pix_w/2-data->pix_w/20,jj(y, data->pix_h, data->ymin, data->ymax)+15);
       ypr = floor(dy*y+0.5)/dy;
       snprintf(c,sizeof(c),"%g",ypr);
       cairo_show_text(gc,c);
       y = y - incry;
       }
     cairo_move_to(gc,data->pix_w/2-data->pix_w/20,data->pix_h-15);
       snprintf(c,sizeof(c),"%g",data->ymin);
       cairo_show_text(gc,c);
     cairo_close_path(gc);
     cairo_stroke(gc);
   }

   if(data->axes_flg ==1 && data->xmin < 0 && data->xmax > 0 && data->ymin < 0 && data->ymax > 0){
     double xp = data->xmax;
     double xm = -data->xmin;
     double yp = data->ymax;
     double ym = -data->ymin;
     
     double x_inc = xp;
     if (xp < xm){x_inc = xm;};
     x_inc = x_inc/10;

     double y_inc = yp;
     if (yp < ym){y_inc = ym;};
     y_inc = y_inc/10;
     
     cairo_new_path(gc);
     cairo_set_source_rgb(gc,1,1,1);
     int i = ii(0,data->pix_w,data->xmin,data->xmax);
     int j = jj(0,data->pix_h, data->ymin, data->ymax);
     cairo_move_to(gc,i,0);
     cairo_line_to(gc,i,data->pix_h);
     cairo_move_to(gc,0,j);
     cairo_line_to(gc,data->pix_w,j);
     cairo_move_to(gc,i+5,j+15);
     cairo_show_text(gc,"0");

     double x = x_inc;
     while (x < xp) {
       cairo_move_to(gc,ii(x,data->pix_w,data->xmin,data->xmax),jj(0,data->pix_h, data->ymin, data->ymax)+5);
       cairo_line_to(gc,ii(x,data->pix_w,data->xmin,data->xmax),jj(0,data->pix_h, data->ymin, data->ymax)-5);
       cairo_move_to(gc,ii(x,data->pix_w,data->xmin,data->xmax)-5,jj(0,data->pix_h, data->ymin, data->ymax)+20);
       snprintf(c,sizeof(c),"%g",x);
       cairo_show_text(gc,c);
       x+=x_inc;
     }
     x = -x_inc;
     while (x > -xm) {
       cairo_move_to(gc,ii(x,data->pix_w,data->xmin,data->xmax),jj(0,data->pix_h, data->ymin, data->ymax)+5);
       cairo_line_to(gc,ii(x,data->pix_w,data->xmin,data->xmax),jj(0,data->pix_h, data->ymin, data->ymax)-5);
       cairo_move_to(gc,ii(x,data->pix_w,data->xmin,data->xmax)-5,jj(0,data->pix_h, data->ymin, data->ymax)+20);
       snprintf(c,sizeof(c),"%g",x);
       cairo_show_text(gc,c);
       x-=x_inc;
     }
     double y = y_inc;
     while (y < yp) {
       cairo_move_to(gc,ii(0,data->pix_w,data->xmin,data->xmax)-5,jj(y,data->pix_h, data->ymin, data->ymax));
       cairo_line_to(gc,ii(0,data->pix_w,data->xmin,data->xmax)+5,jj(y,data->pix_h, data->ymin, data->ymax));
       cairo_move_to(gc,ii(0,data->pix_w,data->xmin,data->xmax)+10,jj(y,data->pix_h, data->ymin, data->ymax));
       snprintf(c,sizeof(c),"%g",y);
       cairo_show_text(gc,c);
       y+=y_inc;
     }
     y = -y_inc;
     while (y > -ym) {
       cairo_move_to(gc,ii(0,data->pix_w,data->xmin,data->xmax)-5,jj(y,data->pix_h, data->ymin, data->ymax));
       cairo_line_to(gc,ii(0,data->pix_w,data->xmin,data->xmax)+5,jj(y,data->pix_h, data->ymin, data->ymax));
       cairo_move_to(gc,ii(0,data->pix_w,data->xmin,data->xmax)+10,jj(y,data->pix_h, data->ymin, data->ymax));
       snprintf(c,sizeof(c),"%g",y);
       cairo_show_text(gc,c);
       y-=y_inc;
     }
     cairo_close_path(gc);
     cairo_stroke(gc);
   }
}

void fill_show(GtkWidget *widget, gpointer datap){
  PlotData *data = datap;
  
  GdkPixbuf *icon;
  icon = gdk_pixbuf_new_from_file("./mand.jpeg",NULL);
  
  GtkWidget *d_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  data->display = d_window;
  
  int w = data->pix;
  int h = data->pix;

  if (data->xmax - data->xmin > data->ymax - data->ymin) {
    h = data->pix *(data->ymax - data->ymin)/(data->xmax - data->xmin);
  } else {
    w = data->pix *(data->xmax - data->xmin)/(data->ymax - data->ymin);
  }

  if (h > data->maxh){
    w = w*(data->maxh)/h;
    h = data->maxh;
  }

  if (w > data->maxw){
    h = h*(data->maxw)/w;
    w = data->maxw;
  }

  if (w > h) {
    gtk_entry_set_text(GTK_ENTRY(pix_entry), g_strdup_printf("%d",w));
  } else {
    gtk_entry_set_text(GTK_ENTRY(pix_entry), g_strdup_printf("%d",h));
  }
  data->pix_h = h;
  data->pix_w = w;
  GtkWidget *canvas = gtk_drawing_area_new();
  gtk_widget_set_size_request (canvas,w,h);
  gtk_window_set_resizable(data->display,FALSE);
  g_signal_connect(G_OBJECT(canvas), "draw", G_CALLBACK(draw), data);



  
  gtk_window_set_title (GTK_WINDOW (data->display), "Fractal Viewer");
  gtk_window_set_icon(GTK_WINDOW(data->display),icon);
  g_signal_connect(G_OBJECT(data->display), "delete-event", G_CALLBACK(gtk_widget_hide_on_delete),NULL);
  gtk_container_add (GTK_CONTAINER (data->display), canvas);
  
  gtk_widget_show_all(data->display);
  gtk_window_move(data->display,data->ctrl_w+10,0);
   }
//Start of Main

int main(int argc, char *argv[]) {

  Display *pdsp = XOpenDisplay(NULL);
  Screen *pwnd = DefaultScreenOfDisplay(pdsp);
  int Max_Width = XWidthOfScreen(pwnd);
  int Max_Height  = XHeightOfScreen(pwnd);
  XCloseDisplay(pdsp);
  
  gtk_init(0,NULL);

  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_resizable(window,FALSE);
  GtkWidget *d_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  GtkWidget *text = gtk_text_view_new();
  
  PlotData data = {0,0,Max_Width,Max_Height,-0.8,0.156,-2,1,-1,1,800,800,800,10,10,2,75,0,0,0.5,0,0.8,0,0,NULL,d_window,text};
  GdkPixbuf *icon;
  icon = gdk_pixbuf_new_from_file("./mand.jpeg",NULL);
  
  GtkWidget *box1;
  GtkWidget *separator;
  GSList *radios;
  GSList *grid_radios;
  GSList *man_radios;
  GSList *axes_radios;
  GSList *huec_radios;
  GSList *rev_radios;
  GtkWidget *button;
  GtkWidget *table;
  GtkWidget *lbl;
  GtkWidget *s_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,0,1,0.01);
  gtk_range_set_value(s_scale,0.8);

  func_entry = gtk_entry_new();
  gtk_entry_set_width_chars (func_entry,30);
  gtk_entry_set_text(GTK_ENTRY(func_entry), "cpow(z,2)");
  data.fun = gtk_editable_get_chars(GTK_ENTRY(func_entry), 0, -1);
  gtk_widget_set_sensitive(func_entry,FALSE);

 
  x_entry = gtk_entry_new();
  gtk_entry_set_width_chars (x_entry,6);
  gtk_entry_set_text(GTK_ENTRY(x_entry), "-0.8");
  gtk_widget_set_sensitive(x_entry,FALSE);
  
  y_entry = gtk_entry_new();
  gtk_entry_set_width_chars (y_entry,6);
  gtk_entry_set_text(GTK_ENTRY(y_entry), "0.156");
  gtk_widget_set_sensitive(y_entry,FALSE);
  
  xmin_entry = gtk_entry_new();
  gtk_entry_set_width_chars (xmin_entry,6);
  gtk_entry_set_text(GTK_ENTRY(xmin_entry), "-2");
  
  xmax_entry = gtk_entry_new();
  gtk_entry_set_width_chars (xmax_entry,6);
  gtk_entry_set_text(GTK_ENTRY(xmax_entry), "1");
  
  ymin_entry = gtk_entry_new();
  gtk_entry_set_width_chars (ymin_entry,6);
  gtk_entry_set_text(GTK_ENTRY(ymin_entry), "-1");
  
  ymax_entry = gtk_entry_new();
  gtk_entry_set_width_chars (ymax_entry,6);
  gtk_entry_set_text(GTK_ENTRY(ymax_entry), "1");
  
  pix_entry = gtk_entry_new();
  gtk_entry_set_width_chars (pix_entry,6);
  gtk_entry_set_text(GTK_ENTRY(pix_entry), "800");
  
  thr_entry = gtk_entry_new();
  gtk_entry_set_width_chars (thr_entry,6);
  gtk_entry_set_text(GTK_ENTRY(thr_entry), "2.0");
  
  iter_entry = gtk_entry_new();
  gtk_entry_set_width_chars (iter_entry,6);
  gtk_entry_set_text(GTK_ENTRY(iter_entry), "75");
  
  hoff1_entry = gtk_entry_new(); 
  gtk_entry_set_width_chars (hoff1_entry,6);
  gtk_entry_set_text(GTK_ENTRY(hoff1_entry), "0.0");
  
  hoff2_entry = gtk_entry_new();
  gtk_entry_set_width_chars (hoff2_entry,6);
  gtk_entry_set_text(GTK_ENTRY(hoff2_entry), "0.5");

  file_name = gtk_entry_new();
  gtk_entry_set_width_chars (file_name,6);
  gtk_entry_set_text(GTK_ENTRY(file_name), "image");
  
  
 
 //Populating control window
  
  d_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Fractal Options");
  gtk_window_set_icon(GTK_WINDOW(window),icon);

  box1 = gtk_vbox_new (FALSE,10);
  gtk_container_add (GTK_CONTAINER (window), box1);

  table = gtk_table_new (4,4,FALSE);
  gtk_table_set_row_spacings(GTK_TABLE(table), 0);
  gtk_table_set_col_spacings(GTK_TABLE(table), 0);
  
  lbl = gtk_label_new (NULL);
  gtk_label_set_markup(GTK_LABEL(lbl),"<b> Fractal Set:</b>");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 0,5,0,1);


  button = gtk_radio_button_new_with_label(NULL, "Mandelbrot Set");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
  g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(julia_option_off),NULL);
  g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(m_window_reset),NULL);
  gtk_table_attach_defaults (GTK_TABLE (table), button, 0,1,1,2);

  radios = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));

  button = gtk_radio_button_new_with_label(radios, "Julia Set");
  g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(set_toggle), &data);
  g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(julia_option_on),NULL);
  g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(j_window_reset),NULL);
  gtk_table_attach_defaults (GTK_TABLE (table), button, 0,1,2,3);

  lbl = gtk_label_new("Manual Entry:");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 1,4,1,2);
  button = gtk_radio_button_new_with_label(NULL, "Default");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
  gtk_table_attach_defaults (GTK_TABLE (table), button, 1,2,2,3);
  lbl = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(lbl), "f(z) =z<sup>2</sup>");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 2,3,2,3);

  man_radios = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));

  button = gtk_radio_button_new_with_label(man_radios, "f(z) = ");
  g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(man_toggle), &data);
  gtk_table_attach_defaults (GTK_TABLE (table), button, 1,2,3,4);
  gtk_table_attach_defaults (GTK_TABLE (table), func_entry, 2,4,3,4);

  button = gtk_button_new_with_label("Help");
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(help), NULL);
  gtk_table_attach_defaults (GTK_TABLE (table), button, 0,1,3,4);
  
    
  gtk_box_pack_start (GTK_BOX (box1), table, FALSE, TRUE, 0);

  separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_box_pack_start (GTK_BOX (box1), separator, FALSE, TRUE, 0);

  table = gtk_table_new (2,10,FALSE);
  gtk_table_set_row_spacings(GTK_TABLE(table), 0);
  gtk_table_set_col_spacings(GTK_TABLE(table), 0);
  
  lbl = gtk_label_new (NULL);
  gtk_label_set_markup(GTK_LABEL(lbl),"<b> Julia Set Only:</b>");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 0,10,0,1);

  lbl = gtk_label_new ("C = x:");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 0,1,1,2);
  gtk_table_attach_defaults (GTK_TABLE (table), x_entry, 1,2,1,2);

  lbl = gtk_label_new ("y:");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 2,3,1,2);
  gtk_table_attach_defaults (GTK_TABLE (table), y_entry, 3,4,1,2);
  
  gtk_box_pack_start (GTK_BOX (box1), table, FALSE, TRUE, 0);

  separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_box_pack_start (GTK_BOX (box1), separator, FALSE, TRUE, 0);

  table = gtk_table_new (6,5,FALSE);
  gtk_table_set_row_spacings(GTK_TABLE(table), 3);
  gtk_table_set_col_spacings(GTK_TABLE(table), 3);

  lbl = gtk_label_new (NULL);
  gtk_label_set_markup(GTK_LABEL(lbl), "<b>Display Data</b>");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 0,5,0,1);
  

  lbl = gtk_label_new ("Bounding Box:");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 0,1,1,2);
 

  lbl = gtk_label_new ("X Min = ");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 1,2,1,2);
  gtk_table_attach_defaults (GTK_TABLE (table), xmin_entry, 2,3,1,2);

  lbl = gtk_label_new ("X Max = ");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 1,2,2,3);
  gtk_table_attach_defaults (GTK_TABLE (table), xmax_entry, 2,3,2,3);

  lbl = gtk_label_new ("Y Min = ");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 3,4,1,2);
  gtk_table_attach_defaults (GTK_TABLE (table), ymin_entry, 4,5,1,2);

  lbl = gtk_label_new ("Y Max = ");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 3,4,2,3);
  gtk_table_attach_defaults (GTK_TABLE (table), ymax_entry, 4,5,2,3);

  lbl = gtk_label_new ("Max Pixels: ");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 0,1,5,6);
  gtk_table_attach_defaults (GTK_TABLE (table), pix_entry, 1,2,5,6);

  gtk_box_pack_start(GTK_BOX (box1), table, FALSE, TRUE, 0);

  separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_box_pack_start (GTK_BOX (box1), separator, FALSE, TRUE, 0);

  table = gtk_table_new (3,5,FALSE);
  gtk_table_set_row_spacings(GTK_TABLE(table), 1);
  gtk_table_set_col_spacings(GTK_TABLE(table), 1);

  lbl = gtk_label_new (NULL);
  gtk_label_set_markup(GTK_LABEL(lbl), "<b>Fractal Parameters</b>");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 0,3,0,1);

  lbl = gtk_label_new ("Threshold:");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 0,1,1,2);
  gtk_table_attach_defaults (GTK_TABLE (table), thr_entry, 1,2,1,2);
  lbl = gtk_label_new ("");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 2,3,1,2);

  lbl = gtk_label_new ("Max Iterations:");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 0,1,2,3);
  gtk_table_attach_defaults (GTK_TABLE (table), iter_entry, 1,2,2,3);
  lbl = gtk_label_new ("");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 2,3,2,3);

  gtk_box_pack_start(GTK_BOX (box1), table, FALSE, TRUE, 0);

  separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_box_pack_start (GTK_BOX (box1), separator, FALSE, TRUE, 0);


  table = gtk_table_new (8,4,FALSE);
  gtk_table_set_row_spacings(GTK_TABLE(table), 3);
  gtk_table_set_col_spacings(GTK_TABLE(table), 3);

  lbl = gtk_label_new (NULL);
  gtk_label_set_markup(GTK_LABEL(lbl), "<b>Color Options</b>");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 0,4,0,1);

  lbl = gtk_label_new ("Hue Offset:");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 0,1,1,2);
  gtk_table_attach_defaults (GTK_TABLE (table), hoff1_entry, 1,2,1,2);

  lbl = gtk_label_new ("Hue Cycler:");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 0,1,2,3);
  
  button = gtk_radio_button_new_with_label(NULL, "On");
  g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(cyc_toggle), &data);
  g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(cycler_option_on),NULL);
    gtk_table_attach_defaults (GTK_TABLE (table), button, 1,2,2,3);

  huec_radios = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));

  button = gtk_radio_button_new_with_label(huec_radios, "Off");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
  g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(cycler_option_off),NULL);
  gtk_widget_set_sensitive(hoff2_entry,FALSE);
  gtk_table_attach_defaults (GTK_TABLE (table), button, 1,2,3,4);
  
  lbl = gtk_label_new ("Cycle Offset:");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 2,3,2,3);
  gtk_table_attach_defaults (GTK_TABLE (table), hoff2_entry, 3,4,2,3);

  lbl = gtk_label_new ("Reverse Colors?");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 0,1,4,5);
  
  button = gtk_radio_button_new_with_label(NULL, "Yes");
  g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(rev_toggle), &data);
  gtk_table_attach_defaults (GTK_TABLE (table), button, 1,2,4,5);

  rev_radios = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));

  button = gtk_radio_button_new_with_label(rev_radios, "No");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
  gtk_table_attach_defaults (GTK_TABLE (table), button, 1,2,5,6);

  gtk_box_pack_start(GTK_BOX (box1), table, FALSE, TRUE, 0);

  lbl = gtk_label_new ("Color Saturation:");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 0,1,6,7);
  gtk_table_attach_defaults (GTK_TABLE (table), s_scale, 1,4,6,7);
  g_signal_connect(G_OBJECT(s_scale),"value-changed",G_CALLBACK(sscale_moved),&data);

  separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_box_pack_start (GTK_BOX (box1), separator, FALSE, TRUE, 0);

  table = gtk_table_new (4,4,TRUE);
  gtk_table_set_row_spacings(GTK_TABLE(table), 2);
  gtk_table_set_col_spacings(GTK_TABLE(table), 2);

  lbl = gtk_label_new (NULL);
  gtk_label_set_markup(GTK_LABEL(lbl), "<b>Grid</b>");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 2,3,0,1);
  
  button = gtk_radio_button_new_with_label(NULL, "On");
  g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(grid_toggle), &data);
  gtk_table_attach_defaults (GTK_TABLE (table), button, 3,4,0,1);

  grid_radios = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));

  button = gtk_radio_button_new_with_label(grid_radios, "Off");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
  gtk_table_attach_defaults (GTK_TABLE (table), button, 3,4,1,2);

  lbl = gtk_label_new (NULL);
  gtk_label_set_markup(GTK_LABEL(lbl), "<b>Axes</b>");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 0,1,0,1);

  button = gtk_radio_button_new_with_label(NULL, "On");
  g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(axes_toggle), &data);
  gtk_table_attach_defaults (GTK_TABLE (table), button, 1,2,0,1);

  axes_radios = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));

  button = gtk_radio_button_new_with_label(axes_radios, "Off");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
  gtk_table_attach_defaults (GTK_TABLE (table), button, 1,2,1,2);

  lbl = gtk_label_new (NULL);
  gtk_label_set_markup(GTK_LABEL(lbl), "<b>Note:</b> Axes display only if the origin is in the view screen.");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 0,4,2,3);

  lbl = gtk_label_new ("\"Axes On\" forces \"Grid Off\" (regardless of toggle).");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 0,4,3,4);

  gtk_box_pack_start (GTK_BOX (box1), table, FALSE, TRUE, 0);
  
  separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_box_pack_start (GTK_BOX (box1), separator, FALSE, TRUE, 0);

  table = gtk_table_new (2,5,TRUE);
  gtk_table_set_row_spacings(GTK_TABLE(table), 2);
  gtk_table_set_col_spacings(GTK_TABLE(table), 2);

  button = gtk_button_new_with_label("RENDER");
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(update_data), &data);
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(remover), &data);
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(fill_show), &data);
  gtk_table_attach_defaults (GTK_TABLE (table), button, 0,2,0,2);

  button = gtk_button_new_with_label("Print to File");
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(print2file), &data);
  gtk_table_attach_defaults (GTK_TABLE (table), button, 2,5,1,2);

  lbl = gtk_label_new ("File Name:");
  gtk_table_attach_defaults (GTK_TABLE (table), lbl, 2,3,0,1);
  gtk_table_attach_defaults (GTK_TABLE (table), file_name, 3,5,0,1);
  gtk_box_pack_end (GTK_BOX (box1), table, TRUE, TRUE, 0);

  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL);

  gtk_widget_show_all(window);
  gtk_window_move(window,0,0);

  int cw;
  int ch;
  gtk_window_get_size(window,&cw,&ch);
  data.ctrl_w = cw;
  data.ctrl_h = ch;
  
  // Initialize Rendering Window

  int w = data.pix;
  int h = data.pix;

  if (data.xmax - data.xmin > data.ymax - data.ymin) {
    h = data.pix *(data.ymax - data.ymin)/(data.xmax - data.xmin);
  } else {
    w = data.pix *(data.xmax - data.xmin)/(data.ymax - data.ymin);
  }

  if (h > data.maxh){
    w = w*(data.maxh)/h;
    h = data.maxh;
  }

  if (w > data.maxw){
    h = h*(data.maxw)/w;
    w = data.maxw;
  }

  if (w > h) {
    gtk_entry_set_text(GTK_ENTRY(pix_entry), g_strdup_printf("%d",w));
  } else {
    gtk_entry_set_text(GTK_ENTRY(pix_entry), g_strdup_printf("%d",h));
  }
  data.pix_h = h;
  data.pix_w = w;
  GtkWidget *canvas = gtk_drawing_area_new();
  gtk_widget_set_size_request (canvas,w,h);
  gtk_window_set_resizable(data.display,FALSE);
  g_signal_connect(G_OBJECT(canvas), "draw", G_CALLBACK(draw), &data);



  
  gtk_window_set_title (GTK_WINDOW (data.display), "Fractal Viewer");
  gtk_window_set_icon(GTK_WINDOW(data.display),icon);
  g_signal_connect(G_OBJECT(data.display), "delete-event", G_CALLBACK(gtk_widget_hide_on_delete),NULL);
  gtk_container_add (GTK_CONTAINER (data.display), canvas);
  
  gtk_widget_show_all(data.display);
  gtk_window_move(data.display,data.ctrl_w+10,0);
  
  gtk_main();
}
