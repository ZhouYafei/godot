package org.godotengine.godot;

// u8sdk begin
import com.u8.sdk.*;
import com.u8.sdk.plugin.*;
import com.u8.sdk.log.Log;
import com.u8.sdk.verify.UToken;
// u8sdk end

import android.app.Activity;
import android.content.Intent;
import android.widget.Toast;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.telephony.TelephonyManager;

/***
 * 记得将游戏工程中的AndroidManifest.xml中application节点，增加一个android:name="U8Application"
 * 如果游戏有自己的Application。那么通过实现IApplicationListener接口来实现，而不要使用继承Application。
 * 然后将自己的Application类，配置到AndroidManifest.xml中的meta-data节点中，name为"U8_Game_Application"
 * @author xiaohei
 *
 */
public class SDK extends Godot.SingletonBase {
	public final static String CALLBACK_INIT = "init";
	public final static String CALLBACK_LOGIN = "login";
	public final static String CALLBACK_SWITCH_LOGIN = "switch_login";
	public final static String CALLBACK_LOGOUT = "logout";
	public final static String CALLBACK_PAY = "pay";
	public final static String CALLBACK_RESULT = "result";

	private Integer sdkHandler = 0;
	private String sdkCallback = "";
	private Activity activity = null;

//	 private Dialog dialog = null;

	static public Godot.SingletonBase initialize(Activity p_activity) {
		return new SDK(p_activity);
	}

