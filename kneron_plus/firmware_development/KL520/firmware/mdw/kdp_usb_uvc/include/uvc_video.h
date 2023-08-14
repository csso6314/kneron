#ifndef UVC_VIDEO_H
#define UVC_VIDEO_H

#include <uvc_dev.h>


int uvc_video_init(struct uvc_device *dev);

int uvc_video_enable(struct uvc_streaming *stream);

int uvc_video_disable(struct uvc_streaming *stream);


#endif // UVC_VIDEO_H

