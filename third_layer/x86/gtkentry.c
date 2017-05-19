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
typedef GtkWidget *(*gtk_entry_new_t)(void);

/* Resolved symbols */
static gtk_entry_new_t gtk_entry_new_real;

/* Forward declarations */
static void init(void) __attribute__((constructor));
static void cleanup(void) __attribute__((destructor));

/* Implementation */
static void init(void) {
  /* Resolve symbols */
  *(void **)(&gtk_entry_new_real) = dlsym(RTLD_NEXT, "gtk_entry_new");
  if (gtk_entry_new_real == NULL) {
    fprintf(stderr, "[error] Could not resolve 'gtk_entry_new'\n");
    return;
  } else {
    fprintf(stderr, "[info] KeyDrown active\n");
  }
}

static void cleanup(void) {}

gboolean cb_gtk_entry_key_press_event(GtkWidget *widget, GdkEvent *event,
                                      gpointer user_data) {
  (void)event;
  (void)user_data;

  gtk_entry_get_text(GTK_ENTRY(widget));

  return FALSE;
}

GtkWidget *gtk_entry_new(void) {
  /* Create real widget */
  GtkWidget *entry = gtk_entry_new_real();

  /* Watch visibility property */
  g_signal_connect(entry, "key-press-event",
                   G_CALLBACK(cb_gtk_entry_key_press_event), NULL);

  return entry;
}
