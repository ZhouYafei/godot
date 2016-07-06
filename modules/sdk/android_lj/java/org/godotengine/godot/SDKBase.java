////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 此文件暂时无用
////////////////////////////////////////////////////////////////////////////////
// package com.android.godot; // for 1.1
package org.godotengine.godot; // for 2.0

import android.util.Log;
import javax.microedition.khronos.opengles.GL10;

import java.util.HashMap;
import java.util.Map;

import org.json.JSONObject;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;

public abstract class SDKBase extends Godot.SingletonBase {
    protected Activity activity = null;
    private Integer sdkHandler = 0;
    private String sdkCallback = "";

    public SDKBase(Activity p_activity) {
        // register class name and functions to bind
        registerClass("SDK", new String[]{
            "set_callback",
            "login",
            "pay",
            "logout",
            "exit",
            "set_ext_data",
            "get_channel_label",
            "showToast"
        });
        // save main activity
        activity = p_activity;
        // initialize sdk
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                showToast("初始化SDK");
                // initialize sdk
                doInit();
            }
        });
    }

    // public functions
    public void set_callback(int p_handler, String p_callback) {
        this.sdkHandler = p_handler;
        this.sdkCallback = p_callback;
    }

    public void login(final String p_custom_param) {
        activity.runOnUiThread(new Runnable() {
            public void run() {
                doLogin(p_custom_param);
            }
        });
    }

    public void pay(final Dictionary p_data) {
        activity.runOnUiThread(new Runnable() {
            public void run() {
                try {
                    doPay(p_data);
                }
                catch(Exception e) {
                    // TODO:
                    //return "INVALID_PARAM";
                }
            }
        });
    }

    public void logout(final String p_custom_param) {
        activity.runOnUiThread(new Runnable() {
            public void run() {
                doLogout(p_custom_param);
            }
        });
    }

    public void exit() {
        activity.runOnUiThread(new Runnable() {
            public void run() {
                doExit();
            }
        });
    }

    public void set_ext_data(final Dictionary p_data) {
        activity.runOnUiThread(new Runnable() {
            public void run() {
                try {
                    doSetExtData(p_data);
                }
                catch(Exception e) {
                    // TODO:
                    //return "INVALID_PARAM";
                }
            }
        });
    }

    public String get_channel_label() {
        return doGetChannelLabel();
    }

    public void showToast(final String content) {
        activity.runOnUiThread(new Runnable() {
            
            @Override
            public void run() {
                Toast.makeText(activity, content, Toast.LENGTH_LONG).show();               
            }
        });
    }

    protected void handlerCallback(final String p_what, final Dictionary p_data) {
        // GodotLib.calldeferred(sdkHandler, sdkCallback, new Object[]{ p_what, p_data });
        if(p_data != null)
            GodotLib.calldeferred(sdkHandler, sdkCallback, new Object[]{ p_what, p_data });
        else
            GodotLib.calldeferred(sdkHandler, sdkCallback, new Object[]{ p_what });
    }


    // abstract sdk interface
    protected abstract void doInit();
    protected abstract void doLogin(final String p_custom_param);
    protected abstract void doPay(final Dictionary p_data);
    protected abstract void doLogout(final String p_custom_param);
    protected abstract void doExit();
    protected abstract void doSetExtData(Dictionary p_data);
    protected abstract String doGetChannelLabel();
}
