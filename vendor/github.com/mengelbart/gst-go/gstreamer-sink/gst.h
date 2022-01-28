#ifndef GST_H
#define GST_H

#include <glib.h>
#include <gst/gst.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct SampleHandlerUserData {
    int pipelineId;
} SampleHandlerUserData;

void gstreamer_receive_start_mainloop(void);

GstElement *gstreamer_receive_create_pipeline(char *pipeline);
void gstreamer_receive_start_pipeline(GstElement *pipeline);
void gstreamer_receive_stop_pipeline(GstElement* pipeline);
void gstreamer_receive_destroy_pipeline(GstElement* pipeline);
void gstreamer_receive_push_buffer(GstElement *pipeline, void *buffer, int len);

extern void goHandleReceiveEOS();
extern void goOnFpsSignal(gdouble current_fps, gdouble loss_rate, gdouble average_fps, int pipelineID);

void gstreamer_send_start_mainloop(void);

void gstreamer_connect_fps_signal(GstElement* pipeline, char *element_name, int pipelineId);

#endif
