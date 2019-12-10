package xyz.rinc.roid.yuv;

import android.graphics.Bitmap;
import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.media.Image;

import java.io.ByteArrayOutputStream;
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

    public static boolean imageToYuvBytes(Image image, YuvFormat yuvFormat, byte[] yuv_bytes) {
        try {
            Rect crop = image.getCropRect();
            int width = crop.width();
            int height = crop.height();
            Image.Plane[] planes = image.getPlanes();
            byte[] row_bytes = new byte[planes[0].getRowStride()];
            int channelOffset = 0;
            int outputStride = 1;
            ByteBuffer buffer;
            for (int i = 0; i < planes.length; i++) {
                switch (i) {
                    case 0: {
                        channelOffset = 0;
                        outputStride = 1;
                        break;
                    }
                    case 1: {
                        if (yuvFormat == YuvFormat.I420) {
                            channelOffset = width * height;
                            outputStride = 1;
                        } else {
                            channelOffset = width * height + 1;
                            outputStride = 2;
                        }
                        break;
                    }
                    case 2: {
                        if (yuvFormat == YuvFormat.I420) {
                            channelOffset = width * height * 5 / 4;
                            outputStride = 1;
                        } else {
                            channelOffset = width * height;
                            outputStride = 2;
                        }
                        break;
                    }
                }
                buffer = planes[i].getBuffer();
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
                        buffer.get(yuv_bytes, channelOffset, length);
                        channelOffset += length;
                    } else {
                        length = (w - 1) * pixelStride + 1;
                        buffer.get(row_bytes, 0, length);
                        for (int col = 0; col < w; col++) {
                            yuv_bytes[channelOffset] = row_bytes[col * pixelStride];
                            channelOffset += outputStride;
                        }
                    }
                    if (row < h - 1) {
                        buffer.position(buffer.position() + rowStride - length);
                    }
                }
                buffer.clear();
            }
            return true;
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }
    
    public static byte[] yuv2Jpeg(byte[] data, int format, final int width, final int height, Rect cropRect) {
        byte[] bytes = null;
        if (cropRect == null) cropRect = new Rect(0, 0, width, height);
        ByteArrayOutputStream baos = new ByteArrayOutputStream(data.length);
        try {
            if (new YuvImage(data, format, width, height, null).compressToJpeg(cropRect, 100, baos)) {
                bytes = baos.toByteArray();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        if (baos != null) {
            try {
                baos.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return bytes;
    }

    public static int bitmapToI420(Bitmap bitmap, byte[] dst_bytes) {
        if (bitmap == null || bitmap.isRecycled() || bitmap.getConfig() != Bitmap.Config.ARGB_8888) return Integer.MIN_VALUE;
        if (dst_bytes == null) dst_bytes = new byte[bitmap.getWidth() * bitmap.getHeight() * 3 / 2];
        int ret = nativeBitmapToI420(bitmap, dst_bytes);
        bitmap.recycle();
        return ret;
    }

    public static int I420ToNV21(byte[] i420_bytes, int width, int height, byte[] dst_bytes) {
        if (i420_bytes == null || width <= 0 || height <= 0) return Integer.MIN_VALUE;
        if (dst_bytes == null) dst_bytes = new byte[width * height * 3 / 2];
        return nativeI420ToNV21(i420_bytes, dst_bytes, width, height);
    }

    public static int NV21ToI420(byte[] nv21_bytes, int width, int height, byte[] dst_bytes) {
        if (nv21_bytes == null || width <= 0 || height <= 0) return Integer.MIN_VALUE;
        if (dst_bytes == null) dst_bytes = new byte[width * height * 3 / 2];
        return nativeNV21ToI420(nv21_bytes, dst_bytes, width, height);
    }

    public static int rotateI420(byte[] src_bytes, int width, int height, RotateMode mode, byte[] dst_bytes) {
        if (src_bytes == null || width <= 0 || height <= 0) return Integer.MIN_VALUE;
        if (mode == RotateMode.Clockwise0) return Integer.MIN_VALUE;
        if (dst_bytes == null) dst_bytes = new byte[width * height * 3 / 2];
        int degree = 0;
        switch (mode) {
            case Clockwise270: degree += 90;
            case Clockwise180: degree += 90;
            case Clockwise90: degree += 90;
        }
        return nativeRotateI420(src_bytes, width, height, dst_bytes, degree);
    }

    public static int mirrorI420(byte[] src_bytes, int width, int height, byte[] dst_bytes) {
        if (src_bytes == null || width <= 0 || height <= 0) return Integer.MIN_VALUE;
        if (dst_bytes == null) dst_bytes = new byte[width * height * 3 / 2];
        return nativeMirrorI420(src_bytes, width, height, dst_bytes);
    }

    public static int scaleI420(byte[] src_bytes, int src_width, int src_height, int dst_width, int dst_height, FilterMode filter_mode, byte[] dst_bytes) {
        if (src_bytes == null || src_width <= 0 || src_height <= 0 || dst_width <= 0 || dst_height <= 0) return Integer.MIN_VALUE;
        if (dst_bytes == null) dst_bytes = new byte[Math.max(src_width * src_height, dst_width * dst_height) * 3 / 2];
        int mode = 0;
        switch (filter_mode) {
            case Box: ++mode;
            case Bilinear: ++mode;
            case Linear: ++mode;
            case None:
        }
        return nativeScaleI420(src_bytes, src_width, src_height, dst_bytes, dst_width, dst_height, mode);
    }

    private static native int nativeBitmapToI420(Bitmap bitmap, byte[] i420_bytes);
    private static native int nativeI420ToNV21(byte[] i420_bytes, byte[] nv21_bytes, int width, int height);
    private static native int nativeNV21ToI420(byte[] nv21_bytes, byte[] i420_bytes, int width, int height);
    private static native int nativeRotateI420(byte[] src_bytes, int width, int height, byte[] dst_bytes, int degree);
    private static native int nativeMirrorI420(byte[] src_bytes, int width, int height, byte[] dst_bytes);
    private static native int nativeScaleI420(byte[] src_bytes, int src_width, int src_height, byte[] dst_bytes, int dst_width, int dst_height, int filter_mode);
}
