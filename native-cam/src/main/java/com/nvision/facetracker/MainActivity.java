package com.nvision.facetracker;

import android.animation.ObjectAnimator;
import android.content.DialogInterface;
import android.os.Bundle;
import android.util.Log;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.WindowManager;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;
import android.widget.PopupMenu;

import com.nvision.face_tracker_android.R;

import java.util.ArrayList;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity implements PopupMenu.OnMenuItemClickListener, View.OnClickListener, Animation.AnimationListener {
    private static final String TAG = "MainActivity";
    private CameraRenderView mCameraView;
    private View mCloseButton;
    private View mSettingsButton;
    AlphaAnimation mIncreaseOpacityAnimation;
    AlphaAnimation mDecreaseOpacityAnimation;
    AlphaAnimation mCurrentAnimation;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);

        mIncreaseOpacityAnimation = new AlphaAnimation(0.0f, 1.0f);
        mIncreaseOpacityAnimation.setDuration(200);
        mIncreaseOpacityAnimation.setFillAfter(true);

        mDecreaseOpacityAnimation = new AlphaAnimation(1.0f, 0.0f);
        mDecreaseOpacityAnimation.setDuration(200);
        mDecreaseOpacityAnimation.setFillAfter(true);

        mCurrentAnimation = mDecreaseOpacityAnimation;
        mCurrentAnimation.setAnimationListener(this);

        mCloseButton = findViewById(R.id.ui_close_button);
        mCloseButton.setOnClickListener(this);

        mSettingsButton = findViewById(R.id.ui_settings_button);
        mSettingsButton.setOnClickListener(this);

        PermissionHelper.requestCameraPermission(this, true);

        setImmersiveSticky();

        getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(new View.OnSystemUiVisibilityChangeListener() {
            @Override
            public void onSystemUiVisibilityChange(int visibility) {
                if ((visibility & View.SYSTEM_UI_FLAG_FULLSCREEN) == 0) {
                    setImmersiveSticky();
                }
            }
        });

        // Forces screen to max brightness.
        WindowManager.LayoutParams layout = getWindow().getAttributes();
        layout.screenBrightness = 1.f;
        getWindow().setAttributes(layout);

        // Prevents screen from dimming/locking.
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mCameraView = findViewById(R.id.camera_view);
        mCameraView.init(this);
        mCameraView.setOnClickListener(this);

        toggleButtonsVisibility();
    }

    private void setImmersiveSticky() {
        getWindow().getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                        | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                        | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_FULLSCREEN
                        | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
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

    public void closeSample() {
        Log.d(TAG, "Leaving VR sample");
        new AlertDialog.Builder(this)
                .setTitle(R.string.closing_app)
                .setMessage(R.string.closing_app_sure)
                .setPositiveButton(R.string.yes, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        finish();
                    }
                })
                .setNegativeButton(R.string.no, null)
                .show();
    }

    public void showSettings(View view) {
        PopupMenu popup = new PopupMenu(this, view);
        MenuInflater inflater = popup.getMenuInflater();
        inflater.inflate(R.menu.main_menu, popup.getMenu());
        popup.setOnMenuItemClickListener(this);
        popup.show();
    }

    private void toggleButtonsVisibility() {
//        Log.d(TAG, "is invisible: " + (mCloseButton.getVisibility() == View.INVISIBLE));
//        int newVisibility = mCloseButton.getVisibility() == View.INVISIBLE ?
//                View.VISIBLE : View.INVISIBLE;
//
//        mCloseButton.setVisibility(newVisibility);
//        mSettingsButton.setVisibility(newVisibility);
//
//        Log.d(TAG, "new visibility: " + mCloseButton.getVisibility());

        ArrayList<ObjectAnimator> animators = new ArrayList<>(); //ArrayList of ObjectAnimators

        mSettingsButton.clearAnimation();
        mCloseButton.clearAnimation();

        mCurrentAnimation.setAnimationListener(this);

        mCloseButton.startAnimation(mCurrentAnimation);
        mSettingsButton.startAnimation(mCurrentAnimation);
    }

    @Override
    public boolean onMenuItemClick(MenuItem menuItem) {
        if (menuItem.getItemId() == R.id.choose_viewer) {
            CameraRenderView.nativeSwitchViewer();
            return true;
        }

        return false;
    }

    @Override
    public void onClick(View view) {
        Log.d(TAG, "View clicked");
        switch (view.getId()) {
            case R.id.ui_close_button:
                closeSample();
                break;
            case R.id.ui_settings_button:
                showSettings(view);
                break;
            case R.id.camera_view:
                toggleButtonsVisibility();
                break;
        }
    }

    @Override
    public void onAnimationStart(Animation animation) {

    }

    @Override
    public void onAnimationEnd(Animation animation) {
        mCurrentAnimation = mCurrentAnimation == mDecreaseOpacityAnimation ?
                mIncreaseOpacityAnimation : mDecreaseOpacityAnimation;
        mCurrentAnimation.setAnimationListener(null);
    }

    @Override
    public void onAnimationRepeat(Animation animation) {

    }
}
