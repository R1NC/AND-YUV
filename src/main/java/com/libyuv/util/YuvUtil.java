package com.libyuv.util;

import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.media.Image;
import android.text.TextUtils;

import java.io.ByteArrayOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

public class YuvUtil {

    static {
        System.loadLibrary("YuvUtil");
    }

    public enum FilterMode {
        None,      // Point sample; Fastest.
        Linear,    // Filter horizontally only.
        Bilinear,  // Faster than box, but lower quality scaling down.
        Box        // Highest quality.
    }
    
    public enum YuvFormat {
        NV21,
        I420
    }
    
    public enum RotateMode {
        Clockwise90,
        Clockwise180,
        Clockwise270
    }

    public static byte[] imageToYuvBytes(Image image, YuvFormat yuvFormat) {
        Rect crop = image.getCropRect();
        int format = image.getFormat();
        int width = crop.width();
        int height = crop.height();
        Image.Plane[] planes = image.getPlanes();
        byte[] data = new byte[width * height * ImageFormat.getBitsPerPixel(format) / 8];
        byte[] rowData = new byte[planes[0].getRowStride()];
        int channelOffset = 0;
        int outputStride = 1;
        for (int i = 0; i < planes.length; i++) {
            switch (i) {
                case 0:
                    channelOffset = 0;
                    outputStride = 1;
                    break;
                case 1:
                    if (yuvFormat == YuvFormat.I420) {
                        channelOffset = width * height;
                        outputStride = 1;
                    } else if (yuvFormat == YuvFormat.NV21) {
                        channelOffset = width * height + 1;
                        outputStride = 2;
                    }
                    break;
                case 2:
                    if (yuvFormat == YuvFormat.I420) {
                        channelOffset = (int) (width * height * 1.25);
                        outputStride = 1;
                    } else if (yuvFormat == YuvFormat.NV21) {
                        channelOffset = width * height;
                        outputStride = 2;
                    }
                    break;
            }
            ByteBuffer buffer = planes[i].getBuffer();
            int rowStride = planes[i].getRowStride();
            int pixelStride = planes[i].getPixelStride();
            int shift = (i == 0) ? 0 : 1;
            int w = width >> shift;
            int h = height >> shift;
            buffer.position(rowStride * (crop.top >> shift) + pixelStride * (crop.left >> shift));
            for (int row = 0; row < h; row++) {
                int length;
                if (pixelStride == 1 && outputStride == 1) {
                    length = w;
                    buffer.get(data, channelOffset, length);
                    channelOffset += length;
                } else {
                    length = (w - 1) * pixelStride + 1;
                    buffer.get(rowData, 0, length);
                    for (int col = 0; col < w; col++) {
                        data[channelOffset] = rowData[col * pixelStride];
                        channelOffset += outputStride;
                    }
                }
                if (row < h - 1) {
                    buffer.position(buffer.position() + rowStride - length);
                }
            }
        }
        return data;
    }
    
    public static byte[] nv21ToJpeg(byte[] data, int w, int h, String dumpFile) {
        final YuvImage yuvImg = new YuvImage(data, ImageFormat.NV21, w, h, null);
        if (!TextUtils.isEmpty(dumpFile)) {
            try {
                FileOutputStream fos = new FileOutputStream(dumpFile);
                yuvImg.compressToJpeg(new Rect(0, 0, w, h), 100, fos);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        ByteArrayOutputStream baos = new ByteArrayOutputStream(data.length);
        if (yuvImg.compressToJpeg(new Rect(0, 0, w, h), 100, baos)) {
            return baos.toByteArray();
        }
        return null;
    }

    public static byte[] I420ToNV21(byte[] i420_bytes, int width, int height) {
        byte[] nv21_bytes = new byte[width * height * 3 / 2];
        I420ToNV21(i420_bytes, nv21_bytes, width, height);
        return nv21_bytes;
    }

    public static byte[] NV21ToI420(byte[] nv21_bytes, int width, int height) {
        byte[] dst_bytes = new byte[width * height * 3 / 2];
        NV21ToI420(nv21_bytes, dst_bytes, width, height);
        return dst_bytes;
    }

    public static byte[] rotateI420(byte[] src_bytes, int width, int height, RotateMode mode) {
        byte[] dst_bytes = new byte[width * height * 3 / 2];
        int degree = 0;
        switch (mode) {
            case Clockwise270:
                degree += 90;
            case Clockwise180:
                degree += 90;
            case Clockwise90:
                degree += 90;
        }
        rotateI420(src_bytes, width, height, dst_bytes, degree);
        return dst_bytes;
    }

    public static byte[] mirrorI420(byte[] src_bytes, int width, int height) {
        byte[] dst_bytes = new byte[width * height * 3 / 2];
        mirrorI420(src_bytes, width, height, dst_bytes);
        return dst_bytes;
    }

    public static byte[] scaleI420(byte[] src_bytes, int src_width, int src_height, int dst_width, int dst_height, FilterMode filter_mode) {
        int mode = 0;
        switch (filter_mode) {
            case Box:
                mode++;
            case Bilinear:
                mode++;
            case Linear:
                mode++;
            case None:
        }
        byte[] dst_bytes = new byte[dst_width * dst_height * 3 / 2];
        scaleI420(src_bytes, src_width, src_height, dst_bytes, dst_width, dst_height, mode);
        return dst_bytes;
    }

    private static native void I420ToNV21(byte[] i420_bytes, byte[] nv21_bytes, int width, int height);

    private static native void NV21ToI420(byte[] nv21_bytes, byte[] i420_bytes, int width, int height);

    private static native void rotateI420(byte[] src_bytes, int width, int height, byte[] dst_bytes, int degree);

    private static native void mirrorI420(byte[] src_bytes, int width, int height, byte[] dst_bytes);

    private static native void scaleI420(byte[] src_bytes, int src_width, int src_height, byte[] dst_bytes, int dst_width, int dst_height, int filter_mode);
}
