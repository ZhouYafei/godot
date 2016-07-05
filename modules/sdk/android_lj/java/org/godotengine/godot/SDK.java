// package com.android.godot; // for 1.1
package org.godotengine.godot; // for 2.0

import android.app.Activity;
import android.util.Log;
import android.content.Intent;
import javax.microedition.khronos.opengles.GL10;

// Dictionary.java

public class SDK extends Godot.SingletonBase {
    private int sdkHandler = 0;
    private String sdkCallback = "";

    static public Godot.SingletonBase initialize(Activity p_activity) {
        return new SDK(p_activity);
    }

    public SDK(Activity p_activity) {
        // register class name and functions to bind
        registerClass("SDK", new String[]{
            "init",
            "request"
        });
        // you might want to try initializing your singleton here, but android
        // threads are weird and this runs in another thread, so you usually have to do
        p_activity.runOnUiThread(new Runnable() {
                public void run() {
                    //useful way to get config info from engine.cfg
//                     String key = GodotLib.getGlobal("plugin/api_key");
//                     SDK.initializeHere();
                }
        });
    }

    public boolean init(int p_handler, String p_callback) {
        sdkHandler = p_handler;
        sdkCallback = p_callback;

        return true;
    }

    public String request(String p_what, Dictionary p_data) {

        return "OK";
    }

    // forwarded callbacks you can reimplement, as SDKs often need them

    protected void onMainActivityResult(int requestCode, int resultCode, Intent data) {}

    protected void onMainPause() {}
    protected void onMainResume() {}
    protected void onMainDestroy() {}

    protected void onGLDrawFrame(GL10 gl) {}
    protected void onGLSurfaceChanged(GL10 gl, int width, int height) {} // singletons will always miss first onGLSurfaceChanged call
}

    // GodotLib.calldeferred(purchaseCallbackId, "purchase_cancel", new Object[]{});
