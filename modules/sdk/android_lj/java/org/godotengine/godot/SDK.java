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


// ljsdk start
import com.xinmei365.game.proxy.GameProxy;
import com.xinmei365.game.proxy.PayCallBack;
import com.xinmei365.game.proxy.XMUser;
import com.xinmei365.game.proxy.XMUserListener;
import com.xinmei365.game.proxy.XMUtils;
import com.xinmei365.game.proxy.exit.LJExitCallback;
import com.xinmei365.game.proxy.init.XMInitCallback;
import com.xinmei365.game.proxy.pay.XMPayParams;
import com.xinmei365.game.proxy.util.LogUtils;
// ljsdk end

// Dictionary.java

/**
 * 游戏主SDK类
 * 
 * SDK接入说明，请在开始接入前仔细阅读： 客户端必接接口：登陆、支付、上传游戏数据、登出、退出、生命周期通知，详细说明如下
 * 
 * 1.需在onCreate、onStart、onResume、onPause、onStop、onDestory、
 * onActivityResult中调用SDK相应方法，接入完成后请确认。
 * onApplicaitonInit方法在游戏启动时调用，onApplicaitonDestory方法在游戏退出时调用。
 * 2.登陆成功获取到SDK返回的用户信息后（如uid、token等），需上传用户信息到游戏服务器，游戏服务器与我方服务器进行通信，
 * 进行用户信息校验，收到校验成功的结果后，方可认为登录成功。
 * 3.支付分定额支付及非定额支付，在充值过程中定额支付不允许用户修改充值金额，非定额支付允许用户选择充值金额，游戏方可根据自身需求进行选择；
 * 4.上传数据接口需在三处调用，分别为进入服务器、玩家升级、玩家创建用户角色，有如下三点需注意：
 * a.游戏中该接口必须调用三次，传入不同的_id，以区分不同的上传数据；
 * b.若游戏中无对应接口功能，如游戏中无需创建角色，则可根据自身情况在合适位置进行调用，如登录成功后；
 * c.在上传游戏数据时，若存在无对应字段值的情况，可传入默认值，具体参见接口说明； 
 * 5.登出操作用于用户切换帐号等操作，有如下不同处理：
 * a.游戏中存在登出或者切换帐号的按钮，则可在点击按钮时进行登出接口调用，在登出回调中进行重新登录等操作
 * b.游戏中不存在登出或者切换帐号的按钮时，建议修改游戏添加登出或切换帐号按钮，若实在无法添加，可在退出游戏前调用登出接口，
 * 这种情况下会存在部分渠道会审核不通过的情况，需游戏方与渠道去进行沟通。 
 * 6.在游戏退出前调用退出接口，有如下不同处理：
 * a.渠道存在退出界面，如91、360等，会收到onExit()通知，此时游戏方只需在回调中进行退出游戏操作即可，无需再弹退出界面；
 * b.渠道不存在退出界面，
 * 如百度移动游戏等，会收到onNo3rdExiterProvide()通知，此时游戏方需在回调中弹出自己的游戏退出确认界面，否则会出现渠道审核不通过情况；
 * 
 * 游戏APK上传到我方开发者后台进行打包时，后台会检测SDK接口是否接入完整，未接入完整拒绝上传。 详细信息请参考SDK官方接入文档
 * http://doc.sdk.feiliu.com/docs_java_feiliu
 */
public class SDK extends Godot.SingletonBase implements XMUserListener {
    private int sdkHandler = 0;
    private String sdkCallback = "";
    private Activity activity = null;
    private Dialog dialog = null;
    private XMUser xmuser;

    static public Godot.SingletonBase initialize(Activity p_activity) {
        return new SDK(p_activity);
    }

    public SDK(Activity p_activity) {
        // register class name and functions to bind
        registerClass("SDK", new String[]{
            "request",
            "showToast"
        });
        // save main activity
        activity = p_activity;
    }

    private void handlerCallback(String p_what, Dictionary p_data) {
        GodotLib.calldeferred(sdkHandler, sdkCallback, new Object[]{ p_what, p_data });
    }

