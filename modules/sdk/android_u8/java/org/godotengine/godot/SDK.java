package org.godotengine.godot;

import java.util.Map;
import java.util.HashMap;

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

import org.godotengine.godot.input.Clipboard;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.zip.ZipFile;
import java.util.zip.ZipEntry;
import java.util.Enumeration;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import android.content.pm.ApplicationInfo;

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
			"get_agent_id",
			"get_agent_channel",
			"get_app_id",
			"get_app_key",
			"get_app_signature",
			"get_clipboard",
			"set_clipboard",
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
				//tip("初始化SDK");
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
	// 获取渠道代理id
	public String get_agent_id() {
		return U8SDK.getInstance().getAgentId();
	}
	// 获取渠道代理推广id
	public String get_agent_channel() {
		return U8SDK.getInstance().getAgentChannel();
	}
	// 获取u8server配置的AppID
	public int get_app_id() {
		return U8SDK.getInstance().getAppID();
	}
	// 获取u8server配置的AppKey
	public String get_app_key() {
		return U8SDK.getInstance().getAppKey();
	}
	// 获取应用的签名信息 md5(META-INF/MANIFEST.MF)
	public String get_app_signature() {

		ApplicationInfo appinfo = activity.getApplicationInfo();
		String sourceDir = appinfo.sourceDir;
		ZipFile zipfile = null;
		try {
			zipfile = new ZipFile(sourceDir);
			if(zipfile == null)
				return "";
		    MessageDigest md5 = MessageDigest.getInstance("MD5");
			Enumeration<?> entries = zipfile.entries();
			while (entries.hasMoreElements()) {
				ZipEntry entry = ((ZipEntry) entries.nextElement());
				String entryName = entry.getName();

				if (entryName.startsWith("META-INF/MANIFEST.MF")) {
					// 利用ZipInputStream读取文件
					int size = (int) entry.getSize();
					if (size > 0) {
						BufferedReader br = new BufferedReader(new InputStreamReader(zipfile.getInputStream(entry)));
						char[] s = new char[size];
						br.read(s, 0, size);

						byte[] buf = new byte[size];
						for(int i = 0; i < size; i++)
							buf[i] = (byte) s[i];
						md5.update(buf);
						br.close();  
					}
					break;
				}
			}
			zipfile.close();

			byte[] messageDigest = md5.digest();
			// Create Hex String
			StringBuffer hexString = new StringBuffer();
			for (int i=0; i<messageDigest.length; i++) {
				String hex = Integer.toHexString(0xFF & messageDigest[i]);
				if(hex.length() == 1)
					hexString.append('0');
				hexString.append(hex);
			}
			return hexString.toString();

		} catch (IOException e) {
			e.printStackTrace();
		} catch(NoSuchAlgorithmException e) {
			e.printStackTrace();
		}
		return "";
 	}
	// 获取剪贴板
	public String get_clipboard() {

		return Clipboard.getInstance(activity).getText();
	}
	// 设置剪贴板
	public void set_clipboard(String p_text) {

		Clipboard cb = Clipboard.getInstance(activity);
		cb.setText(p_text);
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
					params.setType(data.getString("type"));
					params.setProductId(data.getString("product_id"));
					params.setProductName(data.getString("product_name"));
					params.setProductDesc(data.getString("product_desc"));
					params.setPrice(data.getInt("price"));
					if(data.containsKey("ratio"))
						params.setRatio(data.getInt("ratio"));
					else
						params.setRatio(0);
					if(data.containsKey("limit_credit_pay"))
						params.setLimitCreditPay(data.getString("limit_credit_pay"));
					else
						params.setLimitCreditPay("1");
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
					if(data.containsKey("title"))
						params.setTitle(data.getString("title"));
					if(data.containsKey("title_url"))
						params.setTitleUrl(data.getString("title_url"));
					if(data.containsKey("source_name"))
						params.setSourceName(data.getString("source_name"));
					if(data.containsKey("source_url"))
						params.setSourceUrl(data.getString("source_url"));
					if(data.containsKey("content"))
						params.setContent(data.getString("content"));
					if(data.containsKey("image_url"))
						params.setImgUrl(data.getString("image_url"));
					if(data.containsKey("dialog_mode"))
						params.setDialogMode(data.getBoolean("dialog_mode"));
					if(data.containsKey("notify_icon"))
						params.setNotifyIcon(data.getInt("notify_icon"));
					if(data.containsKey("notify_icon_text"))
						params.setNotifyIconText(data.getString("notify_icon_text"));
					if(data.containsKey("comment"))
						params.setComment(data.getString("comment"));
					if(data.containsKey("url"))
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
	private Map<String, String> dict_to_map(Dictionary dict) {
		Map<String, String> map = new HashMap<String, String>();
		String[] keys = dict.get_keys();
		for(int i = 0; i < keys.length; i++) {
			String k = keys[i];
			String v = dict.getString(k);
			map.put(k, v);
		}
		return map;
	}
	// 统计分析
	public void analytics(final Dictionary data) {
		Log.d("U8SDK", "SDK.analytics");
		U8SDK.getInstance().runOnMainThread(new Runnable() {
			@Override public void run() {
				try {
					String what = data.getString("what");
					if(what.equals("start_level"))
						U8Analytics.getInstance().startLevel(data.getString("level"));
					else if(what.equals("fail_level"))
						U8Analytics.getInstance().failLevel(data.getString("level"));
					else if(what.equals("finish_level"))
						U8Analytics.getInstance().finishLevel(data.getString("level"));
					else if(what.equals("start_task"))
						U8Analytics.getInstance().startTask(data.getString("task"), data.getString("type"));
					else if(what.equals("fail_task"))
						U8Analytics.getInstance().failTask(data.getString("task"));
					else if(what.equals("finish_task"))
						U8Analytics.getInstance().finishTask(data.getString("task"));
					else if(what.equals("pay"))
						U8Analytics.getInstance().pay(data.getDouble("money"), data.getInt("num"));
					else if(what.equals("buy"))
						U8Analytics.getInstance().buy(data.getString("item"), data.getInt("num"), data.getDouble("price"));
					else if(what.equals("use"))
						U8Analytics.getInstance().use(data.getString("item"), data.getInt("num"), data.getDouble("price"));
					else if(what.equals("bonus"))
						U8Analytics.getInstance().bonus(data.getString("item"), data.getInt("num"), data.getDouble("price"), data.getInt("trigger"));
					else if(what.equals("login"))
						U8Analytics.getInstance().login(data.getString("user_id"));
					else if(what.equals("logout"))
						U8Analytics.getInstance().logout();
					else if(what.equals("levelup"))
						U8Analytics.getInstance().levelup(data.getInt("level"));
					else if(what.equals("event")) {
						String eventId = data.getString("event_id");
						if(data.containsKey("label")) {
							// public void onEvent(String eventId, String label);
							U8Analytics.getInstance().onEvent(eventId, data.getString("label"));
						}
						else if(data.containsKey("map")) {
							// public void onEvent(String eventId, Map<String, String> map);
							Map<String, String> map = dict_to_map(data.getDictionary("map"));
							U8Analytics.getInstance().onEvent(eventId, map);
						} else {
							// public void onEvent(String eventId);
							U8Analytics.getInstance().onEvent(eventId);
						}
					}
					else if(what.equals("event_value")) {
						// public void onEventValue(String eventId, Map<String, String> map, int duration);
						String eventId = data.getString("event_id");
						Map<String, String> map = dict_to_map(data.getDictionary("map"));
						int duration = data.getInt("duration");
						U8Analytics.getInstance().onEventValue(eventId, map, duration);
					}
					else if(what.equals("event_begin")) {
						String eventId = data.getString("event_id");
						if(data.containsKey("label")) {
							// public void onEventBegin(String eventId, String label);
							String label = data.getString("label");
							U8Analytics.getInstance().onEventBegin(eventId, label);
						} else {
							// public void onEventBegin(String eventId);
							U8Analytics.getInstance().onEventBegin(eventId);
						}
					}
					else if(what.equals("event_end")) {
						String eventId = data.getString("event_id");
						if(data.containsKey("label")) {
							// public void onEventEnd(String eventId, String label);
							String label = data.getString("label");
							U8Analytics.getInstance().onEventEnd(eventId, label);
						} else {
							// public void onEventEnd(String eventId);
							U8Analytics.getInstance().onEventEnd(eventId);					
						}
					}
					else if(what.equals("event_kv_begin")) {
						// public void onKVEventBegin(String eventId, Map<String, String> map, String label);
						String eventId = data.getString("event_id");
						Map<String, String> map = dict_to_map(data.getDictionary("map"));
						String label = data.getString("label");
						U8Analytics.getInstance().onKVEventBegin(eventId, map, label);
					}
					else if(what.equals("event_kv_end")) {
						// public void onKVEventEnd(String eventId, String label);
						String eventId = data.getString("event_id");
						String label = data.getString("label");
						U8Analytics.getInstance().onKVEventEnd(eventId, label);
					}
				} catch (Exception e) {
					e.printStackTrace();
				}
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