	public SDK(Activity p_activity) {
		// register class name and functions to bind
		registerClass("SDK", new String[]{
			// SDK common
			"init",
			"tip",
			"get_network_type",
			"is_support",
			"get_curr_channel",
			"get_logic_channel",
			"get_app_id",
			"get_app_key",
			// U8User plugin
			"login",
			"login_custom",
			"switch_login",
			"logout",
			"show_user_center",
			"submit_extra",
			"exit",
			// U8Pay plugin
			"pay",
			// U8Push plugin
			"start_push",
			"stop_push",
			"add_tags",
			"remove_tags",
			"add_alias",
			"remove_alias",
			// U8Share plugin
			"share",
			// U8Analytics plugin
			"analytics",
			// U8Download plugin
			"download",
		});
		// save main activity
		activity = p_activity;

		activity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				tip("初始化SDK");
				// initialize sdk
				initSDK();
			}
		});
	}
	///////////////////////////////////////////////////////////////////////////
	// U8 SDK common
	///////////////////////////////////////////////////////////////////////////
	// 初始化，设置engine的回调对象/函数
	public void init(int p_handler, String p_callback) {
		Log.d("U8SDK", "SDK.init");
		this.sdkHandler = p_handler;
		this.sdkCallback = p_callback;
	}
	// 弹出系统提示（toast）
	public void tip(final String tip) {
		U8SDK.getInstance().runOnMainThread(new Runnable() {
			@Override public void run() {
				Toast.makeText(activity, tip, Toast.LENGTH_SHORT).show();
			}
		});
	}
	// 获取网络连接类型
	public String get_network_type() {
		ConnectivityManager connectivityManager = (ConnectivityManager) activity.getSystemService(Context.CONNECTIVITY_SERVICE);
		if(connectivityManager == null)
			return "none";
		NetworkInfo info = connectivityManager.getActiveNetworkInfo();
		if(info == null)
			return "none";
		int type = info.getType();
		int sub = info.getSubtype();

		switch(type) {
			case ConnectivityManager.TYPE_WIFI:
				return "wifi";
			case ConnectivityManager.TYPE_ETHERNET:
				return "ethernet";
			case ConnectivityManager.TYPE_BLUETOOTH:
				return "bluetooth";
			case ConnectivityManager.TYPE_WIMAX:
				return "wimax";
			case ConnectivityManager.TYPE_MOBILE: {
				switch(sub) {
					case TelephonyManager.NETWORK_TYPE_1xRTT:
					case TelephonyManager.NETWORK_TYPE_CDMA:
					case TelephonyManager.NETWORK_TYPE_EDGE:
					case TelephonyManager.NETWORK_TYPE_GPRS:
					case TelephonyManager.NETWORK_TYPE_IDEN:
						return "2G";
					case TelephonyManager.NETWORK_TYPE_HSDPA:
					case TelephonyManager.NETWORK_TYPE_HSUPA:
					case TelephonyManager.NETWORK_TYPE_HSPA:
					case TelephonyManager.NETWORK_TYPE_EVDO_0:
					case TelephonyManager.NETWORK_TYPE_EVDO_A:
					case TelephonyManager.NETWORK_TYPE_UMTS:
						return "3G";
					case TelephonyManager.NETWORK_TYPE_LTE: // 4G
					case TelephonyManager.NETWORK_TYPE_EHRPD: // 3G ++ interop / with 4G
					case TelephonyManager.NETWORK_TYPE_HSPAP: // 3G ++ but marketed as 4G
						return "4G";
					default:
						return "unknown";
				}
			}
			default:
				return "unknown";
		}
	}
	// U8SDK 初始化，必须在onCreate中调用
	private void initSDK() {
		U8SDK.getInstance().setSDKListener(new SDKListener(this));
		U8SDK.getInstance().init(activity);
		U8SDK.getInstance().onCreate();
	}
	// 发送回调给引擎处理
	public void sendCallback(final String p_what, final Dictionary p_data) {
		// GodotLib.calldeferred(sdkHandler, sdkCallback, new Object[]{ p_what, p_data });
		if(p_data != null)
			GodotLib.calldeferred(sdkHandler, sdkCallback, new Object[]{ p_what, p_data });
		else
			GodotLib.calldeferred(sdkHandler, sdkCallback, new Object[]{ p_what });
	}
	// 判断U8SDK各插件是否支持功能
	public boolean is_support(String plugin, String what) {

		if(plugin.equals("user"))
			return U8User.getInstance().isSupport(what);
		else if(plugin.equals("pay"))
			return U8Pay.getInstance().isSupport(what);
		else if(plugin.equals("push"))
			return U8Push.getInstance().isSupport(what);
		else if(plugin.equals("share"))
			return U8Share.getInstance().isSupport(what);
		else if(plugin.equals("analytics"))
			return U8Analytics.getInstance().isSupport(what);
		else if(plugin.equals("download"))
			return U8Download.getInstance().isSupport(what);
		else
			return false;
	}
	// 获取当前接入的渠道号
	public int get_curr_channel() {
		return U8SDK.getInstance().getCurrChannel();
	}
	// 获取CPS,CPA,CPD等非SDK联运渠道的逻辑渠道号
	public int get_logic_channel() {
		return U8SDK.getInstance().getLogicChannel();
	}
	// 获取u8server配置的AppID
	public int get_app_id() {
		return U8SDK.getInstance().getAppID();
	}
	// 获取u8server配置的AppKey
	public String get_app_key() {
		return U8SDK.getInstance().getAppKey();
	}
	///////////////////////////////////////////////////////////////////////////
	// U8User plugin implements
	///////////////////////////////////////////////////////////////////////////
	// 登录接口
	public void login() {
		Log.d("U8SDK", "SDK.login");
		U8SDK.getInstance().runOnMainThread(new Runnable() {
			@Override public void run() {
				U8User.getInstance().login();
			}
		});
	}
	// 登录接口
	public void login_custom(final String extension) {
		Log.d("U8SDK", "SDK.login_custom");
		U8SDK.getInstance().runOnMainThread(new Runnable() {
			@Override public void run() {
				U8User.getInstance().login(extension);
			}
		});
	}
	// 切换帐号接口
	public void switch_login() {
		Log.d("U8SDK", "SDK.switch_login");
		U8SDK.getInstance().runOnMainThread(new Runnable() {
			@Override public void run() {
				U8User.getInstance().switchLogin();
			}
		});
	}
	// 登出接口
	public void logout() {
		Log.d("U8SDK", "SDK.logout");
		U8SDK.getInstance().runOnMainThread(new Runnable() {
			@Override public void run() {
				U8User.getInstance().logout();
			}
		});
	}
	// 显示用户中心接口
	public void show_user_center() {
		Log.d("U8SDK", "SDK.show_user_center");
		U8SDK.getInstance().runOnMainThread(new Runnable() {
			@Override public void run() {
				U8User.getInstance().showAccountCenter();
			}
		});
	}
	// 提交扩展数据
	public void submit_extra(final Dictionary data) {
		Log.d("U8SDK", "SDK.submit_extra");
		U8SDK.getInstance().runOnMainThread(new Runnable() {
			@Override public void run() {

				UserExtraData extra = new UserExtraData();
				try {
					extra.setDataType(data.getInt("type"));
					extra.setRoleID(data.getString("id"));
					extra.setRoleName(data.getString("name"));
					extra.setRoleLevel(data.getString("level"));
					extra.setServerID(data.getInt("server_id"));
					extra.setServerName(data.getString("server_name"));

					if(data.containsKey("money"))
						extra.setMoneyNum(data.getInt("money"));
					if(data.containsKey("create_time"))
						extra.setRoleCreateTime(data.getInt("create_time"));
					if(data.containsKey("levelup_time"))
						extra.setRoleLevelUpTime(data.getInt("levelup_time"));

				} catch(Exception e) {
					e.printStackTrace();
				}
				U8User.getInstance().submitExtraData(extra);
			}
		});
	}
	// SDK退出接口
	public void exit() {
		Log.d("U8SDK", "SDK.exit");
		 U8SDK.getInstance().runOnMainThread(new Runnable() {
			@Override public void run() {
				U8User.getInstance().exit();
			}
		}); 
	}

	///////////////////////////////////////////////////////////////////////////
	// U8Pay plugin implements
	///////////////////////////////////////////////////////////////////////////
	// 支付接口
	public void pay(final Dictionary data){
		Log.d("U8SDK", "SDK.pay");
		U8SDK.getInstance().runOnMainThread(new Runnable() {
			@Override public void run() {

				PayParams params = new PayParams();
				try {
					params.setProductId(data.getString("product_id"));
					params.setProductName(data.getString("product_name"));
					params.setProductDesc(data.getString("product_desc"));
					params.setPrice(data.getInt("price"));
					if(data.containsKey("ratio"))
						params.setRatio(data.getInt("ratio"));
					else
						params.setRatio(0);
					params.setBuyNum(data.getInt("count"));
					params.setCoinNum(data.getInt("coin"));
					params.setServerId(data.getString("server_id"));
					params.setServerName(data.getString("server_name"));
					params.setRoleId(data.getString("role_id"));
					params.setRoleName(data.getString("role_name"));
					params.setRoleLevel(data.getInt("role_level"));
					if(data.containsKey("notify_url"))
						params.setPayNotifyUrl(data.getString("notify_url"));
					else
						params.setPayNotifyUrl("");
					params.setVip(data.getString("vip"));
					params.setOrderID(data.getString("order_id"));
					params.setExtension(data.getString("extension"));
				} catch(Exception e) {
					e.printStackTrace();
				}
				U8Pay.getInstance().pay(params);
			}
		});		 
	}
	///////////////////////////////////////////////////////////////////////////
	// U8Push plugin implements
	///////////////////////////////////////////////////////////////////////////
	// 开始推送功能
	public void start_push() {
		Log.d("U8SDK", "SDK.start_push");
		U8SDK.getInstance().runOnMainThread(new Runnable() {
			@Override public void run() {
				U8Push.getInstance().startPush();
			}
		}); 
	}
	// 停止推送功能
	public void stop_push() {
		Log.d("U8SDK", "SDK.stop_push");
		U8SDK.getInstance().runOnMainThread(new Runnable() {
			@Override public void run() {
				U8Push.getInstance().stopPush();
			}
		}); 
	}
	// 增加标签
	public void add_tags(final String[] tags) {
		Log.d("U8SDK", "SDK.add_tags");
		U8SDK.getInstance().runOnMainThread(new Runnable() {
			@Override public void run() {
				U8Push.getInstance().addTags(tags);
			}
		}); 
	}
	// 删除标签
	public void remove_tags(final String[] tags) {
		Log.d("U8SDK", "SDK.remove_tags");
		U8SDK.getInstance().runOnMainThread(new Runnable() {
			@Override public void run() {
				U8Push.getInstance().removeTags(tags);
			}
		}); 
	}
	// 增加别名
	public void add_alias(final String alias) {
		Log.d("U8SDK", "SDK.add_alias");
		U8SDK.getInstance().runOnMainThread(new Runnable() {
			@Override public void run() {
				U8Push.getInstance().addAlias(alias);
			}
		}); 
	}
	// 删除别名
	public void remove_alias(final String alias) {
		Log.d("U8SDK", "SDK.remove_alias");
		U8SDK.getInstance().runOnMainThread(new Runnable() {
			@Override public void run() {
				U8Push.getInstance().removeAlias(alias);
			}
		}); 
	}
	///////////////////////////////////////////////////////////////////////////
	// U8Share plugin implements
	///////////////////////////////////////////////////////////////////////////
	// 分享接口
	public void share(final Dictionary data) {
		Log.d("U8SDK", "SDK.share");
		U8SDK.getInstance().runOnMainThread(new Runnable() {
			@Override public void run() {

				ShareParams params = new ShareParams();
				try {
					params.setTitle(data.getString("title"));
					params.setTitleUrl(data.getString("title_url"));
					params.setSourceName(data.getString("source_name"));
					params.setSourceUrl(data.getString("source_url"));
					params.setContent(data.getString("content"));
					params.setImgUrl(data.getString("image_url"));
					params.setDialogMode(data.getBoolean("dialog_mode"));
					params.setNotifyIcon(data.getInt("notify_icon"));
					params.setNotifyIconText(data.getString("notify_icon_text"));
					params.setComment(data.getString("comment"));
					params.setUrl(data.getString("url"));
					if(data.containsKey("platform"))
						params.setPlatform(data.getString("platform"));
					if(data.containsKey("silent"))
						params.setSilent(data.getBoolean("silent"));
				} catch(Exception e) {
					e.printStackTrace();
				}
				U8Share.getInstance().share(params);
			}
		});		 
	}
	///////////////////////////////////////////////////////////////////////////
	// U8Analytics plugin implements
	///////////////////////////////////////////////////////////////////////////
	// 统计分析
	public void analytics(final Dictionary data) {
		Log.d("U8SDK", "SDK.analytics");
		U8SDK.getInstance().runOnMainThread(new Runnable() {
			@Override public void run() {
				String type = data.getString("type");
				if(type.equals("start_level"))
					U8Analytics.getInstance().startLevel(data.getString("level"));
				else if(type.equals("fail_level"))
					U8Analytics.getInstance().failLevel(data.getString("level"));
				else if(type.equals("finish_level"))
					U8Analytics.getInstance().finishLevel(data.getString("level"));
				else if(type.equals("start_task"))
					U8Analytics.getInstance().startTask(data.getString("task"), data.getString("type"));
				else if(type.equals("fail_task"))
					U8Analytics.getInstance().failTask(data.getString("task"));
				else if(type.equals("finish_task"))
					U8Analytics.getInstance().finishTask(data.getString("task"));
				else if(type.equals("pay"))
					U8Analytics.getInstance().pay(data.getDouble("money"), data.getInt("num"));
				else if(type.equals("buy"))
					U8Analytics.getInstance().buy(data.getString("item"), data.getInt("num"), data.getDouble("price"));
				else if(type.equals("use"))
					U8Analytics.getInstance().use(data.getString("item"), data.getInt("num"), data.getDouble("price"));
				else if(type.equals("bonus"))
					U8Analytics.getInstance().bonus(data.getString("item"), data.getInt("num"), data.getDouble("price"), data.getInt("trigger"));
				else if(type.equals("login"))
					U8Analytics.getInstance().login(data.getString("user_id"));
				else if(type.equals("logout"))
					U8Analytics.getInstance().logout();
				else if(type.equals("levelup"))
					U8Analytics.getInstance().levelup(data.getInt("level"));
			}
		});
	}
	///////////////////////////////////////////////////////////////////////////
	// U8Download plugin implements
	///////////////////////////////////////////////////////////////////////////
	// 下载
	public void download(final String url, final boolean showConfirm, final boolean force) {
		Log.d("U8SDK", "SDK.download");
		U8SDK.getInstance().runOnMainThread(new Runnable() {
			@Override public void run() {
				U8Download.getInstance().download(url, showConfirm, force);
			}
		}); 
	}
	// Activity 系统回调处理
	protected void onMainActivityResult(int requestCode, int resultCode, Intent data){
		U8SDK.getInstance().onActivityResult(requestCode, resultCode, data);
	}
	protected void onMainStart(){
		U8SDK.getInstance().onStart();
	}
	protected void onMainPause(){
		U8SDK.getInstance().onPause();
	}
	protected void onMainResume(){
		U8SDK.getInstance().onResume();
	}
	protected void onMainNewIntent(Intent newIntent){
		U8SDK.getInstance().onNewIntent(newIntent);
	}
	protected void onMainStop(){
		U8SDK.getInstance().onStop();
	}
	protected void onMainDestroy(){
		U8SDK.getInstance().onDestroy();
	}
	protected void onMainRestart(){
		U8SDK.getInstance().onRestart();
	}
}