    /**
     * 初始化
     */
    private void doInit() {
        // you might want to try initializing your singleton here, but android
        // threads are weird and this runs in another thread, so you usually have to do
        activity.runOnUiThread(new Runnable() {
                public void run() {
                    // useful way to get config info from engine.cfg
                    // String key = GodotLib.getGlobal("plugin/api_key");
                    // 需要先调用初始化 之后才可以调用其它的方法 
                    GameProxy.getInstance().init(activity, new XMInitCallback() {

                        @Override
                        public void onInitSuccess() {
                            showToast("初始化成功");
                            Dictionary data = new Dictionary();
                            data.put("result", true);
                            handlerCallback("init", data);
                        }

                        @Override
                        public void onInitFailure(String msg) {
                            showToast("初始化失败");
                            Dictionary data = new Dictionary();
                            data.put("result", false);
                            handlerCallback("init", data);
                        }
                    });
                    GameProxy.getInstance().onCreate(activity);
                    GameProxy.getInstance().setUserListener(activity, SDK.this);
                }
        });
    }

    /**
     * 登陆接口说明：
     * 
     * @param activity
     *            当前activity
     * @param customObject
     *            用户自定义参数，在登陆回调中原样返回
     */
    private void doLogin(Object p_object) {
        LogUtils.d("doLogin");
        GameProxy.getInstance().login(activity, p_object);
    }

    /**
     * 定额支付接口说明
     * 
     * @param context
     *            上下文Activity
     * @param total
     *            定额支付总金额，单位为人民币分
     * @param unitName
     *            游戏币名称，如金币、钻石等
     * @param count
     *            购买商品数量，如100钻石，传入100；10魔法石，传入10
     * @param callBackInfo
     *            游戏开发者自定义字段，会与支付结果一起通知到游戏服务器，游戏服务器可通过该字段判断交易的详细内容（金额 角色等）
     * @param callBackUrl
     *            支付结果通知地址，支付完成后我方后台会向该地址发送支付通知
     * @param payCallBack
     *            支付回调接口
     */
    private void doPay(Dictionary p_data) { 
        Integer amount = Integer.parseInt(p_data.get("amount").toString());
        String item_name = p_data.get("item_name").toString();
        Integer point_count = Integer.parseInt(p_data.get("point_count").toString());
        String point_name = p_data.get("point_name").toString();
        String custom_param = p_data.get("custom_param").toString();
        String callback_url = p_data.get("callback_url").toString();

        XMPayParams params = new XMPayParams();
        params.setAmount(amount);
        params.setItemName(item_name);
        params.setCount(point_count);
        params.setChargePointName(point_name);
        params.setCustomParam(custom_param);
        params.setCallbackUrl(callback_url);
        GameProxy.getInstance().pay(activity, params, new PayCallBack() {

            @Override
            public void onSuccess(String sucessInfo) {
                // 此处回调仅代表用户已发起支付操作，不代表是否充值成功，具体充值是否到账需以服务器间通知为准；
                // 在此回调中游戏方可开始向游戏服务器发起请求，查看订单是否已支付成功，若支付成功则发送道具。
                showToast("已发起支付，请向游戏服务器查询充值是否到账");
                Dictionary data = new Dictionary();
                data.put("type", "prepare");
                data.put("result", true);
                handlerCallback("pay", data);
            }
            
            @Override
            public void onFail(String failInfo) {
                // 此处回调代表用户已放弃支付，无需向服务器查询充值状态
                showToast("用户已放弃支付");
                Dictionary data = new Dictionary();
                data.put("type", "prepare");
                data.put("result", false);
                handlerCallback("pay", data);
            }
        });
    }

    /**
     * 登出接口说明：
     * 
     * @param activity
     *            当前activity
     * @param customObject
     *            用户自定义参数，在登陆回调中原样返回
     */
    private void doLogout(Object p_object) {
        GameProxy.getInstance().logout(activity, p_object);
    }

