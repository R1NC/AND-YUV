#include <jni.h>
#include <libyuv.h>

using namespace libyuv;

class YUV {
private:
    JNIEnv *env;
    jbyteArray jba;
public:
    jbyte *data;
    uint8_t *y, *vu, *u, *v;
    int y_stride, vu_stride, u_stride, v_stride;

    YUV(JNIEnv *env, jbyteArray jba, int w, int h) {
        this->env = env;
        this->jba = jba;
        int size = w * h;
        data = env->GetByteArrayElements(jba, NULL);
        y = (uint8_t *) data;
        u = y + size;
        vu = u;
        v = u + size / 4;
        y_stride = w;
        u_stride = w / 2;
        vu_stride = w;
        v_stride = w / 2;
    }

    ~YUV() {
        env->ReleaseByteArrayElements(jba, data, JNI_ABORT);
    }
};

extern "C"
JNIEXPORT void JNICALL
Java_com_libyuv_util_YuvUtil_I420ToNV21(JNIEnv *env, jclass,
                                                            jbyteArray i420_bytes,
                                                            jbyteArray nv21_bytes,
                                                            jint width, jint height) {
    YUV i420(env, i420_bytes, width, height);
    YUV nv21(env, nv21_bytes, width, height);

    I420ToNV21(
            i420.y, i420.y_stride,
            i420.u, i420.u_stride,
            i420.v, i420.v_stride,
            nv21.y, nv21.y_stride,
            nv21.vu, nv21.vu_stride,
            width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_libyuv_util_YuvUtil_NV21ToI420(JNIEnv *env, jclass,
                                                            jbyteArray nv21_bytes,
                                                            jbyteArray i420_bytes,
                                                            jint width, jint height) {
    YUV nv21(env, nv21_bytes, width, height);
    YUV i420(env, i420_bytes, width, height);

    NV21ToI420(nv21.y, nv21.y_stride,
               nv21.vu, nv21.vu_stride,
               i420.y, i420.y_stride,
               i420.u, i420.u_stride,
               i420.v, i420.v_stride,
               width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_libyuv_util_YuvUtil_rotateI420(JNIEnv *env, jclass,
                                                            jbyteArray src_bytes, jint width,
                                                            jint height, jbyteArray dst_bytes,
                                                            jint degree) {
    YUV src(env, src_bytes, width, height);
    YUV dst(env, dst_bytes, width, height);

    I420Rotate(src.y, src.y_stride,
               src.u, src.u_stride,
               src.v, src.v_stride,
               dst.y, dst.y_stride,
               dst.u, dst.u_stride,
               dst.v, dst.v_stride,
               width, height,
               (RotationMode) degree);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_libyuv_util_YuvUtil_mirrorI420(JNIEnv *env, jclass,
                                                            jbyteArray src_bytes, jint width,
                                                            jint height, jbyteArray dst_bytes) {
    YUV src(env, src_bytes, width, height);
    YUV dst(env, dst_bytes, width, height);

    I420Mirror(src.y, src.y_stride,
               src.u, src.u_stride,
               src.v, src.v_stride,
               dst.y, dst.y_stride,
               dst.u, dst.u_stride,
               dst.v, dst.v_stride,
               width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_libyuv_util_YuvUtil_scaleI420(JNIEnv *env, jclass,
                                                           jbyteArray src_bytes, jint src_width,
                                                           jint src_height, jbyteArray dst_bytes,
                                                           jint dst_width, jint dst_height,
                                                           jint mode) {
    YUV src(env, src_bytes, src_width, src_height);
    YUV dst(env, dst_bytes, dst_width, dst_height);

    I420Scale(src.y, src.y_stride,
              src.u, src.u_stride,
              src.v, src.v_stride,
              src_width, src_height,
              dst.y, dst.y_stride,
              dst.u, dst.u_stride,
              dst.v, dst.v_stride,
              dst_width, dst_height,
              (FilterMode) mode);
}