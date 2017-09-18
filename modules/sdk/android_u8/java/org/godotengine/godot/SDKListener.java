package org.godotengine.godot;

// u8sdk begin
import com.u8.sdk.*;
import com.u8.sdk.plugin.*;
import com.u8.sdk.log.Log;
import com.u8.sdk.verify.UToken;
// u8sdk end

import java.util.HashMap;

public class SDKListener implements IU8SDKListener {

	private SDK sdk;
	// 当前是否为切换帐号
	private boolean isSwitchAccount = false;

	private static HashMap<Integer, String> U8CodeStrings = new HashMap<Integer, String>() {
		{
			put(U8Code.CODE_NO_NETWORK, "no_network");
			put(U8Code.CODE_INIT_SUCCESS, "init_success");
			put(U8Code.CODE_INIT_FAIL, "init_fail");
			put(U8Code.CODE_UNINIT, "uninit");
			put(U8Code.CODE_LOGIN_SUCCESS, "login_success");
			put(U8Code.CODE_LOGIN_FAIL, "login_fail");
			put(U8Code.CODE_LOGIN_TIMEOUT, "login_timeout");
			put(U8Code.CODE_UNLOGIN, "unlogin");
			put(U8Code.CODE_LOGOUT_SUCCESS, "logout_success");
			put(U8Code.CODE_LOGOUT_FAIL, "logout_fail");
			put(U8Code.CODE_PAY_SUCCESS, "pay_success");
			put(U8Code.CODE_PAY_FAIL, "pay_fail");
			put(U8Code.CODE_TAG_ADD_SUC, "tag_add_suc");
			put(U8Code.CODE_TAG_ADD_FAIL, "tag_add_fail");
			put(U8Code.CODE_TAG_DEL_SUC, "tag_del_suc");
			put(U8Code.CODE_TAG_DEL_FAIL, "tag_del_fail");
			put(U8Code.CODE_ALIAS_ADD_SUC, "alias_add_suc");
			put(U8Code.CODE_ALIAS_ADD_FAIL, "alias_add_fail");
			put(U8Code.CODE_ALIAS_REMOVE_SUC, "alias_remove_suc");
			put(U8Code.CODE_ALIAS_REMOVE_FAIL, "alias_remove_fail");
			put(U8Code.CODE_PUSH_MSG_RECIEVED, "push_msg_recieved");
			put(U8Code.CODE_PARAM_ERROR, "param_error");
			put(U8Code.CODE_PARAM_NOT_COMPLETE, "param_not_complete");
			put(U8Code.CODE_SHARE_SUCCESS, "share_success");
			put(U8Code.CODE_SHARE_FAILED, "share_failed");
			put(U8Code.CODE_UPDATE_SUCCESS, "update_success");
			put(U8Code.CODE_UPDATE_FAILED, "update_failed");
			put(U8Code.CODE_REAL_NAME_REG_SUC, "real_name_reg_suc");
			put(U8Code.CODE_REAL_NAME_REG_FAILED, "real_name_reg_failed");
			put(U8Code.CODE_ADDICTION_ANTI_RESULT, "addiction_anti_result");
			put(U8Code.CODE_PUSH_ENABLED, "push_enabled");
			put(U8Code.CODE_POST_GIFT_SUC, "post_gift_suc");
			put(U8Code.CODE_POST_GIFT_FAILED, "post_gift_failed");
		}
	};

	public SDKListener(SDK sdk) {
		this.sdk = sdk;
	}

	// SDK各种操作之结果的回调
	@Override public void onResult(final int code, final String msg) {

		U8SDK.getInstance().runOnMainThread(new Runnable() {
			@Override public void run() {
				Dictionary d = new Dictionary();
				d.put("code", code);
				d.put("what", U8CodeStrings.get(code));
				d.put("msg", msg);

				switch(code) {
					case U8Code.CODE_INIT_SUCCESS:
					sdk.sendCallback(SDK.CALLBACK_INIT, d);
					return;
					case U8Code.CODE_INIT_FAIL:
					sdk.tip("SDK初始化失败");
					break;
					case U8Code.CODE_LOGIN_FAIL:
					// 这里不需要提示，一般SDK有提示
					// sdk.tip("SDK登录失败");
					break;
					case U8Code.CODE_SHARE_SUCCESS:
					//sdk.tip("分享成功");
					break;
					case U8Code.CODE_SHARE_FAILED:
					sdk.tip("分享失败");
					break;
					case U8Code.CODE_PAY_FAIL:
					sdk.tip("支付失败");
					break;
					case U8Code.CODE_PAY_SUCCESS:
					// 一般这里不用提示
					// context.tip("支付成功,到账时间可能稍有延迟");
 					break;
				}
				sdk.sendCallback(SDK.CALLBACK_RESULT, d);
			}			
		});
	}

	// 此接口已经废弃
	@Override public void onInitResult(InitResult result) {}

	// SDK登录成功的回调
	@Override public void onLoginResult(String data) {
		Log.d("U8SDK", "SDK 登录成功,不用做处理，在onAuthResult中处理登录成功, 参数如下:");
		Log.d("U8SDK", data);
		this.isSwitchAccount = false;
		//sdk.tip("SDK登录成功");
                // 通知引擎，sdk登录成功
                Dictionary d = new Dictionary();
                d.put("login_sdk", true);
                sdk.sendCallback(SDK.CALLBACK_LOGIN, d);
	}

	// 切换帐号，需要回到登录界面，并弹出SDK登录界面
	@Override public void onSwitchAccount() {
		sdk.sendCallback(SDK.CALLBACK_SWITCH_LOGIN, null);
	}

	// 切换帐号，并登录成功，到这里和Login的回调onLoginResult一样
	@Override public void onSwitchAccount(String data) {
		Log.d("U8SDK", "SDK 切换帐号并登录成功,不用做处理，在onAuthResult中处理登录成功, 参数如下:");
		Log.d("U8SDK", data);
		this.isSwitchAccount = true;		
		//sdk.tip( "切换帐号成功");
	}

	// 登出，需要回到登录界面，并弹出SDK登录界面
	@Override public void onLogout() {
		sdk.sendCallback(SDK.CALLBACK_LOGOUT, null);
	}

	// SDK登录成功之后，去U8Server进行登录认证
	@Override public void onAuthResult(UToken authResult) {

		if(!authResult.isSuc())
			sdk.tip("SDK登录认证失败,确认U8Server是否配置");

		Dictionary d = new Dictionary();
		try {
			d.put("success", authResult.isSuc());
			d.put("switch_account", isSwitchAccount);
			d.put("extension", authResult.getExtension());

			if(authResult.isSuc()) {
				d.put("user_id", authResult.getUserID());
				d.put("user_name", authResult.getUsername());
				d.put("sdk_user_id", authResult.getSdkUserID());
				d.put("sdk_user_name", authResult.getSdkUsername());
				d.put("token", authResult.getToken());
			}
		} catch(Exception e) {
			e.printStackTrace();
		}
		sdk.sendCallback(SDK.CALLBACK_LOGIN, d);
	}
	
	// 对于手机网游,不需要实现这个接口，因为网游支付是通过服务器回调通知加虚拟币的。
	// 这个接口主要用于单机游戏，作为单机的支付结果回调接口
	@Override public void onPayResult(PayResult result) {

		Dictionary d = new Dictionary();
		d.put("product_id", result.getProductID());
		d.put("product_name", result.getProductName());
		d.put("extension", result.getExtension());
		sdk.sendCallback(SDK.CALLBACK_PAY, d);
	}
}