    /**
     * 退出接口说明：
     * 
     * @param context
     *            当前activity
     * @param callback
     *            退出回调
     */
    private void doExit() {
        GameProxy.getInstance().exit(activity, new LJExitCallback() {

            @Override
            public void onGameExit() {
                // 渠道不存在退出界面，如百度移动游戏等，此时需在此处弹出游戏退出确认界面，否则会出现渠道审核不通过情况
                // 游戏定义自己的退出界面 ，实现退出逻辑
                AlertDialog.Builder builder = new Builder(activity);
                builder.setTitle("游戏自带退出界面");
                builder.setPositiveButton("退出",new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog,int which) {
                        dialog.dismiss();
                        // 该方法必须在退出时调用
                        GameProxy.getInstance().applicationDestroy(activity);

                        /**** 退出逻辑需确保能够完全销毁游戏 ****/
                        activity.finish();
                        onMainDestroy();
                        android.os.Process.killProcess(android.os.Process.myPid());
                        /**** 退出逻辑请根据游戏实际情况，勿照搬Demo ****/
                    }
                });
                builder.setNegativeButton("取消", new DialogInterface.OnClickListener() {
                    
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                    }
                });
                
                dialog = builder.create();
                dialog.show();
            }

            @Override
            public void onChannelExit() {
                // 渠道存在退出界面，如91、360等，此处只需进行退出逻辑即可，无需再弹游戏退出界面；
                Toast.makeText(activity, "由渠道退出界面退出",Toast.LENGTH_LONG).show();
                // 该方法必须在退出时调用
                GameProxy.getInstance().applicationDestroy(activity);

                /**** 退出逻辑需确保能够完全销毁游戏 ****/
                activity.finish();
                onMainDestroy();
                android.os.Process.killProcess(android.os.Process.myPid());
                /**** 退出逻辑请根据游戏实际情况，勿照搬Demo ****/

            }
        });
    }

    /**
     * 数据上传接口说明：(需在进入服务器、角色升级、创建角色处分别调用，否则无法上传apk)
     * 
     * @param activity
     *            上下文Activity，不要使用getApplication()
     * @param data
     *            上传数据
     *            _id 当前情景，目前支持 enterServer，levelUp，createRole
     *            游戏方需在进入服务器、角色升级、创建角色处分别调用 roleId 当前登录的玩家角色ID，必须为数字，若如，传入userid
     *            roleName 当前登录的玩家角色名，不能为空，不能为null，若无，传入"游戏名称+username"
     *            roleLevel 当前登录的玩家角色等级，必须为数字，若无，传入1 zoneId
     *            当前登录的游戏区服ID，必须为数字，若无，传入1 zoneName
     *            当前登录的游戏区服名称，不能为空，不能为null，若无，传入"无区服" balance
     *            当前用户游戏币余额，必须为数字，若无，传入0 vip 当前用户VIP等级，必须为数字，若无，传入1 partyName
     *            当前用户所属帮派，不能为空，不能为null，若无，传入"无帮派"
     */
    private void doSetExtData(Dictionary p_data) {
        String id = p_data.get("id").toString();
        String role_id = p_data.get("role_id").toString();
        String role_name = p_data.get("role_name").toString();
        String role_level = p_data.get("role_level").toString();
        String zone_id = p_data.get("zone_id").toString();
        String zone_name = p_data.get("zone_name").toString();
        String zone_level = p_data.get("zone_level").toString();
        String balance = p_data.get("balance").toString();
        String vip = p_data.get("vip").toString();
        String party_name = p_data.get("party_name").toString();

        Map<String, String> datas = new HashMap<String, String>();
        datas.put("_id", id);
        datas.put("roleId", role_id);
        datas.put("roleName", role_name);
        datas.put("roleLevel", role_level);
        datas.put("zoneId", zone_id);
        datas.put("zoneName", zone_name);
        datas.put("balance", balance);
        datas.put("vip", vip);
        datas.put("partyName", party_name);
        JSONObject obj = new JSONObject(datas);
        GameProxy.getInstance().setExtData(activity, obj.toString());
    }

    /**
     * 用于获取渠道标识，游戏开发者可在任意处调用该方法获取到该字段，含义请参照《如何区分渠道》中的渠道与ChannelLabel对照表
     * 
     * @return
     */
    public String getChannelLabel() {
        return XMUtils.getChannelLabel(activity);
    }


    @Override
    public void onLoginSuccess(final XMUser user, Object customParams) {
        showToast("登录成功");
        // save user object
        xmuser = user;
        // TODO:登陆成功之后需要调用用户信息扩展接口
        // doSetExtData();

        Dictionary data = new Dictionary();
        data.put("custom_param", customParams);
        data.put("result", true);
        data.put("user_name", user.getUsername());
        data.put("user_id", user.getUserID());
        data.put("token", user.getToken());
        data.put("channel_id", user.getChannelID());
        data.put("channel_user_id", user.getChannelUserId());
        data.put("channel_label", user.getChannelLabel());
        handlerCallback("login", data);
        // 登录成功后，进行登录信息校验，此步为必须完成操作，若不完成用户信息验证，我方平台拒绝提包
        //  TODO: gameserver auth
    }

    @Override
    public void onLoginFailed(String reason, Object customParams) {
        showToast("登录失败");
        Dictionary data = new Dictionary();
        data.put("custom_param", customParams);
        data.put("result", false);
        handlerCallback("login", data);
    }

    @Override
    public void onLogout(Object customParams) {
        // customObject为logout方法中传入的参数，原样返回
        xmuser = null;
        showToast("已成功登出");
        Dictionary data = new Dictionary();
        data.put("custom_param", customParams);
        data.put("result", true);
        handlerCallback("logout", data);
    }

    public Object request(String p_what, Object p_object) {

        // setup sdk handler(instance id) and callback function name
        //  and initialize sdk
        if(p_what.equals("init")) {
            if(sdkHandler == 0) {
                Dictionary p_data = (Dictionary) p_object;
                if(p_data == null)
                    return "INVALID_PARAM";

                try {
                    sdkHandler = Integer.parseInt(p_data.get("handler").toString());
                    sdkCallback = p_data.get("callback").toString();
                }
                catch(Exception e) {
                    return "INVALID_PARAM";
                }
                doInit();
            }
        }
        else if(p_what.equals("login")) {
            doLogin(p_object);
        }
        else if(p_what.equals("pay")) {
            Dictionary p_data = (Dictionary) p_object;
            if(p_data == null)
                return "INVALID_PARAM";

            try {
                doPay(p_data);
            }
            catch(Exception e) {
                return "INVALID_PARAM";
            }
        }
        else if(p_what.equals("logout")) {
            doLogout(p_object);
        }
        else if(p_what.equals("exit")) {
            doExit();
        }
        else if(p_what.equals("set_ext_data")) {
            Dictionary p_data = (Dictionary) p_object;
            if(p_data == null)
                return "INVALID_PARAM";

            try {
                doSetExtData(p_data);
            }
            catch(Exception e) {
                return "INVALID_PARAM";
            }
        }
        else if(p_what.equals("get_channel_label")) {
            return getChannelLabel();
        }
        return "OK";
    }

    public void showToast(final String content) {
        activity.runOnUiThread(new Runnable() {
            
            @Override
            public void run() {
                Toast.makeText(activity, content, Toast.LENGTH_LONG).show();               
            }
        });
    }

    // forwarded callbacks you can reimplement, as SDKs often need them
    protected void onMainActivityResult(int requestCode, int resultCode, Intent data) {
        GameProxy.getInstance().onActivityResult(activity, requestCode, resultCode, data);
    }
    protected void onMainPause() {
        GameProxy.getInstance().onPause(activity);
    }
    protected void onMainResume() {
        GameProxy.getInstance().onResume(activity);
    }
    protected void onMainDestroy() {
        GameProxy.getInstance().onDestroy(activity);
    }
    protected void onMainStart() {
        GameProxy.getInstance().onStart(activity);
    }
    protected void onMainStop() {
        GameProxy.getInstance().onStop(activity);
    }
    protected void onMainRestart() {
        GameProxy.getInstance().onRestart(activity);
    }
    protected void onMainNewIntent(Intent intent) {
        GameProxy.getInstance().onNewIntent(intent);
    }

    protected void onGLDrawFrame(GL10 gl) {}
    // singletons will always miss first onGLSurfaceChanged call
    protected void onGLSurfaceChanged(GL10 gl, int width, int height) {}
}
