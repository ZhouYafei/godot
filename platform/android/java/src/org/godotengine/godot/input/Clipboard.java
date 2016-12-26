/*************************************************************************/
/*  Clipboard.java                                                       */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2016 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
package org.godotengine.godot.input;

import android.app.Activity;

import android.content.Context;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.util.Log;
import com.u8.sdk.U8SDK;


public class Clipboard {
    private static final String TAG = "Clipboard";

    private Activity activity;
    private String clipdata;
    private static Clipboard instance;
    private ClipboardManager clipboard;

    private Clipboard(final Activity activity) {
        this.activity = activity;
        U8SDK.getInstance().runOnMainThread(new Runnable() {
            @Override
            public void run() {
                clipboard = (ClipboardManager) activity.getSystemService(Context.CLIPBOARD_SERVICE);
            }
        });
        try {
            Thread.sleep(300);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public static Clipboard getInstance(Activity activity) {
        if (instance == null) {
            instance = new Clipboard(activity);
        }
        return instance;
    }

    public boolean setText(String text) {
        final String ss = text;
        if (clipboard != null) {
            ClipData clip = ClipData.newPlainText("text", ss);
            clipboard.setPrimaryClip(clip);
        }
        return true;
    }

    public String getText() {
        if (clipboard != null) {
            ClipData clip = clipboard.getPrimaryClip();
            ClipData.Item item = clip.getItemAt(0);
            String text = item.getText().toString();
            clipdata = text;
        } else {
            if (clipdata == null) {
                clipdata = "请不要频繁操作！";
            }
        }
        Log.e(TAG, "getText: " + clipdata);
        return clipdata;
    }

    public boolean hasText() {
        /* Lazy implementation... */
        return this.getText() != null;
    }
}
