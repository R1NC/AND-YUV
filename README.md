# LibYUV-Android
Build [LibYUV][1] for Android.


[1]: https://chromium.googlesource.com/libyuv/libyuv



* **Image to YUV**:

```java
public static boolean imageToYuvBytes(Image image, YuvFormat yuvFormat, byte[] yuv_bytes);
public static int bitmapToI420(Bitmap bitmap, byte[] dst_bytes);
```

* **YUV format transform**:

```java
public static int I420ToNV21(byte[] i420_bytes, int width, int height, byte[] dst_bytes);
public static int NV21ToI420(byte[] nv21_bytes, int width, int height, byte[] dst_bytes);
```

* **YUV matrix transform**:

```java
public static int rotateI420(byte[] src_bytes, int width, int height, RotateMode mode, byte[] dst_bytes);
public static int mirrorI420(byte[] src_bytes, int width, int height, byte[] dst_bytes);
public static int scaleI420(byte[] src_bytes, int src_width, int src_height, int dst_width, int dst_height, FilterMode filter_mode, byte[] dst_bytes);
```

* **YUV compress to Jpeg**:

```java
public static byte[] yuv2Jpeg(byte[] data, int format, final int width, final int height, Rect cropRect);
```
