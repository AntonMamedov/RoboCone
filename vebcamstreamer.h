#pragma once
#include <opencv/cv.h>
#include <opencv/highgui.h>

class VebCamStreamer
{
public:
    VebCamStreamer(unsigned long flag);
    ~VebCamStreamer();
    void GetImage(CvMat* buf);
private:
    CvCapture* capture;
    IplImage* frame;

};
