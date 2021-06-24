package com.nvision.facetracker;

import android.content.Context;
import android.content.SharedPreferences;

import static android.content.Context.MODE_PRIVATE;

public class ConfigUtils {

    private static final String PREFERENCES_NAME = "sailsXRPreferences";
    private static final String PREFERENCE_EFFECT = "preference.effect";

    public static void saveCurrentEffect(Context context, MainActivity.EffectType effectType) {
        SharedPreferences.Editor editor = context.getSharedPreferences(PREFERENCES_NAME, MODE_PRIVATE).edit();
        editor.putString(PREFERENCE_EFFECT, effectType.toString());
        editor.apply();
    }

    public static MainActivity.EffectType getLastEffect(Context context) {
        SharedPreferences reader = context.getSharedPreferences(PREFERENCES_NAME, MODE_PRIVATE);
        return MainActivity.EffectType.valueOf(reader.getString(PREFERENCE_EFFECT, "NONE"));
    }

}
