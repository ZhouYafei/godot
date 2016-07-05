package com.ljapps.p636;
import com.xinmei365.game.proxy.XMSplashActivity;
import org.godotengine.godot.Godot;

import android.content.Intent;
import android.graphics.Color;

/**
 * 闪屏Activity
 * 将闪屏Activity继承com.xinmei365.game.proxy.XMSplashActivity,并将该Activity设置为程序启动时的Activity（可参考AndroidManifest.xml文件配置），
 * 并实现getBackgroundColor()及onSplashStop()方法。
 * 闪屏功能为必须接入功能，接入该接口后开发者可以在游戏后台灵活配置闪屏内容、数量、次序等，不配置闪屏图片则不出现闪屏,该接口不会影响程序原有闪屏。
 */
public class SplashActivity extends XMSplashActivity {
	
	//配置默认的闪屏背景色，适配性问题导致闪屏图片不能在所有手机上做到完全覆盖，当无法完全覆盖会将此颜色设置为屏幕背景色
	public int getBackgroundColor() {
		return Color.WHITE;
	}

	@Override
	public void onSplashStop() {
		//闪屏结束，执行进入游戏操作
		Intent intent = new Intent(this, Godot.class);
		startActivity(intent);
		this.finish();
	}
}
