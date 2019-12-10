#include <jni.h>
#include <libyuv.h>
#include <android/bitmap.h>
#include <android/log.h>

#define TAG "LibYUV-JNI"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__)
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__)

using namespace libyuv;

class YUV {
private:
    JNIEnv *env;
    jbyteArray jba;
public:
    jbyte *data;
    uint8_t *y, *vu, *u, *v;
    int y_stride, vu_stride, u_stride, v_stride;

    YUV(JNIEnv *env, jbyteArray jba, int w, int h, bool rotate = false) {
        this->env = env;
        this->jba = jba;
        int size = w * h;
        data = env->GetByteArrayElements(jba, NULL);
        y = (uint8_t *) data;
        u = y + size;
        vu = u;
        v = u + size / 4;
        y_stride = rotate ? h : w;
        u_stride = y_stride / 2;
        vu_stride = y_stride;
        v_stride = y_stride / 2;
    }

    ~YUV() {
        env->ReleaseByteArrayElements(jba, data, JNI_ABORT);
    }
};

extern "C"
JNIEXPORT jint JNICALL
Java_com_tencent_sppd_yuv_YuvUtil_nativeBitmapToI420(JNIEnv *env, jclass,
                                                     jobject bitmap, jbyteArray i420_bytes) {
    int ret;

    AndroidBitmapInfo bitmapInfo;
    if ((ret = AndroidBitmap_getInfo(env, bitmap, &bitmapInfo)) < 0) {
        LOGE("AndroidBitmap_getInfo() error:%d", ret);
        return -1;
    }
    int width = bitmapInfo.width, height = bitmapInfo.height;

    void *argb_bytes = nullptr;
    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &argb_bytes)) < 0) {
        LOGE("AndroidBitmap_lockPixels() error:%d", ret);
        return -2;
    }

    YUV i420(env, i420_bytes, width, height);

    if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        ret = ARGBToI420((const u_int8_t *) argb_bytes, bitmapInfo.stride,
                i420.y, i420.y_stride,
                i420.u, i420.u_stride,
                i420.v, i420.v_stride,
                width, height);
    } else if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_4444) {
        ret = ARGB4444ToI420((const u_int8_t *) argb_bytes, bitmapInfo.stride,
                i420.y, i420.y_stride,
                i420.u, i420.u_stride,
                i420.v, i420.v_stride,
                width, height);
    } else if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGB_565) {
        ret = RGB565ToI420((const u_int8_t *) argb_bytes, bitmapInfo.stride,
                i420.y, i420.y_stride,
                i420.u, i420.u_stride,
                i420.v, i420.v_stride,
                width, height);
    } else {
        LOGE("Unsupported format!");
        ret = -3;
    }

    AndroidBitmap_unlockPixels(env, bitmap);

    return ret;
}

extern "C"
JNIEXPORT jint JNICALL
Java_xyz_rinc_roid_yuv_YuvUtil_nativeI420ToNV21(JNIEnv *env, jclass,
                                                            jbyteArray i420_bytes,
                                                            jbyteArray nv21_bytes,
                                                            jint width, jint height) {
    YUV i420(env, i420_bytes, width, height);
    YUV nv21(env, nv21_bytes, width, height);

    return I420ToNV21(
            i420.y, i420.y_stride,
            i420.u, i420.u_stride,
            i420.v, i420.v_stride,
            nv21.y, nv21.y_stride,
            nv21.vu, nv21.vu_stride,
            width, height);
}

extern "C"
JNIEXPORT jint JNICALL
Java_xyz_rinc_roid_yuv_YuvUtil_nativeNV21ToI420(JNIEnv *env, jclass,
                                                            jbyteArray nv21_bytes,
                                                            jbyteArray i420_bytes,
                                                            jint width, jint height) {
    YUV nv21(env, nv21_bytes, width, height);
    YUV i420(env, i420_bytes, width, height);

    return NV21ToI420(nv21.y, nv21.y_stride,
               nv21.vu, nv21.vu_stride,
               i420.y, i420.y_stride,
               i420.u, i420.u_stride,
               i420.v, i420.v_stride,
               width, height);
}

extern "C"
JNIEXPORT jint JNICALL
Java_xyz_rinc_roid_yuv_YuvUtil_nativeRotateI420(JNIEnv *env, jclass,
                                                            jbyteArray src_bytes, jint width,
                                                            jint height, jbyteArray dst_bytes,
                                                            jint degree) {
    YUV src(env, src_bytes, width, height);
    YUV dst(env, dst_bytes, width, height, degree == 90 || degree == 270);

    return I420Rotate(src.y, src.y_stride,
               src.u, src.u_stride,
               src.v, src.v_stride,
               dst.y, dst.y_stride,
               dst.u, dst.u_stride,
               dst.v, dst.v_stride,
               width, height,
               (RotationMode) degree);
}

extern "C"
JNIEXPORT jint JNICALL
Java_xyz_rinc_roid_yuv_YuvUtil_nativeMirrorI420(JNIEnv *env, jclass,
                                                            jbyteArray src_bytes, jint width,
                                                            jint height, jbyteArray dst_bytes) {
    YUV src(env, src_bytes, width, height);
    YUV dst(env, dst_bytes, width, height);

    return I420Mirror(src.y, src.y_stride,
               src.u, src.u_stride,
               src.v, src.v_stride,
               dst.y, dst.y_stride,
               dst.u, dst.u_stride,
               dst.v, dst.v_stride,
               width, height);
}

extern "C"
JNIEXPORT jint JNICALL
Java_xyz_rinc_roid_yuv_YuvUtil_nativeScaleI420(JNIEnv *env, jclass,
                                                           jbyteArray src_bytes, jint src_width,
                                                           jint src_height, jbyteArray dst_bytes,
                                                           jint dst_width, jint dst_height,
                                                           jint mode) {
    YUV src(env, src_bytes, src_width, src_height);
    YUV dst(env, dst_bytes, dst_width, dst_height);

    return I420Scale(src.y, src.y_stride,
              src.u, src.u_stride,
              src.v, src.v_stride,
              src_width, src_height,
              dst.y, dst.y_stride,
              dst.u, dst.u_stride,
              dst.v, dst.v_stride,
              dst_width, dst_height,
              (FilterMode) mode);
}
