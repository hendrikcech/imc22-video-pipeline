#include "gst.h"

#include <gst/app/gstappsrc.h>

GMainLoop *gstreamer_receive_main_loop = NULL;
void gstreamer_receive_start_mainloop(void) {
  gstreamer_receive_main_loop = g_main_loop_new(NULL, FALSE);

  g_main_loop_run(gstreamer_receive_main_loop);
}

static gboolean gstreamer_receive_bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS: {
        goHandleReceiveEOS();
        break;
    }

    case GST_MESSAGE_ERROR: {
        gchar *debug;
        GError *error;

        gst_message_parse_error(msg, &error, &debug);
        g_free(debug);

        g_printerr("Error: %s\n", error->message);
        g_error_free(error);
        exit(1);
    }

    default:
        break;
    }

  return TRUE;
}

GstElement *gstreamer_receive_create_pipeline(char *pipeline) {
  gst_init(NULL, NULL);
  GError *error = NULL;
  return gst_parse_launch(pipeline, &error);
}

void gstreamer_receive_start_pipeline(GstElement *pipeline) {
  GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
  gst_bus_add_watch(bus, gstreamer_receive_bus_call, NULL);
  gst_object_unref(bus);

  gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void gstreamer_receive_stop_pipeline(GstElement* pipeline) {
    gst_element_send_event(pipeline, gst_event_new_eos());
}

void gstreamer_receive_destroy_pipeline(GstElement* pipeline) {
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
}

void gstreamer_receive_push_buffer(GstElement *pipeline, void *buffer, int len) {
  GstElement *src = gst_bin_get_by_name(GST_BIN(pipeline), "src");
  if (src != NULL) {
    gpointer p = g_memdup(buffer, len);
    GstBuffer *buffer = gst_buffer_new_wrapped(p, len);
    gst_app_src_push_buffer(GST_APP_SRC(src), buffer);
    gst_object_unref(src);
  }
}

void gstreamer_on_fps_signal(GstElement *element, gdouble current_fps,  gdouble loss_rate, gdouble average_fps, gpointer user_data) {
    SampleHandlerUserData *s = (SampleHandlerUserData*) user_data;
    goOnFpsSignal(current_fps, loss_rate, average_fps, s->pipelineId);
}

// Connect singal of fpsdisplaysink
void gstreamer_connect_fps_signal(GstElement* pipeline, char *element_name, int pipelineId) {
    GstElement *element = gst_bin_get_by_name(GST_BIN(pipeline), element_name);
    SampleHandlerUserData* s = malloc(sizeof(SampleHandlerUserData));
    s->pipelineId = pipelineId;
    g_signal_connect(element, "fps-measurements", G_CALLBACK(gstreamer_on_fps_signal), s);
    gst_object_unref(element);
}

gint gstreamer_get_rtpjitterbuffer_percent(GstElement *pipeline) {
  GstElement *buffer = gst_bin_get_by_name(GST_BIN(pipeline), "rtpjitterbuffer");
  if (buffer != NULL) {
    gint percent;
    g_object_get(buffer, "percent", &percent, NULL);
    gst_object_unref(buffer);
    return percent;
  }
  return -1;
}
