/* Waybar CFFI v2 hover mixer. All PulseAudio/GTK callbacks share GTK's main loop.
 * ABI: Alexays/Waybar resources/custom_modules/cffi_example/waybar_cffi_module.h
 * Uses PipeWire's PulseAudio compatibility server; no shell commands or polling.
 */
#include <gtk/gtk.h>
#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>
#include <stdint.h>
#include <playerctl/playerctl.h>
#include <gtk-layer-shell.h>
typedef struct wbcffi_module wbcffi_module;
typedef struct {
    wbcffi_module *obj; const char *waybar_version;
    GtkContainer *(*get_root_widget)(wbcffi_module *);
    void (*queue_update)(wbcffi_module *);
} InitInfo;
typedef struct { const char *key, *value; } ConfigEntry;
const size_t wbcffi_version = 2;
typedef struct Mixer Mixer;
typedef struct {
    Mixer *m; GtkWidget *box, *title, *scale, *mute, *percent;
    uint32_t index; gboolean master, updating, muted; pa_cvolume volume;
} Row;
struct Mixer {
    GtkWidget *anchor, *label, *popover, *rows_box, *empty, *scroll, *media_box;
    PlayerctlPlayerManager *players; GHashTable *media_rows;
    guint max_volume;
    GHashTable *rows; Row *master;
    pa_glib_mainloop *loop; pa_context *context;
    guint close_timer, open_timer, reconnect_timer;
    gboolean over_anchor, over_popup, stopping;
};
static void request_server(Mixer *m);
static void connect_audio(Mixer *m);
static void done(pa_operation *op) { if (op) pa_operation_unref(op); }
static gboolean ready(Mixer *m) { return m->context && pa_context_get_state(m->context) == PA_CONTEXT_READY; }
static guint percent(const pa_cvolume *v) { return (guint)((100ULL * pa_cvolume_max(v) + PA_VOLUME_NORM / 2) / PA_VOLUME_NORM); }
static void volume_changed(GtkRange *range, Row *r) {
    if (r->updating || !ready(r->m) || !pa_cvolume_valid(&r->volume)) return;
    pa_cvolume v = r->volume;
    pa_cvolume_scale(&v, (pa_volume_t)(gtk_range_get_value(range) * PA_VOLUME_NORM / 100.0));
    if (r->master) done(pa_context_set_sink_volume_by_index(r->m->context, r->index, &v, NULL, NULL));
    else done(pa_context_set_sink_input_volume(r->m->context, r->index, &v, NULL, NULL));
    char text[24]; g_snprintf(text, sizeof text, "%.0f%%", gtk_range_get_value(range));
    gtk_label_set_text(GTK_LABEL(r->percent), text);
}
static void mute_clicked(GtkButton *button, Row *r) {
    if (!ready(r->m)) return;
    if (r->master) done(pa_context_set_sink_mute_by_index(r->m->context, r->index, !r->muted, NULL, NULL));
    else done(pa_context_set_sink_input_mute(r->m->context, r->index, !r->muted, NULL, NULL));
}
static Row *row_new(Mixer *m, uint32_t index, gboolean master) {
    Row *r = g_new0(Row, 1); r->m = m; r->index = index; r->master = master;
    r->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_style_context_add_class(gtk_widget_get_style_context(r->box), "mixer-row");
    r->title = gtk_label_new(""); gtk_label_set_xalign(GTK_LABEL(r->title), 0);
    gtk_label_set_ellipsize(GTK_LABEL(r->title), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars(GTK_LABEL(r->title), 34);
    gtk_box_pack_start(GTK_BOX(r->box), r->title, FALSE, FALSE, 0);
    GtkWidget *line = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    r->mute = gtk_button_new_with_label(""); gtk_widget_set_tooltip_text(r->mute, "Mute / unmute");
    r->scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, m->max_volume, 1);
    gtk_scale_add_mark(GTK_SCALE(r->scale), 100, GTK_POS_BOTTOM, NULL);
    gtk_scale_set_draw_value(GTK_SCALE(r->scale), FALSE); gtk_widget_set_hexpand(r->scale, TRUE);
    gtk_widget_set_size_request(r->scale, 210, -1);
    r->percent = gtk_label_new("0%"); gtk_label_set_width_chars(GTK_LABEL(r->percent), 5);
    gtk_box_pack_start(GTK_BOX(line), r->mute, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(line), r->scale, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(line), r->percent, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(r->box), line, FALSE, FALSE, 0);
    g_signal_connect(r->scale, "value-changed", G_CALLBACK(volume_changed), r);
    g_signal_connect(r->mute, "clicked", G_CALLBACK(mute_clicked), r);
    return r;
}
static void row_free(gpointer data) { Row *r = data; gtk_widget_destroy(r->box); g_free(r); }
static void row_update(Row *r, const char *title, const pa_cvolume *v, gboolean muted) {
    r->volume = *v; r->muted = muted; r->updating = TRUE;
    gtk_label_set_text(GTK_LABEL(r->title), title); gtk_widget_set_tooltip_text(r->title, title);
    if (!(gtk_widget_get_state_flags(r->scale) & GTK_STATE_FLAG_ACTIVE))
        gtk_range_set_value(GTK_RANGE(r->scale), MIN(percent(v), r->m->max_volume));
    char text[24]; g_snprintf(text, sizeof text, "%u%%", percent(v));
    gtk_label_set_text(GTK_LABEL(r->percent), text);
    GtkStyleContext *style = gtk_widget_get_style_context(r->percent);
    if (percent(v) > 100) gtk_style_context_add_class(style, "boost");
    else gtk_style_context_remove_class(style, "boost");
    gtk_button_set_label(GTK_BUTTON(r->mute), muted ? "󰝟" : ""); r->updating = FALSE;
}
static void update_empty(Mixer *m) {
    guint count = g_hash_table_size(m->rows);
    gtk_widget_set_visible(m->empty, count == 0);
    gtk_widget_set_visible(m->scroll, count > 0);
    gtk_widget_set_size_request(m->scroll, -1, (int)MIN(count, 4) * 78);
}
static void stream_info(pa_context *c, const pa_sink_input_info *info, int eol, void *data) {
    Mixer *m = data; if (eol || !info) return;
    Row *r = g_hash_table_lookup(m->rows, GUINT_TO_POINTER(info->index));
    if (!r) {
        r = row_new(m, info->index, FALSE); g_hash_table_insert(m->rows, GUINT_TO_POINTER(info->index), r);
        gtk_box_pack_start(GTK_BOX(m->rows_box), r->box, FALSE, FALSE, 0); gtk_widget_show_all(r->box);
    }
    const char *app = pa_proplist_gets(info->proplist, PA_PROP_APPLICATION_NAME);
    const char *media = pa_proplist_gets(info->proplist, PA_PROP_MEDIA_TITLE);
    if (!media || !*media) media = pa_proplist_gets(info->proplist, PA_PROP_MEDIA_NAME);
    gboolean generic = !media || !*media || !g_ascii_strcasecmp(media, "Playback");
    /* Chromium's generic streams have no tab identifier; never guess a mapping. */
    char *title = generic ? g_strdup_printf("%s · Stream #%u", app ? app : "Audio", info->index)
        : (app && g_strcmp0(media, app)) ? g_strdup_printf("%s · %s", app, media)
        : g_strdup(media);
    row_update(r, title, &info->volume, info->mute); g_free(title); update_empty(m);
}
static void sink_info(pa_context *c, const pa_sink_info *info, int eol, void *data) {
    Mixer *m = data; if (eol || !info) return;
    m->master->index = info->index; row_update(m->master, "Master volume", &info->volume, info->mute);
    gtk_widget_set_sensitive(m->master->box, TRUE);
    char text[32];
    if (info->mute) g_strlcpy(text, "󰝟 Muted", sizeof text);
    else g_snprintf(text, sizeof text, " %u%%", percent(&info->volume));
    gtk_label_set_text(GTK_LABEL(m->label), text);
}
static void server_info(pa_context *c, const pa_server_info *info, void *data) {
    if (info && info->default_sink_name) done(pa_context_get_sink_info_by_name(c, info->default_sink_name, sink_info, data));
}
static void request_server(Mixer *m) { if (ready(m)) done(pa_context_get_server_info(m->context, server_info, m)); }
static void subscribed(pa_context *c, pa_subscription_event_type_t type, uint32_t index, void *data) {
    Mixer *m = data; int facility = type & PA_SUBSCRIPTION_EVENT_FACILITY_MASK;
    if (facility == PA_SUBSCRIPTION_EVENT_SINK_INPUT) {
        if ((type & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
            g_hash_table_remove(m->rows, GUINT_TO_POINTER(index)); update_empty(m);
        } else done(pa_context_get_sink_input_info(c, index, stream_info, m));
    } else request_server(m);
}
static gboolean reconnect(gpointer data) { Mixer *m = data; m->reconnect_timer = 0; connect_audio(m); return G_SOURCE_REMOVE; }
static void context_state(pa_context *context, void *data) {
    Mixer *m = data;
    switch (pa_context_get_state(context)) {
    case PA_CONTEXT_READY:
        gtk_label_set_text(GTK_LABEL(m->empty), "No apps are playing audio");
        pa_context_set_subscribe_callback(context, subscribed, m);
        done(pa_context_subscribe(context, PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SINK_INPUT | PA_SUBSCRIPTION_MASK_SERVER, NULL, NULL));
        request_server(m); done(pa_context_get_sink_input_info_list(context, stream_info, m)); break;
    case PA_CONTEXT_FAILED:
    case PA_CONTEXT_TERMINATED:
        gtk_label_set_text(GTK_LABEL(m->label), "󰝟 N/A"); gtk_widget_set_sensitive(m->master->box, FALSE);
        g_hash_table_remove_all(m->rows);
        gtk_label_set_text(GTK_LABEL(m->empty), "Audio service unavailable · reconnecting…"); update_empty(m);
        if (!m->stopping && !m->reconnect_timer) m->reconnect_timer = g_timeout_add_seconds(3, reconnect, m);
        break;
    default: break;
    }
}
static void connect_audio(Mixer *m) {
    if (m->context) {
        pa_context_set_state_callback(m->context, NULL, NULL); pa_context_set_subscribe_callback(m->context, NULL, NULL);
        pa_context_disconnect(m->context); pa_context_unref(m->context);
    }
    m->context = pa_context_new(pa_glib_mainloop_get_api(m->loop), "Waybar hover mixer");
    pa_context_set_state_callback(m->context, context_state, m);
    if (pa_context_connect(m->context, NULL, PA_CONTEXT_NOFAIL, NULL) < 0 && !m->reconnect_timer)
        m->reconnect_timer = g_timeout_add_seconds(3, reconnect, m);
}
static void show_popup(Mixer *m) {
    GtkWidget *bar = gtk_widget_get_toplevel(m->anchor);
    GdkMonitor *monitor = gdk_display_get_monitor_at_window(gtk_widget_get_display(bar), gtk_widget_get_window(bar));
    gtk_layer_set_monitor(GTK_WINDOW(m->popover), monitor);
    GdkRectangle geometry; gdk_monitor_get_geometry(monitor, &geometry);
    int x = 0, y = 0;
    gtk_widget_translate_coordinates(m->anchor, bar, 0, 0, &x, &y);
    int bar_x = gtk_layer_get_margin(GTK_WINDOW(bar), GTK_LAYER_SHELL_EDGE_LEFT);
    if (!gtk_layer_get_anchor(GTK_WINDOW(bar), GTK_LAYER_SHELL_EDGE_LEFT) &&
        gtk_layer_get_anchor(GTK_WINDOW(bar), GTK_LAYER_SHELL_EDGE_RIGHT))
        bar_x = geometry.width - gtk_widget_get_allocated_width(bar) - gtk_layer_get_margin(GTK_WINDOW(bar), GTK_LAYER_SHELL_EDGE_RIGHT);
    x += bar_x;
    GtkRequisition minimum, natural;
    gtk_widget_get_preferred_size(m->popover, &minimum, &natural);
    int width = MAX(360, natural.width);
    int right = MAX(8, geometry.width - (x + gtk_widget_get_allocated_width(m->anchor) / 2 + width / 2));
    right = MIN(right, MAX(8, geometry.width - width - 8));
    gtk_layer_set_margin(GTK_WINDOW(m->popover), GTK_LAYER_SHELL_EDGE_RIGHT, right);
    gtk_layer_set_margin(GTK_WINDOW(m->popover), GTK_LAYER_SHELL_EDGE_TOP,
        gtk_layer_get_margin(GTK_WINDOW(bar), GTK_LAYER_SHELL_EDGE_TOP) + gtk_widget_get_allocated_height(bar) + 2);
    gtk_widget_show(m->popover);
    update_empty(m);
}
static gboolean close_popup(gpointer data) {
    Mixer *m = data; m->close_timer = 0;
    if (!m->over_anchor && !m->over_popup) gtk_widget_hide(m->popover);
    return G_SOURCE_REMOVE;
}
static gboolean open_popup(gpointer data) {
    Mixer *m = data; m->open_timer = 0;
    if (m->over_anchor) show_popup(m);
    return G_SOURCE_REMOVE;
}
static gboolean entered(GtkWidget *widget, GdkEventCrossing *event, Mixer *m) {
    if (event->detail == GDK_NOTIFY_INFERIOR) return FALSE;
    if (widget == m->anchor) m->over_anchor = TRUE; else m->over_popup = TRUE;
    if (m->close_timer) { g_source_remove(m->close_timer); m->close_timer = 0; }
    if (widget == m->anchor && !m->open_timer) m->open_timer = g_timeout_add(180, open_popup, m);
    return FALSE;
}
static gboolean left(GtkWidget *widget, GdkEventCrossing *event, Mixer *m) {
    if (event->detail == GDK_NOTIFY_INFERIOR) return FALSE;
    if (widget == m->anchor) m->over_anchor = FALSE; else m->over_popup = FALSE;
    if (m->open_timer) { g_source_remove(m->open_timer); m->open_timer = 0; }
    if (!m->close_timer) m->close_timer = g_timeout_add(450, close_popup, m);
    return FALSE;
}
static gboolean clicked(GtkWidget *widget, GdkEventButton *event, Mixer *m) {
    if (event->button == 3) mute_clicked(NULL, m->master);
    else if (event->button == 1) show_popup(m);
    return TRUE;
}
static gboolean scrolled(GtkWidget *widget, GdkEventScroll *event, Mixer *m) {
    if (!ready(m) || !pa_cvolume_valid(&m->master->volume)) return TRUE;
    double change = 0;
    if (event->direction == GDK_SCROLL_UP) change = 2;
    else if (event->direction == GDK_SCROLL_DOWN) change = -2;
    else if (event->direction == GDK_SCROLL_SMOOTH) change = -event->delta_y * 2;
    gtk_range_set_value(GTK_RANGE(m->master->scale), CLAMP((double)percent(&m->master->volume) + change, 0, m->max_volume));
    return TRUE;
}
/* MPRIS titles are player-wide context, separate from the targeted audio streams. */
typedef struct { GtkWidget *box, *heading, *title; } MediaRow;
static void media_free(gpointer data) {
    MediaRow *r = data; gtk_widget_destroy(r->box); g_free(r);
}
static void media_visibility(Mixer *m) {
    GHashTableIter iter; gpointer value; gboolean visible = FALSE;
    g_hash_table_iter_init(&iter, m->media_rows);
    while (g_hash_table_iter_next(&iter, NULL, &value))
        visible |= gtk_widget_get_visible(((MediaRow *)value)->box);
    gtk_widget_set_visible(m->media_box, visible);
}
static void media_refresh(PlayerctlPlayer *player, Mixer *m) {
    MediaRow *r = g_hash_table_lookup(m->media_rows, player); if (!r) return;
    GVariant *metadata = NULL; char *name = NULL;
    PlayerctlPlaybackStatus status;
    g_object_get(player, "metadata", &metadata, "player-name", &name, "playback-status", &status, NULL);
    const char *title = NULL;
    if (metadata) g_variant_lookup(metadata, "xesam:title", "&s", &title);
    if (title && *title && status != PLAYERCTL_PLAYBACK_STATUS_STOPPED) {
        char *heading = g_strdup_printf("%s · %s", status == PLAYERCTL_PLAYBACK_STATUS_PLAYING ? "Now playing" : "Paused", name ? name : "Media");
        gtk_label_set_text(GTK_LABEL(r->heading), heading); g_free(heading);
        gtk_label_set_text(GTK_LABEL(r->title), title);
        gtk_widget_set_tooltip_text(r->title, title);
        gtk_widget_show(r->box);
    } else gtk_widget_hide(r->box);
    if (metadata) g_variant_unref(metadata);
    g_free(name); media_visibility(m);
}
static void media_metadata(PlayerctlPlayer *player, GVariant *metadata, Mixer *m) { media_refresh(player, m); }
static void media_status(PlayerctlPlayer *player, PlayerctlPlaybackStatus status, Mixer *m) { media_refresh(player, m); }
static void media_appeared(PlayerctlPlayerManager *manager, PlayerctlPlayerName *name, Mixer *m) {
    GError *error = NULL;
    PlayerctlPlayer *player = playerctl_player_new_from_name(name, &error);
    if (!player) { g_clear_error(&error); return; }
    MediaRow *r = g_new0(MediaRow, 1);
    r->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    r->heading = gtk_label_new(""); r->title = gtk_label_new("");
    gtk_label_set_xalign(GTK_LABEL(r->heading), 0); gtk_label_set_xalign(GTK_LABEL(r->title), 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(r->heading), "mixer-media-heading");
    gtk_label_set_ellipsize(GTK_LABEL(r->title), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars(GTK_LABEL(r->title), 36);
    gtk_widget_set_tooltip_text(r->heading, "Player's current media title. Browser media sessions may cover multiple audio streams.");
    gtk_box_pack_start(GTK_BOX(r->box), r->heading, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(r->box), r->title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(m->media_box), r->box, FALSE, FALSE, 0);
    g_hash_table_insert(m->media_rows, player, r);
    g_signal_connect(player, "metadata", G_CALLBACK(media_metadata), m);
    g_signal_connect(player, "playback-status", G_CALLBACK(media_status), m);
    playerctl_player_manager_manage_player(manager, player);
    gtk_widget_show_all(r->box); media_refresh(player, m);
    g_object_unref(player);
}
static void media_vanished(PlayerctlPlayerManager *manager, PlayerctlPlayer *player, Mixer *m) {
    g_signal_handlers_disconnect_by_data(player, m);
    g_hash_table_remove(m->media_rows, player); media_visibility(m);
}
static void connect_media(Mixer *m) {
    m->media_rows = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, media_free);
    GError *error = NULL; m->players = playerctl_player_manager_new(&error);
    if (!m->players) { g_clear_error(&error); return; }
    g_signal_connect(m->players, "name-appeared", G_CALLBACK(media_appeared), m);
    g_signal_connect(m->players, "player-vanished", G_CALLBACK(media_vanished), m);
    GList *names = NULL; g_object_get(m->players, "player-names", &names, NULL);
    /* The property is a borrowed list; managing players does not modify it. */
    for (GList *it = names; it; it = it->next) media_appeared(m->players, it->data, m);
}

void *wbcffi_init(const InitInfo *info, const ConfigEntry *entries, size_t len) {
    Mixer *m = g_new0(Mixer, 1); m->max_volume = 150;
    for (size_t i = 0; i < len; i++) {
        if (!g_strcmp0(entries[i].key, "max-volume")) {
            char *end = NULL; guint64 max = g_ascii_strtoull(entries[i].value, &end, 10);
            if (end != entries[i].value && max >= 100 && max <= 200) m->max_volume = (guint)max;
        }
    }
    m->rows = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, row_free);
    m->anchor = gtk_event_box_new(); gtk_widget_set_name(m->anchor, "audio-mixer");
    m->label = gtk_label_new(" …"); gtk_container_add(GTK_CONTAINER(m->anchor), m->label);
    gtk_container_add(info->get_root_widget(info->obj), m->anchor);
    m->popover = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_decorated(GTK_WINDOW(m->popover), FALSE);
    gtk_widget_set_name(m->popover, "audio-mixer-popup");
    gtk_layer_init_for_window(GTK_WINDOW(m->popover));
    gtk_layer_set_layer(GTK_WINDOW(m->popover), GTK_LAYER_SHELL_LAYER_OVERLAY);
    gtk_layer_set_namespace(GTK_WINDOW(m->popover), "waybar-audio-mixer");
    gtk_layer_set_anchor(GTK_WINDOW(m->popover), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(m->popover), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
    /* Position from the bar ourselves; do not add its reserved height twice. */
    gtk_layer_set_exclusive_zone(GTK_WINDOW(m->popover), -1);
    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_container_set_border_width(GTK_CONTAINER(content), 14); gtk_container_add(GTK_CONTAINER(m->popover), content);
    GtkWidget *heading = gtk_label_new("Audio mixer"); gtk_label_set_xalign(GTK_LABEL(heading), 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(heading), "mixer-heading");
    gtk_box_pack_start(GTK_BOX(content), heading, FALSE, FALSE, 0);
    m->master = row_new(m, PA_INVALID_INDEX, TRUE); gtk_widget_set_sensitive(m->master->box, FALSE);
    gtk_box_pack_start(GTK_BOX(content), m->master->box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 0);
    m->media_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_no_show_all(m->media_box, TRUE);
    gtk_box_pack_start(GTK_BOX(content), m->media_box, FALSE, FALSE, 0);
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL); m->scroll = scroll;
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(scroll), TRUE);
    gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(scroll), 360);
    m->rows_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12); gtk_container_add(GTK_CONTAINER(scroll), m->rows_box);
    gtk_box_pack_start(GTK_BOX(content), scroll, TRUE, TRUE, 0);
    m->empty = gtk_label_new("Connecting to audio service…"); gtk_box_pack_start(GTK_BOX(content), m->empty, FALSE, FALSE, 0);
    gtk_widget_show_all(content); gtk_widget_show_all(m->anchor);
    for (int i = 0; i < 2; i++) {
        GtkWidget *widget = i ? m->popover : m->anchor;
        gtk_widget_add_events(widget, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
        g_signal_connect(widget, "enter-notify-event", G_CALLBACK(entered), m);
        g_signal_connect(widget, "leave-notify-event", G_CALLBACK(left), m);
    }
    gtk_widget_add_events(m->anchor, GDK_BUTTON_PRESS_MASK | GDK_SCROLL_MASK | GDK_SMOOTH_SCROLL_MASK);
    g_signal_connect(m->anchor, "button-press-event", G_CALLBACK(clicked), m);
    g_signal_connect(m->anchor, "scroll-event", G_CALLBACK(scrolled), m);
    connect_media(m);
    m->loop = pa_glib_mainloop_new(NULL); connect_audio(m); return m;
}
void wbcffi_deinit(void *instance) {
    Mixer *m = instance; if (!m) return; m->stopping = TRUE;
    if (m->open_timer) g_source_remove(m->open_timer);
    if (m->close_timer) g_source_remove(m->close_timer);
    if (m->reconnect_timer) g_source_remove(m->reconnect_timer);
    if (m->players) {
        GHashTableIter iter; gpointer player;
        g_hash_table_iter_init(&iter, m->media_rows);
        while (g_hash_table_iter_next(&iter, &player, NULL)) g_signal_handlers_disconnect_by_data(player, m);
        g_signal_handlers_disconnect_by_data(m->players, m);
        g_object_unref(m->players);
    }
    g_hash_table_destroy(m->media_rows);
    pa_context_set_state_callback(m->context, NULL, NULL); pa_context_set_subscribe_callback(m->context, NULL, NULL);
    pa_context_disconnect(m->context); pa_context_unref(m->context); pa_glib_mainloop_free(m->loop);
    g_hash_table_destroy(m->rows); row_free(m->master);
    gtk_widget_destroy(m->popover); gtk_widget_destroy(m->anchor); g_free(m);
}

void wbcffi_update(void *instance) { /* Server subscriptions update widgets directly. */ }
void wbcffi_refresh(void *instance, int signal_number) { request_server(instance); }
void wbcffi_doaction(void *instance, const char *action) { /* No shell action dispatch. */ }
