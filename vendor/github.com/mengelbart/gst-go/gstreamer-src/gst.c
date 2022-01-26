#include "gst.h"

GMainLoop *gstreamer_send_main_loop = NULL;
void gstreamer_send_start_mainloop(void) {
  gstreamer_send_main_loop = g_main_loop_new(NULL, FALSE);

  g_main_loop_run(gstreamer_send_main_loop);
}

static gboolean go_gst_bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
    switch (GST_MESSAGE_TYPE(msg)) {

    case GST_MESSAGE_EOS: {
        goHandleSendEOS();
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

GstFlowReturn go_gst_send_new_sample_handler(GstElement *object, gpointer user_data) {
    GstSample *sample = NULL;
    GstBuffer *buffer = NULL;
    gpointer copy = NULL;
    gsize copy_size = 0;
    SampleHandlerUserData *s = (SampleHandlerUserData*) user_data;

    g_signal_emit_by_name (object, "pull-sample", &sample);

    if (sample) {
        buffer = gst_sample_get_buffer(sample);
        if (buffer) {
            gst_buffer_extract_dup(buffer, 0, gst_buffer_get_size(buffer), &copy, &copy_size);
            goHandlePipelineBuffer(copy, copy_size, s->pipelineId);
        }
        gst_sample_unref(sample);
    }

    return GST_FLOW_OK;
}

GstElement* gstreamer_send_create_pipeline(char *pipelineStr) {
    GError *error = NULL;
    GstElement *pipeline;

    gst_init(NULL, NULL);

    return gst_parse_launch(pipelineStr, &error);
}

void gstreamer_send_start_pipeline(GstElement* pipeline, int pipelineId) {
    SampleHandlerUserData* s = malloc(sizeof(SampleHandlerUserData));
    s->pipelineId = pipelineId;

    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus, go_gst_bus_call, NULL);
    gst_object_unref(bus);

    GstElement *appsink = gst_bin_get_by_name(GST_BIN(pipeline), "appsink");
    g_object_set(appsink, "emit-signals", TRUE, NULL);
    g_signal_connect(appsink, "new-sample", G_CALLBACK(go_gst_send_new_sample_handler), s);
    gst_object_unref(appsink);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void gstreamer_send_stop_pipeline(GstElement* pipeline) {
    gst_element_send_event(pipeline, gst_event_new_eos());
}

void gstreamer_send_destroy_pipeline(GstElement* pipeline) {
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
}

unsigned int gstreamer_get_property_uint(GstElement* pipeline, char *name, char *prop) {
    GstElement* element;
    element = gst_bin_get_by_name(GST_BIN(pipeline), name);
    unsigned int value = 0;

    if (element) {
        g_object_get(element, prop, &value, NULL);
    }
    return value;
}

void gstreamer_send_set_property_uint(GstElement* pipeline, char *name, char *prop, unsigned int value) {
    GstElement* element;
    element = gst_bin_get_by_name(GST_BIN(pipeline), name);

    if (element) {
        g_object_set(element, prop, value, NULL);
        gst_object_unref(element);
    }
}
