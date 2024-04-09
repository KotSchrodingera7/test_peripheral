#pragma once


#include <gst/gst.h>

#include <utility>

class CameraGST
{
public: 
    CameraGST(int *arg, char **argv);
    ~CameraGST();
    bool Init();

template<typename ...Args>
    void ObjectSet(GstElement* object, const char* name, Args... args) 
    {
        g_object_set(object, name, std::forward<Args>(args)...);
    }
    GstElement *GetSinkElement();

    void CameraPlay();
    void CameraPause();

private:
    GstElement *pipeline_;
    GstElement *sink;
    bool state_init_{false};
};