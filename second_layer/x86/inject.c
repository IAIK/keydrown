#include <fcntl.h>
#include <gtk/gtk.h>
#include <inttypes.h>
#include <math.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "configuration.h"

static struct pollfd pfd;
static int fd;

static GdkEvent *base_event = NULL;

// ----------------------------------------------------------------------------------------------------------
size_t get_timestamp(void) {
  long ms;
  time_t s;
  struct timespec spec;

  clock_gettime(CLOCK_REALTIME, &spec);
  s = spec.tv_sec;
  ms = (long)(spec.tv_nsec / 1.0e6);

  return s * 1000 + ms;
}

// ----------------------------------------------------------------------------------------------------------
static int get_scancode(int key) {
  GdkKeymapKey *keys;
  gint n_keys;
  int hardware_keycode = 0;

  if (gdk_keymap_get_entries_for_keyval(gdk_keymap_get_default(), key, &keys,
                                        &n_keys)) {
    guint16 hardware_keycode;

    hardware_keycode = keys[0].keycode;
    g_free(keys);
  }

  return hardware_keycode;
}

// ----------------------------------------------------------------------------------------------------------
static void inject_key(GtkWidget *window, int key, int scancode, int press) {
  if (!base_event)
    return;
#if DEBUG
  size_t rip = 0;
  asm("leaq (%%rip), %0;" : "=r"(rip));
  printf("@0x%zx\n", rip);
#endif

  GdkEvent *event = gdk_event_copy(base_event);

  GdkEventKey *key_event = (GdkEventKey *)event;
  key_event->type = press ? GDK_KEY_PRESS : GDK_KEY_RELEASE;
  key_event->time = get_timestamp();
  key_event->keyval = key;
  key_event->hardware_keycode = scancode;
  gdk_threads_enter();
  gtk_main_do_event(event);
  gdk_threads_leave();

  gdk_event_free(event);
}

// ----------------------------------------------------------------------------------------------------------
#if DEBUG
static void dump_event(GdkEventKey *event) {
  g_print("type: %d\n", event->type);
  g_print("window: %p\n", event->window);
  g_print("send_event: %d\n", event->send_event);
  g_print("time: %d\n", event->time);
  g_print("state: %d\n", event->state);
  g_print("keyval: %d\n", event->keyval);
  g_print("length: %d\n", event->length);
  g_print("hardware_keycode: %d\n", event->hardware_keycode);
  g_print("group: %d\n", event->group);
  g_print("is_modifier: %d\n", event->is_modifier);
}
#endif

// ----------------------------------------------------------------------------------------------------------
static gboolean snooper(GtkWidget *widget, GdkEventKey *event, gpointer data) {
#if DEBUG
  g_print("0x%x\n", event->keyval);
#endif

  if (event->window && !base_event) {
    base_event = gdk_event_copy((const GdkEvent *)event);

#if !DEBUG
    gtk_widget_hide(widget);
#endif
  }
#if DEBUG
  dump_event(event);
#endif

  return FALSE;
}

// ----------------------------------------------------------------------------------------------------------
void *inject_thread(void *arg) {
  GtkWidget *window = (GtkWidget *)arg;

  int range = rand() % 2;
  int range1 = GDK_KEY_a + rand() % (GDK_KEY_z - GDK_KEY_a + 1);
  int range2 = GDK_KEY_A + rand() % (GDK_KEY_Z - GDK_KEY_A + 1);

  int scan = 0;
  int runs = rand() % 100;

  for (int i = 0; i < runs; i++) {
    inject_key(NULL, range1 * range + range2 * (1 - range), scan, 1);
    usleep(rand() % 100000);
    inject_key(NULL, range1 * range + range2 * (1 - range), scan, 0);
    if (poll(&pfd, 1, 100000) > 0) {
      if (pfd.revents & POLLIN) {
#if DEBUG
        printf("Key\n");
#endif
        char tmp[1024];
        read(fd, tmp, 1024);
      }
    }
  }
}
static void _inject_thread_end() { asm volatile("nop"); }

// ----------------------------------------------------------------------------------------------------------
void *inject_thread_wrapper(void *arg) {
  while (1) {
    inject_thread(arg);
  }
}

// ----------------------------------------------------------------------------------------------------------
int main(int argc, char **argv) {
  fd = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
  pfd.fd = fd;
  pfd.events = POLLIN;

  pthread_t injector;
  GtkWidget *window;
  GtkWidget *textbox;

  srand(time(NULL));

  gdk_threads_init();
  gtk_init(&argc, &argv);
  gtk_key_snooper_install((GtkKeySnoopFunc)snooper, 0);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 60);

  textbox = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(textbox),
                     "Please enter a character with your keyboard");

  gtk_container_add(GTK_CONTAINER(window), textbox);

  g_signal_connect(window, "delete-event", G_CALLBACK(gtk_main_quit), 0);

  gtk_widget_show_all(window);

  pthread_create(&injector, NULL, inject_thread_wrapper, (void *)textbox);

  gtk_main();

  return 0;
}
