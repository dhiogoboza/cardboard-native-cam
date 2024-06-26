package com.nvision.facetracker;


import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.ImageFormat;
import android.graphics.Point;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.os.Handler;
import android.os.HandlerThread;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.util.Log;
import android.util.Size;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import androidx.annotation.NonNull;
import androidx.core.content.ContextCompat;


public class CameraRenderView extends SurfaceView implements SurfaceHolder.Callback {

    private static final String TAG = "CameraRenderView";
    private WeakReference<Activity> mWeakActivity;
    private CameraDevice mCamera;
    private String mCameraId;

    private CaptureRequest.Builder mPreviewBuilder;
    private CameraCaptureSession mCaptureSession;

    private ImageReader mImageReader;

    boolean mIsSurfaceAvailable;
    private Surface mSurface;
    private SurfaceTexture mSurfaceTexture;
    private final Semaphore mCameraOpenCloseLock = new Semaphore(1);
    private HandlerThread mCamSessionThread;
    private Handler mCamSessionHandler;

    private static final int MAX_PREVIEW_WIDTH = 1280;
    private static final int MAX_PREVIEW_HEIGHT = 720;

    private Size mPreviewSize;
    private int mWidth, mHeight;

    //public static int IMAGE_WIDTH = 640, IMAGE_HEIGHT = 480;
    public static int IMAGE_WIDTH = 1280, IMAGE_HEIGHT = 720;

