#include "vebcamstreamer.h"

VebCamStreamer::VebCamStreamer(unsigned long flag){
    this->capture = cvCreateCameraCapture(flag);
    this->frame = nullptr;
}

VebCamStreamer::~VebCamStreamer(){
    cvReleaseCapture( &capture );
}

void VebCamStreamer::GetImage(CvMat* buf){
    frame = cvQueryFrame( capture );
    buf = cvEncodeImage(".jpg", frame);
}
