package com.nvision.facetracker;

import android.annotation.SuppressLint;
import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.util.Log;
import android.util.Pair;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.WindowManager;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;
import android.widget.PopupMenu;

import com.nvision.face_tracker_android.R;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;

import androidx.annotation.RawRes;
import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity implements PopupMenu.OnMenuItemClickListener, View.OnClickListener, Animation.AnimationListener, SensorEventListener {
    private static final String TAG = "MainActivity";
    private static final float ENABLE_MAGNET_CLICK_THRESHOLD = 100;
    private static final int ENABLE_MAGNET_CLICK_MS_DELAY = 500;
    private CameraRenderView mCameraView;
    private View mCloseButton;
    private View mSettingsButton;
    private AlphaAnimation mIncreaseOpacityAnimation;
    private AlphaAnimation mDecreaseOpacityAnimation;
    private AlphaAnimation mCurrentAnimation;
    private SensorManager mSensorManager = null;
    private Sensor mMagneticSensor = null;
    private long mMagnetClickedTime = 0;
    private MenuItem mPreviousMenu = null;
    private PopupMenu mMenuPopup;

    public enum EffectType {
        NONE,
        CARTOON,
        GAUSSIAN,
        NOSTALGIA,
        PIXELIZE
    }

    private final Map<Integer, Pair<Integer, EffectType>> mEffectsMap = new HashMap<Integer, Pair<Integer, EffectType>>() {{
        put(R.id.choose_effect_disable, new Pair<>(R.raw.original, EffectType.NONE));
        put(R.id.choose_effect_cartoon, new Pair<>(R.raw.cartoon, EffectType.CARTOON));
        put(R.id.choose_effect_gaussian, new Pair<>(R.raw.gaussian, EffectType.GAUSSIAN));
        put(R.id.choose_effect_nostalgia, new Pair<>(R.raw.nostalgia, EffectType.NOSTALGIA));
        put(R.id.choose_effect_pixelize, new Pair<>(R.raw.pixelize, EffectType.PIXELIZE));
    }};

    @SuppressLint("ClickableViewAccessibility")
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
        //WindowManager.LayoutParams layout = getWindow().getAttributes();
        //layout.screenBrightness = 1.f;
        //getWindow().setAttributes(layout);

        // Prevents screen from dimming/locking.
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mMenuPopup = new PopupMenu(this, mSettingsButton);
        MenuInflater inflater = mMenuPopup.getMenuInflater();
        inflater.inflate(R.menu.main_menu, mMenuPopup.getMenu());
        mMenuPopup.setOnMenuItemClickListener(this);

        mPreviousMenu = mMenuPopup.getMenu().findItem(R.id.choose_effect_disable);

        // Get last used effect
        EffectType effectType = ConfigUtils.getLastEffect(this);
        String shader = "";
        Log.d(TAG, "Restoring last used effect: " + effectType);
        for (Map.Entry<Integer, Pair<Integer, EffectType>> item : mEffectsMap.entrySet()) {
            if (item.getValue().second == effectType) {
                shader = getStringFromRaw(item.getValue().first);
                MenuItem menu = mMenuPopup.getMenu().findItem(item.getKey());
                if (menu != null) {
                    mPreviousMenu = menu;
                }
                break;
            }
        }

        mPreviousMenu.setChecked(true);

        mCameraView = findViewById(R.id.camera_view);
        mCameraView.init(this, shader);
        mCameraView.setOnClickListener(this);

        toggleButtonsVisibility();

        mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        mMagneticSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
        if (mMagneticSensor == null) {
            Log.w(TAG, "This device doesn't have a magnetic sensor");
        }
    }

    private String getStringFromRaw(@RawRes int shaderId) {
        try (InputStream is = getResources().openRawResource(shaderId)) {
            ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
            int i = is.read();
            while (i != -1) {
                byteArrayOutputStream.write(i);
                i = is.read();
            }

            return byteArrayOutputStream.toString();
        } catch (IOException e) {
            Log.e(TAG, "Error reading raw effect", e);
        }

        return "";
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

        if (mCameraView != null)
            mCameraView.onResume();

        if (mMagneticSensor != null)
            mSensorManager.registerListener(this, mMagneticSensor, SensorManager.SENSOR_DELAY_NORMAL);
    }

    @Override
    protected void onPause() {
        if (mCameraView != null)
            mCameraView.onPause();

        if (mMagneticSensor != null)
            mSensorManager.unregisterListener(this);

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
        finish();
    }

    public void showSettings() {
        mMenuPopup.show();
    }

    private void toggleButtonsVisibility() {
        mSettingsButton.clearAnimation();
        mCloseButton.clearAnimation();

        mCurrentAnimation.setAnimationListener(this);

        mCloseButton.startAnimation(mCurrentAnimation);
        mSettingsButton.startAnimation(mCurrentAnimation);
    }

    @Override
    public boolean onMenuItemClick(MenuItem menuItem) {
        int itemId = menuItem.getItemId();

        if (itemId == R.id.choose_viewer) {
            CameraRenderView.nativeSwitchViewer();
            return true;
        }

        if (!mEffectsMap.containsKey(itemId)) {
            return false;
        }

        Pair<Integer, EffectType> effect = mEffectsMap.get(itemId);
        if (mCameraView.setEffect(getStringFromRaw(effect.first))) {
            ConfigUtils.saveCurrentEffect(this, effect.second);
        }

        if (mPreviousMenu != null) {
            mPreviousMenu.setChecked(false);
        }
        menuItem.setChecked(true);
        mPreviousMenu = menuItem;

        return true;
    }

    @Override
    public void onClick(View view) {
        int id = view.getId();
        if (id == R.id.ui_close_button) {
            closeSample();
        } else if (id == R.id.ui_settings_button) {
            if (mCurrentAnimation == mIncreaseOpacityAnimation)
                toggleButtonsVisibility();
            showSettings();
        } else if (id == R.id.camera_view) {
            toggleButtonsVisibility();
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

    @Override
    public void onSensorChanged(SensorEvent sensorEvent) {
        if (Math.abs(sensorEvent.values[0]) > ENABLE_MAGNET_CLICK_THRESHOLD) {
            long current = System.currentTimeMillis();
            if (current - ENABLE_MAGNET_CLICK_MS_DELAY > mMagnetClickedTime) {
                mCameraView.performClick();

                mMagnetClickedTime = current;
            }
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int i) {

    }
}
