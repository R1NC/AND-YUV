#include <jni.h>
#include <libyuv.h>

class YUV {
private:
    JNIEnv *env;
    jbyteArray jba;
public:
    jbyte *data, *y, *vu, *u, *v;
    int y_stride, vu_stride, u_stride, v_stride;

    YUV(JNIEnv *env, jbyteArray jba, int w, int h) {
        this->env = env;
        this->jba = jba;
        int size = w * h;
        data = env->GetByteArrayElements(jba, NULL);
        y = data;
        y_stride = w;
        u = y + size;
        u_stride = w / 2;
        vu = u;
        vu_stride = w;
        v = u + size / 4;
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

    libyuv::I420ToNV21(
            (const uint8_t *) i420.y, i420.y_stride,
            (const uint8_t *) i420.u, i420.u_stride,
            (const uint8_t *) i420.v, i420.v_stride,
            (uint8_t *) nv21.y, nv21.y_stride,
            (uint8_t *) nv21.vu, nv21.vu_stride,
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

    libyuv::NV21ToI420((const uint8_t *) nv21.y, nv21.y_stride,
                       (const uint8_t *) nv21.vu, nv21.vu_stride,
                       (uint8_t *) i420.y, i420.y_stride,
                       (uint8_t *) i420.u, i420.u_stride,
                       (uint8_t *) i420.v, i420.v_stride,
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

    libyuv::I420Rotate((const uint8_t *) src.y, src.y_stride,
                       (const uint8_t *) src.u, src.u_stride,
                       (const uint8_t *) src.v, src.v_stride,
                       (uint8_t *) dst.y, dst.y_stride,
                       (uint8_t *) dst.u, dst.u_stride,
                       (uint8_t *) dst.v, dst.v_stride,
                       width, height,
                       (libyuv::RotationMode) degree);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_libyuv_util_YuvUtil_mirrorI420(JNIEnv *env, jclass,
                                                            jbyteArray src_bytes, jint width,
                                                            jint height, jbyteArray dst_bytes) {
    YUV src(env, src_bytes, width, height);
    YUV dst(env, dst_bytes, width, height);

    libyuv::I420Mirror((const uint8_t *) src.y, src.y_stride,
                       (const uint8_t *) src.u, src.u_stride,
                       (const uint8_t *) src.v, src.v_stride,
                       (uint8_t *) dst.y, dst.y_stride,
                       (uint8_t *) dst.u, dst.u_stride,
                       (uint8_t *) dst.v, dst.v_stride,
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

    libyuv::I420Scale((const uint8_t *) src.y, src.y_stride,
                      (const uint8_t *) src.u, src.u_stride,
                      (const uint8_t *) src.v, src.v_stride,
                      src_width, src_height,
                      (uint8_t *) dst.y, dst.y_stride,
                      (uint8_t *) dst.u, dst.u_stride,
                      (uint8_t *) dst.v, dst.v_stride,
                      dst_width, dst_height,
                      (libyuv::FilterMode) mode);
}
