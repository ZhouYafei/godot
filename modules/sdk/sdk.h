/*************************************************************************/
/*  sdk.h                                                                */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
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
#ifdef MODULE_SDK_ENABLED
#ifndef SDK_H
#define SDK_H

#include "core/object.h"

static const char *CALLBACK_INIT = "init";
static const char *CALLBACK_LOGIN = "login";
static const char *CALLBACK_SWITCH_LOGIN = "switch_login";
static const char *CALLBACK_LOGOUT = "logout";
static const char *CALLBACK_PAY = "pay";
static const char *CALLBACK_RESULT = "result";

enum U8Codes {
	CODE_NO_NETWORK = 0,			// 没有网络连接
	CODE_INIT_SUCCESS = 1,			// 初始化成功
	CODE_INIT_FAIL = 2,				// 初始化失败
	CODE_UNINIT = 3,				// 没有初始化
	CODE_LOGIN_SUCCESS = 4,			// 登录成功
	CODE_LOGIN_FAIL = 5,			// 登录失败
	CODE_LOGIN_TIMEOUT = 6,			// 登录超时
	CODE_UNLOGIN = 7,				// 没有登录
	CODE_LOGOUT_SUCCESS = 8,		// 登出成功
	CODE_LOGOUT_FAIL = 9,			// 登出失败
	CODE_PAY_SUCCESS = 10,			// 支付成功
	CODE_PAY_FAIL = 11,				// 支付失败
	CODE_TAG_ADD_SUC = 12,			// 添加Tag成功
	CODE_TAG_ADD_FAIL = 13,			// 添加Tag失败
	CODE_TAG_DEL_SUC = 14,			// 删除Tag成功
	CODE_TAG_DEL_FAIL = 15,			// 删除Tag失败
	CODE_ALIAS_ADD_SUC = 16,		// 添加Alias成功
	CODE_ALIAS_ADD_FAIL = 17,		// 添加Alias失败
	CODE_ALIAS_REMOVE_SUC = 18,		// 删除Alias成功
	CODE_ALIAS_REMOVE_FAIL = 19,	// 删除Alias失败
	CODE_PUSH_MSG_RECIEVED = 20,	// Push 收到msg
	CODE_PARAM_ERROR = 21,			// 参数 错误
	CODE_PARAM_NOT_COMPLETE = 22,	// 参数不全
	CODE_SHARE_SUCCESS = 23,		// 分享成功
	CODE_SHARE_FAILED = 24,			// 分享失败
	CODE_UPDATE_SUCCESS = 25,		// 更新成功
	CODE_UPDATE_FAILED = 26,		// 更新失败
	CODE_REAL_NAME_REG_SUC = 27,	// 实名注册成功
	CODE_REAL_NAME_REG_FAILED = 28,	// 实名注册失败
	CODE_ADDICTION_ANTI_RESULT = 29,// 房沉迷查询结果
	CODE_PUSH_ENABLED = 30,			// 推送enable成功的回调，携带一个参数，比如友盟推送，这参数是Device Token
	CODE_POST_GIFT_SUC = 31,		// 提交礼包兑换码成功
	CODE_POST_GIFT_FAILED = 32,		// 提交礼包兑换码失败
	CODE_PAY_CANCEL = 33,			// 取消支付
	CODE_PAY_UNKNOWN = 34,			// 支付未知
	CODE_PAYING = 35,				// 支付中
};

class Sdk : public Object {

	OBJ_TYPE(Sdk, Object);

	int handler;
	String callback;

	static Sdk* instance;

protected:
	virtual void on_result(int p_code, const String& p_msg);

	void sendCallback(const String p_what, const Dictionary& p_data = Dictionary());

	static void _bind_methods();

public:
	virtual void init(int p_handler, const String& p_callback);
	virtual bool is_support(const String& p_plugin, const String& p_what);
	virtual int get_curr_channel();
	virtual int get_logic_channel();
	virtual int get_app_id();
	virtual String get_app_key();
	virtual String get_app_signature();
	virtual void tip(const String& p_tip);
	virtual String get_network_type();
	virtual void login();
	virtual void login_custom(const String& p_extension);
	virtual void switch_login();
	virtual void logout();
	virtual void show_user_center();
	virtual void submit_extra(const Dictionary& p_data);
	virtual void exit();
	virtual void pay(const Dictionary& p_data);
	virtual void start_push();
	virtual void stop_push();
	virtual void add_tags(const StringArray& p_tags);
	virtual void remove_tags(const StringArray& p_tags);
	virtual void add_alias(const String& p_alias);
	virtual void remove_alias(const String& p_alias);
	virtual void share(const Dictionary& p_data);
	virtual void analytics(const Dictionary& p_data);
	virtual void download(const String& p_url, bool p_show_confirm, bool p_force);
	virtual String get_clipboard();
	virtual void set_clipboard(const String& p_text);
	virtual String get_install_params();
	virtual String get_agent_id();
	virtual String get_agent_channel();

	static Sdk* get_singleton();

	Sdk();
	virtual ~Sdk();
};

#endif // SDK_H
#endif // MODULE_SDK_ENABLED
