/* Integration checks use only a disposable sink-input identified by media.name. */
#include "mixer.c"
#include <gtk-layer-shell.h>
static Mixer *test_m;
static GtkWidget *root, *window;
static guint phase, ticks;
static gboolean expected_mute;
static GtkContainer *get_root(wbcffi_module *obj) { return GTK_CONTAINER(root); }
static gboolean check(gpointer data) {
    if (++ticks > 120) { g_printerr("Timed out at phase %u\n", phase); exit(1); }
    Row *row = NULL; GHashTableIter iter; gpointer value;
    g_hash_table_iter_init(&iter, test_m->rows);
    while (g_hash_table_iter_next(&iter, NULL, &value)) {
        Row *r = value;
        if (strstr(gtk_label_get_text(GTK_LABEL(r->title)), "Disposable mixer test")) row = r;
    }
    if (!row) return G_SOURCE_CONTINUE;
    if (ticks % 20 == 0) g_print("phase=%u stream=%u volume=%u muted=%d popup=%d\n", phase, row->index, percent(&row->volume), row->muted, gtk_widget_get_visible(test_m->popover));
    if (phase == 0) {
        g_assert_true(ready(test_m));
        GdkEventCrossing event = { .detail = GDK_NOTIFY_NONLINEAR };
        entered(test_m->anchor, &event, test_m); phase++;
    } else if (phase == 1) {
        if (!gtk_widget_get_visible(test_m->popover)) return G_SOURCE_CONTINUE;
        GdkEventCrossing event = { .detail = GDK_NOTIFY_NONLINEAR };
        entered(test_m->popover, &event, test_m); left(test_m->anchor, &event, test_m);
        gtk_range_set_value(GTK_RANGE(row->scale), 125); phase++;
    } else if (phase == 2) {
        if (percent(&row->volume) != 125) return G_SOURCE_CONTINUE;
        expected_mute = !row->muted; gtk_button_clicked(GTK_BUTTON(row->mute)); phase++;
    } else if (phase == 3) {
        if (row->muted != expected_mute) return G_SOURCE_CONTINUE;
        g_assert_true(gtk_widget_get_visible(test_m->popover));
        g_print("Hover opened popup; entering popup kept it open; test stream volume 125%% and mute confirmed by server.\n");
        phase++;
    } else if (phase == 4) {
        /* Synthetic crossing events must not depend on the user's real pointer. */
        GdkEventCrossing hold = { .detail = GDK_NOTIFY_NONLINEAR };
        entered(test_m->popover, &hold, test_m);
        if (!gtk_widget_get_visible(test_m->popover)) show_popup(test_m);
        /* Hold the verified popup briefly for a screenshot. */
        if (ticks < 60) return G_SOURCE_CONTINUE;
        GdkEventCrossing event = { .detail = GDK_NOTIFY_NONLINEAR };
        left(test_m->popover, &event, test_m); phase++;
    } else if (phase == 5) {
        if (gtk_widget_get_visible(test_m->popover)) return G_SOURCE_CONTINUE;
        wbcffi_deinit(test_m);
        InitInfo info = { .get_root_widget = get_root };
        test_m = wbcffi_init(&info, NULL, 0); phase++;
    } else {
        g_assert_cmpuint(percent(&row->volume), ==, 125);
        wbcffi_deinit(test_m); gtk_widget_destroy(window);
        g_print("Leave closed popup; module teardown and reinitialization passed.\n");
        gtk_main_quit(); return G_SOURCE_REMOVE;
    }
    return G_SOURCE_CONTINUE;
}
int main(int argc, char **argv) {
    gtk_init(&argc, &argv);
    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_path(css, "../style.css", NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(css), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(css);
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_layer_init_for_window(GTK_WINDOW(window));
    gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_OVERLAY);
    gtk_layer_set_namespace(GTK_WINDOW(window), "audio-mixer-test");
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
    gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, 48);
    gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, 700);
    root = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); gtk_container_add(GTK_CONTAINER(window), root);
    InitInfo info = { .get_root_widget = get_root };
    test_m = wbcffi_init(&info, NULL, 0);
    gtk_widget_show_all(window); g_timeout_add(100, check, NULL); gtk_main(); return 0;
}
