package com.nvision.facetracker;

import android.os.Bundle;

import com.nvision.face_tracker_android.R;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {

    private CameraRenderView mCameraView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        PermissionHelper.requestCameraPermission(this, true);

        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        mCameraView = (CameraRenderView) findViewById(R.id.camera_view);
        mCameraView.init(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mCameraView != null) {
            mCameraView.onResume();
        }
    }

    @Override
    protected void onPause() {
        if (mCameraView != null) {
            mCameraView.onPause();
        }
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mCameraView != null) {
            mCameraView.deinit();
        }
    }
}
