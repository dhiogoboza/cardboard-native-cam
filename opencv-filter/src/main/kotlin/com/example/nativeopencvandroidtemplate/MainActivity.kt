//package com.example.nativeopencvandroidtemplate
//
//import android.Manifest
//import android.app.Activity
//import android.content.pm.PackageManager
//import android.os.Bundle
//import androidx.core.app.ActivityCompat
//import android.util.Log
//import android.view.SurfaceView
//import android.view.WindowManager
//import android.widget.Toast
//import android.widget.LinearLayout
//import org.opencv.android.BaseLoaderCallback
//import org.opencv.android.CameraBridgeViewBase
//import org.opencv.android.LoaderCallbackInterface
//import org.opencv.android.OpenCVLoader
//import org.opencv.core.Mat
//import androidx.core.view.WindowCompat
//import androidx.core.view.WindowInsetsControllerCompat
//import androidx.core.view.WindowInsetsCompat
//
//class MainActivity : Activity(), CameraBridgeViewBase.CvCameraViewListener2 {
//
//    private var mOpenCvCameraView: CameraBridgeViewBase? = null
//
//    private val mLoaderCallback = object : BaseLoaderCallback(this) {
//        override fun onManagerConnected(status: Int) {
//            when (status) {
//                LoaderCallbackInterface.SUCCESS -> {
//                    Log.i(TAG, "OpenCV loaded successfully")
//
//                    // Load native library after(!) OpenCV initialization
//                    System.loadLibrary("native-lib")
//
//                    mOpenCvCameraView!!.enableView()
//                }
//                else -> {
//                    super.onManagerConnected(status)
//                }
//            }
//        }
//    }
//
//    override fun onCreate(savedInstanceState: Bundle?) {
//        Log.i(TAG, "called onCreate")
//        super.onCreate(savedInstanceState)
//        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
//
//        nativeCreateApp()
//
//        // Permissions for Android 6+
//        ActivityCompat.requestPermissions(
//            this@MainActivity,
//            arrayOf(Manifest.permission.CAMERA),
//            CAMERA_PERMISSION_REQUEST
//        )
//
//        setContentView(R.layout.activity_main)
//
//        mOpenCvCameraView = findViewById<CameraBridgeViewBase>(R.id.main_surface)
//
//        mOpenCvCameraView!!.visibility = SurfaceView.VISIBLE
//
//        mOpenCvCameraView!!.setCvCameraViewListener(this)
//
//        hideSystemUI()
//    }
//
//    private fun hideSystemUI() {
//        WindowCompat.setDecorFitsSystemWindows(window, false)
//        WindowInsetsControllerCompat(window, findViewById<LinearLayout>(R.id.main_container)).let { controller ->
//            controller.hide(WindowInsetsCompat.Type.systemBars())
//            controller.systemBarsBehavior = WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
//        }
//    }
//
//    override fun onRequestPermissionsResult(
//        requestCode: Int,
//        permissions: Array<String>,
//        grantResults: IntArray
//    ) {
//        when (requestCode) {
//            CAMERA_PERMISSION_REQUEST -> {
//                if (grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
//                    mOpenCvCameraView!!.setCameraPermissionGranted()
//                } else {
//                    val message = "Camera permission was not granted"
//                    Log.e(TAG, message)
//                    Toast.makeText(this, message, Toast.LENGTH_LONG).show()
//                }
//            }
//            else -> {
//                Log.e(TAG, "Unexpected permission request")
//            }
//        }
//    }
//
//    override fun onPause() {
//        super.onPause()
//        if (mOpenCvCameraView != null)
//            mOpenCvCameraView!!.disableView()
//    }
//
//    override fun onResume() {
//        super.onResume()
//        if (!OpenCVLoader.initDebug()) {
//            Log.d(TAG, "Internal OpenCV library not found. Using OpenCV Manager for initialization")
//            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION, this, mLoaderCallback)
//        } else {
//            Log.d(TAG, "OpenCV library found inside package. Using it!")
//            mLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS)
//        }
//    }
//
//    override fun onDestroy() {
//        super.onDestroy()
//        if (mOpenCvCameraView != null)
//            mOpenCvCameraView!!.disableView()
//    }
//
//    override fun onCameraViewStarted(width: Int, height: Int) {}
//
//    override fun onCameraViewStopped() {}
//
//    override fun onCameraFrame(frame: CameraBridgeViewBase.CvCameraViewFrame): Mat {
//        // get current camera frame as OpenCV Mat object
//        val mat = frame.rgba() //gray()
//
//        // native call to process current camera frame
//        adaptiveThresholdFromJNI(mat.nativeObjAddr)
//
//        // return processed frame for live preview
//        return mat
//    }
//
//    //private external fun adaptiveThresholdFromJNI(matAddr: Long)
//
//    private external fun nativeCreateApp();
//    private external fun nativeOnDestroy()
//    private external fun nativeOnSurfaceCreated(texture: Int, surface: android.view.Surface)
//    private external fun nativeOnDrawFrame(matAddr: Long)
//    private external fun nativeOnPause()
//    private external fun nativeOnResume()
//    //private external fun nativeSetScreenParams(nativeApp: Long, width: Int, height: Int)
//    //private external fun nativeSwitchViewer(nativeApp: Long)
//
//    companion object {
//        private const val TAG = "MainActivity"
//        private const val CAMERA_PERMISSION_REQUEST = 1
//    }
//}
