#ifndef GST_SRC_H
#define GST_SRC_H

#include <gst/gst.h>

typedef struct SampleHandlerUserData {
    int pipelineId;
} SampleHandlerUserData;

extern void goHandleSendEOS();
extern void goHandlePipelineBuffer(void *buffer, int bufferLen, int pipelineId);

void gstreamer_send_start_mainloop(void);

GstElement* gstreamer_send_create_pipeline(char *pipelineStr);
void gstreamer_send_start_pipeline(GstElement* pipeline, int pipelineId);
void gstreamer_send_stop_pipeline(GstElement* pipeline);
void gstreamer_send_destroy_pipeline(GstElement* pipeline);

unsigned int gstreamer_get_property_uint(GstElement* pipeline, char *name, char *prop);
void gstreamer_send_set_property_uint(GstElement* pipeline, char *name, char *prop, unsigned int value);

#endif
