/* See LICENSE file for license and copyright information */

#define _GNU_SOURCE

#include <dlfcn.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include <gtk/gtk.h>

/* Typedefs */
typedef GtkWidget *(*gtk_text_view_new_t)(void);

/* Resolved symbols */
static gtk_text_view_new_t gtk_text_view_new_real;

/* Forward declarations */
static void init(void) __attribute__((constructor));
static void cleanup(void) __attribute__((destructor));

/* Implementation */
static void init(void) {
  /* Resolve symbols */
  *(void **)(&gtk_text_view_new_real) = dlsym(RTLD_NEXT, "gtk_text_view_new");
  if (gtk_text_view_new_real == NULL) {
    fprintf(stderr, "[error] Could not resolve 'gtk_text_view_new'\n");
    return;
  }
}

static void cleanup(void) {}

gboolean cb_gtk_text_view_key_press_event(GtkWidget *widget, GdkEvent *event,
                                          gpointer user_data) {
  (void)event;
  (void)user_data;

  GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));

  GtkTextIter start;
  GtkTextIter end;
  gtk_text_buffer_get_start_iter(buffer, &start);
  gtk_text_buffer_get_end_iter(buffer, &end);
  gtk_text_buffer_get_text(buffer, &start, &end, TRUE);

  return FALSE;
}

GtkWidget *gtk_text_view_new(void) {
  /* Create real widget */
  GtkWidget *entry = gtk_text_view_new_real();

  /* Watch visibility property */
  g_signal_connect(entry, "key-press-event",
                   G_CALLBACK(cb_gtk_text_view_key_press_event), NULL);

  return entry;
}
