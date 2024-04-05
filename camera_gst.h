#pragma once


#include <gst/gst.h>

#include <utility>

class CameraGST
{
public: 
    CameraGST() = default;
    ~CameraGST();
    bool Init(int *arg, char **argv);

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
};