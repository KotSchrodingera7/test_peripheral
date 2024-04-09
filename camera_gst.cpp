
#include "camera_gst.h"

CameraGST::CameraGST(int *arg, char **argv)
{
    gst_init(arg, &argv);
    sink = gst_element_factory_make ("qmlglsink", NULL);
}

CameraGST::~CameraGST()
{
    gst_element_set_state (pipeline_, GST_STATE_NULL);
    gst_object_unref (pipeline_);

    gst_deinit();
}

bool CameraGST::Init()
{
    pipeline_ = gst_pipeline_new (NULL);
    GstElement *src = gst_element_factory_make("v4l2src", NULL);
    if( !src ) 
    {
        return false;
    }

    g_object_set (src, "device", "/dev/video0", NULL);

    GstElement *capsfilter = gst_element_factory_make("capsfilter", NULL);
    if( !capsfilter ) 
    {
        return false;
    }

    GstCaps *caps = gst_caps_new_simple("video/x-raw",
                    "format", G_TYPE_STRING, "UYVY",
                    "width", G_TYPE_INT, 1920,
                    "height", G_TYPE_INT, 1080,
                    "framerate", GST_TYPE_FRACTION, 30, 1, NULL);
    if( !caps ) 
    {
        return false;
    } 

    g_object_set (capsfilter, "caps", caps, NULL);

    GstElement *glupload = gst_element_factory_make ("glupload", NULL);
    if( !glupload )
    {
        return false;
    }
    GstElement *glcolorconvert = gst_element_factory_make ("glcolorconvert", NULL);
    if( !glcolorconvert )
    {
        return false;
    }

    GstElement *gldownload = gst_element_factory_make ("gldownload", NULL);
    if( !gldownload )
    {
        return false;

    }

    if( !sink ) {
        return false;   
    }
    gst_bin_add_many (GST_BIN (pipeline_), src, capsfilter, glupload, glcolorconvert, gldownload, sink, NULL);
    gst_element_link_many (src, capsfilter, glupload, glcolorconvert, gldownload, sink, NULL);

    state_init_ = true;
    return true;
}

GstElement *CameraGST::GetSinkElement()
{
    return sink;
}


void CameraGST::CameraPlay()
{
    gst_element_set_state (pipeline_, GST_STATE_PLAYING);
}

void CameraGST::CameraPause()
{
    gst_element_set_state (pipeline_, GST_STATE_PAUSED);
}