    //CameraDevice StateCallback
    private final CameraDevice.StateCallback mCameraDeviceCallback = new CameraDevice.StateCallback() {
        @Override
        public void onOpened(@NonNull CameraDevice cameraDevice) {
            mCameraOpenCloseLock.release();
            mCamera = cameraDevice;

            try {
                createCameraPreviewSession();
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void onDisconnected(@NonNull CameraDevice cameraDevice) {
            mCameraOpenCloseLock.release();
            cameraDevice.close();
            mCamera = null;
        }

        @Override
        public void onError(@NonNull CameraDevice cameraDevice, int i) {

        }
    };

    // Session State Callback
    private final CameraCaptureSession.StateCallback mSessionStateCallback = new CameraCaptureSession.StateCallback() {
        @Override
        public void onConfigured(@NonNull CameraCaptureSession cameraCaptureSession) {
            if (null == mCamera) return;

            mCaptureSession = cameraCaptureSession;
            try {
                mPreviewBuilder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_VIDEO);
                //mPreviewBuilder.set(CaptureRequest.CONTROL_AE_MODE, CaptureRequest.CONTROL_AE_MODE_OFF);
                mPreviewBuilder.set(CaptureRequest.LENS_OPTICAL_STABILIZATION_MODE, CaptureRequest.LENS_OPTICAL_STABILIZATION_MODE_ON);
                mPreviewBuilder.set(CaptureRequest.SENSOR_SENSITIVITY, 1600);
                //mPreviewBuilder.set(CaptureRequest.CONTROL_AE_EXPOSURE_COMPENSATION, 0);

                startPreview(mCaptureSession);
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }

        }

        @Override
        public void onConfigureFailed(@NonNull CameraCaptureSession cameraCaptureSession) {
            Log.e("CameraRenderView", "onConfigureFailed");
        }
    };


    private final CameraCaptureSession.CaptureCallback mSessionCaptureCallback = new CameraCaptureSession.CaptureCallback() {
        @Override
        public void onCaptureCompleted(@NonNull CameraCaptureSession session, @NonNull CaptureRequest request, @NonNull TotalCaptureResult result) {
            //Log.i("CameraRenderView","CameraCaptureSession Capture Completed");
        }

        @Override
        public void onCaptureProgressed(@NonNull CameraCaptureSession session, @NonNull CaptureRequest request, @NonNull CaptureResult partialResult) {

        }
    };

    //This is a callback object for ImageReader OnImageAvailble will be called when a still image is ready for process
    private final ImageReader.OnImageAvailableListener mOnImageAvailableListener = new ImageReader.OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader imageReader) {
            //Log.i("CameraRenderView", "CameraRenderView OnImageAvailable Since last time " + (cur_time - last_time));
            Image image = imageReader.acquireNextImage();
            image.close();

        }
    };

    static {
        System.loadLibrary("nvision_core");
    }

    public CameraRenderView(Context context) {
        this(context, null);
    }

    public CameraRenderView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }


    public void init(Activity activity, String shader) {
        mWeakActivity = new WeakReference<>(activity);
        SurfaceHolder surfaceHolder = this.getHolder();
        surfaceHolder.addCallback(this);
        surfaceHolder.setKeepScreenOn(true);

        mIsSurfaceAvailable = false;

        //Create a App
        nativeCreateApp(activity, shader);
    }

    public void onResume() {
        nativeResumeApp();
    }

    public void onPause() {
        nativePauseApp();
    }

    public void deinit() {
        nativeDestroyApp();
    }


    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {
        mIsSurfaceAvailable = false;
        nativeCreateSurface(surfaceHolder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int format, int width, int height) {
        mWidth = width;
        mHeight = height;

        //This method may block the ui thread until the gl context and surface texture id created
        nativeSetSurface(surfaceHolder.getSurface());
        //configure the output sizes for the surfaceTexture and select a id for camera
        configureCamera(width, height);
        //Only First time we open the camera and create imageReader
        if (!mIsSurfaceAvailable) {
            createImageReader();

            if (mCameraId != null) {
                startCameraSessionThread();
                openCamera();
            }

        }
        mIsSurfaceAvailable = true;

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
        nativeSetSurface(null);

        closeCamera();
        destroyImageReader();
        stopCameraSessionThread();
        destroyPreviewSurface();

        mIsSurfaceAvailable = false;
    }

    private void startCameraSessionThread() {
        mCamSessionThread = new HandlerThread("Camera2");
        mCamSessionThread.start();
        mCamSessionHandler = new Handler(mCamSessionThread.getLooper());
    }

    private void stopCameraSessionThread() {
        mCamSessionThread.quitSafely();
        try {
            mCamSessionThread.join();
            mCamSessionThread = null;
            mCamSessionHandler = null;
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private void createCameraPreviewSession() throws CameraAccessException {
        mSurface = getPreviewSurface(mPreviewSize);
        mPreviewBuilder = mCamera.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW); //This must called before createCaptureSession
        mCamera.createCaptureSession(Arrays.asList(mSurface, mImageReader.getSurface()), mSessionStateCallback, null);
    }

    private void startPreview(CameraCaptureSession session) throws CameraAccessException {
        //Set Surface of SurfaceView as the target of the builder
        mPreviewBuilder.addTarget(mSurface);
        mPreviewBuilder.addTarget(mImageReader.getSurface());
        session.setRepeatingRequest(mPreviewBuilder.build(), mSessionCaptureCallback, mCamSessionHandler);
    }

    @SuppressWarnings("unused")
    private void configureCamera(int width, int height) {
        // FIXME: Use width and height

        // Assume it is a face back camera
        mCameraId = "" + CameraCharacteristics.LENS_FACING_BACK;

        // Get back camera
        try {
            if (getActivity() != null) {
                CameraManager cameraManager = (CameraManager) getActivity().getSystemService(Context.CAMERA_SERVICE);
                for (String id : cameraManager.getCameraIdList()) {
                    CameraCharacteristics characteristics = cameraManager.getCameraCharacteristics(id);
                    if (characteristics.get(CameraCharacteristics.LENS_FACING)
                            == CameraCharacteristics.LENS_FACING_BACK) {
                        mCameraId = id;
                        break;
                    }
                }
            }
        } catch (CameraAccessException e) {
            Log.e("CameraRenderView", "Error acquiring camera information", e);
        }

        ///Configure camera output surfaces
        setupCameraOutputs(mWidth, mHeight);
    }

    private void openCamera() {
        if (getActivity() == null)
            return;

        if (ContextCompat.checkSelfPermission(getActivity(), Manifest.permission.CAMERA)
                != PackageManager.PERMISSION_GRANTED) {
            PermissionHelper.requestCameraPermission(getActivity(), true);
            return;
        }

        ///Prepare for camera
        CameraManager cameraManager = (CameraManager) getActivity().getSystemService(Context.CAMERA_SERVICE);
        try {
            if (!mCameraOpenCloseLock.tryAcquire(2500, TimeUnit.MILLISECONDS)) {
                throw new RuntimeException("Time out waiting to lock camera opening.");
            }
        } catch (InterruptedException e) {
            throw new RuntimeException("Interupted while trying to lock camera opening.", e);
        }

        try {
            cameraManager.openCamera(mCameraId, mCameraDeviceCallback, mCamSessionHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }


    private void closeCamera() {
        try {
            mCameraOpenCloseLock.acquire();

            if (null != mCaptureSession) {
                mCaptureSession.close();
                mCaptureSession = null;
            }

            if (null != mCamera) {
                mCamera.close();
                mCamera = null;
                mCameraId = null;
            }

        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while trying to lock camera closing", e);
        } finally {
            mCameraOpenCloseLock.release();
        }
    }

    private void createImageReader() {
        mImageReader = ImageReader.newInstance(IMAGE_WIDTH, IMAGE_HEIGHT, ImageFormat.YUV_420_888, 2);
        mImageReader.setOnImageAvailableListener(mOnImageAvailableListener, mCamSessionHandler);
    }

    private void destroyImageReader() {

        if (null != mImageReader) {
            mImageReader.close();
            mImageReader = null;
        }
    }


    private void setupCameraOutputs(int width, int height) {
        Activity activity = getActivity();
        if (activity == null)
            return;

        CameraManager manager = (CameraManager) activity.getSystemService(Context.CAMERA_SERVICE);
        try {

            CameraCharacteristics characteristics = manager.getCameraCharacteristics(mCameraId);

            StreamConfigurationMap map = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);

            Size largest = Collections.max(Arrays.asList(map.getOutputSizes(ImageFormat.YUV_420_888)), new CompareSizesByArea());

            //Log.i("CameraRenderView", "CameraRenderView displaySize");
            Point displaySize = new Point();
            activity.getWindowManager().getDefaultDisplay().getSize(displaySize);
            int rotatedPreviewWidth = width;
            int rotatedPreviewHeight = height;
            int maxPreviewWidth = displaySize.x;
            int maxPreviewHeight = displaySize.y;

            if (rotatedPreviewHeight > rotatedPreviewWidth) {
                int aux = rotatedPreviewHeight;
                //noinspection SuspiciousNameCombination
                rotatedPreviewHeight = rotatedPreviewWidth;
                rotatedPreviewWidth = aux;
            }

            if (maxPreviewWidth > MAX_PREVIEW_WIDTH) {
                maxPreviewWidth = MAX_PREVIEW_WIDTH;
            }

            if (maxPreviewHeight > MAX_PREVIEW_HEIGHT) {
                maxPreviewHeight = MAX_PREVIEW_HEIGHT;
            }

            Log.i("CameraRenderView", "CameraRenderView PreviewSize");
            // Danger, W.R.! Attempting to use too large a preview size could  exceed the camera
            // bus' bandwidth limitation, resulting in gorgeous previews but the storage of
            // garbage capture data.
            mPreviewSize = chooseOptimalSize(map.getOutputSizes(SurfaceTexture.class),
                    rotatedPreviewWidth, rotatedPreviewHeight, maxPreviewWidth,
                    maxPreviewHeight, largest);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        } catch (NullPointerException e) {
            Log.e("CameraRenderView", "This device doesn't support Camera2 API");
        }
    }


    private Surface getPreviewSurface(Size size) {
        if (mSurface == null) {
            Activity activity = getActivity();
            boolean flip = activity != null
                    && activity.getWindowManager().getDefaultDisplay().getRotation()
                    == Surface.ROTATION_270;

            //Get the SurfaceTexture from SurfaceView GL Context
            mSurfaceTexture = nativeSurfaceTexture(flip);
            mSurfaceTexture.setOnFrameAvailableListener(new SurfaceTexture.OnFrameAvailableListener() {
                @Override
                public void onFrameAvailable(SurfaceTexture surfaceTexture) {
                    nativeRequestUpdateTexture();
                }
            });
            //This is the output surface we need to start preview
            mSurface = new Surface(mSurfaceTexture);
        }

        mSurfaceTexture.setDefaultBufferSize(size.getWidth(), size.getHeight());
        return mSurface;
    }

    private void destroyPreviewSurface() {
        if (mSurface != null) {

            mSurfaceTexture.release();
            nativeDestroyTexture();
            mSurfaceTexture = null;
            mSurface.release();
            mSurface = null;
        }
    }


    private Activity getActivity() {
        return mWeakActivity != null ? mWeakActivity.get() : null;
    }


    private static Size chooseOptimalSize(Size[] choices, int textureViewWidth,
                                          int textureViewHeight, int maxWidth, int maxHeight, Size aspectRatio) {

        // Collect the supported resolutions that are at least as big as the preview Surface
        List<Size> bigEnough = new ArrayList<>();
        // Collect the supported resolutions that are smaller than the preview Surface
        List<Size> notBigEnough = new ArrayList<>();
        int w = aspectRatio.getWidth();
        int h = aspectRatio.getHeight();
        for (Size option : choices) {
            if (option.getWidth() <= maxWidth && option.getHeight() <= maxHeight &&
                    option.getHeight() == option.getWidth() * h / w) {
                if (option.getWidth() >= textureViewWidth &&
                        option.getHeight() >= textureViewHeight) {
                    bigEnough.add(option);
                } else {
                    notBigEnough.add(option);
                }
            }
        }

        // Pick the smallest of those big enough. If there is no one big enough, pick the
        // largest of those not big enough.
        if (bigEnough.size() > 0) {
            return Collections.min(bigEnough, new CompareSizesByArea());
        } else if (notBigEnough.size() > 0) {
            return Collections.max(notBigEnough, new CompareSizesByArea());
        } else {
            Log.e("CameraRenderView", "Couldn't find any suitable preview size");
            return choices[0];
        }
    }

    public boolean setEffect(String shaderStr) {
        if (mPreviewBuilder != null) {
            if (TextUtils.isEmpty(shaderStr)) {
                Log.e(TAG, "Empty shader");
                return false;
            }

            Log.d(TAG, "shaderStr: " + shaderStr);

            nativePauseApp();
            nativeSetShader(shaderStr);
            nativeResumeApp();

            try {
                mCaptureSession.setRepeatingRequest(mPreviewBuilder.build(), mSessionCaptureCallback, mCamSessionHandler);
            } catch (CameraAccessException e) {
                Log.e(TAG, "Error setting camera effect", e);
                return false;
            }

            return true;
        } else {
            Log.w(TAG, "Preview builder is null");
        }

        return false;
    }

    static class CompareSizesByArea implements Comparator<Size> {
        @Override
        public int compare(Size lhs, Size rhs) {
            return Long.signum((long) lhs.getWidth() * lhs.getHeight() - (long) rhs.getWidth() * rhs.getHeight());
        }
    }


    public static native void nativeCreateApp(Activity activity, String shader);

    public static native void nativeResumeApp();

    public static native void nativeSetSurface(Surface surface);

    public static native void nativeSetShader(String shaderStr);

    public static native void nativePauseApp();

    public static native void nativeDestroyApp();

    public static native SurfaceTexture nativeSurfaceTexture(boolean flip);

    public static native void nativeRequestUpdateTexture();

    public static native void nativeDestroyTexture();

    public static native void nativeCreateSurface(Surface surface);

    public static native void nativeSwitchViewer();
}
