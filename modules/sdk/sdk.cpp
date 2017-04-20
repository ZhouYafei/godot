/*************************************************************************/
/*  sdk.cpp                                                              */
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

#include "sdk.h"
#include "os/os.h"

static const char *U8CodeStrings[] = {
	"no_network",
	"init_success",
	"init_fail",
	"uninit",
	"login_success",
	"login_fail",
	"login_timeout",
	"unlogin",
	"logout_success",
	"logout_fail",
	"pay_success",
	"pay_fail",
	"tag_add_suc",
	"tag_add_fail",
	"tag_del_suc",
	"tag_del_fail",
	"alias_add_suc",
	"alias_add_fail",
	"alias_remove_suc",
	"alias_remove_fail",
	"push_msg_recieved",
	"param_error",
	"param_not_complete",
	"share_success",
	"share_failed",
	"update_success",
	"upate_failed",
	"real_name_reg_suc",
	"real_name_reg_failed",
	"addiction_anti_result",
	"push_enabled",
	"post_gift_suc",
	"post_gift_failed",
	"pay_cancel",
	"pay_unknown",
	"paying",
};

Sdk *Sdk::instance = NULL;

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

void Sdk::on_result(int p_code, const String& p_msg) {

	Dictionary d;
	d["code"] = p_code;
	d["what"] = (p_code >= ARRAY_SIZE(U8CodeStrings)) ? ("unknown code : " + String::num(p_code)) : U8CodeStrings[p_code];
	d["msg"] = p_msg;

	switch(p_code) {
	case CODE_INIT_SUCCESS:
		sendCallback(CALLBACK_INIT, d);
		return;
	case CODE_INIT_FAIL:
		tip("SDK init failure");
		break;
	case CODE_LOGIN_FAIL:
		// 这里不需要提示，一般SDK有提示
		// sdk.tip("SDK登录失败");
		break;
   	case CODE_SHARE_SUCCESS:
    	tip("share success");
    	break;
    case CODE_SHARE_FAILED:
    	tip("share failed");
    	break;
	case CODE_PAY_FAIL:
		tip("pay failure");
		break;
	case CODE_PAY_SUCCESS:
		// 一般这里不用提示
		// context.tip("支付成功,到账时间可能稍有延迟");
 		break;
	}
	sendCallback(CALLBACK_RESULT, d);
}

void Sdk::sendCallback(const String p_what, const Dictionary& p_data) {

	Object *obj = ObjectDB::get_instance(handler);
	if(obj == NULL)
		return;
	obj->call(callback, p_what, p_data);
}

void Sdk::init(int p_handler, const String& p_callback) {

	this->handler = p_handler;
	this->callback = p_callback;

	on_result(CODE_INIT_SUCCESS, "SDK initialize success");
}

bool Sdk::is_support(const String& p_plugin, const String& p_what) {

	if(p_plugin == "user") {
		if(p_what == "exit")
			return false;
	}
	return false;
}

int Sdk::get_curr_channel() {
	return 0;
}

int Sdk::get_logic_channel() {
	return 0;
}

int Sdk::get_app_id() {
	return -1;
}

String Sdk::get_app_key() {
	return "";
}

String Sdk::get_app_signature() {
	return "";
}

void Sdk::tip(const String& p_tip) {

	printf("SDK: tip '%s'\n", p_tip.utf8().get_data());
}

String Sdk::get_network_type() {

	return "wifi";
}

void Sdk::login() {

	login_custom("");
}

void Sdk::login_custom(const String& p_extension) {

	Dictionary d;
	d["success"] = true;
	d["switch_account"] = false;
	d["extension"] = p_extension;
	d["user_id"] = -1;
	d["user_name"] = "";
	d["sdk_user_id"] = "";
	d["sdk_user_name"] = "";
	d["token"] = "";
	on_result(CODE_LOGIN_SUCCESS, "login success");
	sendCallback(CALLBACK_SWITCH_LOGIN);
	sendCallback(CALLBACK_LOGIN, d);
}

void Sdk::switch_login() {

	Dictionary d;
	d["success"] = true;
	d["switch_account"] = true;
	d["extension"] = "";
	d["user_id"] = -1;
	d["user_name"] = "";
	d["sdk_user_id"] = "";
	d["sdk_user_name"] = "";
	d["token"] = "";
	on_result(CODE_LOGIN_SUCCESS, "login success");
	sendCallback(CALLBACK_LOGIN, d);
}

void Sdk::logout() {

	on_result(CODE_LOGOUT_SUCCESS, "logout success");
	sendCallback(CALLBACK_LOGOUT);
}

void Sdk::show_user_center() {
}

void Sdk::submit_extra(const Dictionary& p_data) {

	p_data;
}

void Sdk::exit() {
}

void Sdk::pay(const Dictionary& p_data) {

	p_data;
}

void Sdk::start_push() {
}

void Sdk::stop_push() {
}

void Sdk::add_tags(const StringArray& p_tags) {

	p_tags;
}

void Sdk::remove_tags(const StringArray& p_tags) {

	p_tags;
}

void Sdk::add_alias(const String& p_alias) {

	p_alias;
}

void Sdk::remove_alias(const String& p_alias) {

	p_alias;
}

void Sdk::share(const Dictionary& p_data) {

	p_data;

	on_result(CODE_SHARE_SUCCESS, "share success");
}

void Sdk::analytics(const Dictionary& p_data) {

	p_data;
}

void Sdk::download(const String& p_url, bool p_show_confirm, bool p_force) {

	p_url; p_show_confirm; p_force;
}

String Sdk::get_clipboard() {

	return OS::get_singleton()->get_clipboard();
}

void Sdk::set_clipboard(const String& p_text) {

	OS::get_singleton()->set_clipboard(p_text);
}

String Sdk::get_install_params() {
	return "{}";
}

void Sdk::_bind_methods() {

 	ObjectTypeDB::bind_method(_MD("init", "inst_id", "callback"),&Sdk::init);
 	ObjectTypeDB::bind_method(_MD("is_support", "plugin_name", "what"),&Sdk::is_support);
 	ObjectTypeDB::bind_method(_MD("get_curr_channel"),&Sdk::get_curr_channel);
 	ObjectTypeDB::bind_method(_MD("get_logic_channel"),&Sdk::get_logic_channel);
 	ObjectTypeDB::bind_method(_MD("get_app_id"),&Sdk::get_app_id);
 	ObjectTypeDB::bind_method(_MD("get_app_key"),&Sdk::get_app_key);
 	ObjectTypeDB::bind_method(_MD("get_app_signature"),&Sdk::get_app_signature);
 	ObjectTypeDB::bind_method(_MD("tip", "tip"),&Sdk::tip);
 	ObjectTypeDB::bind_method(_MD("get_network_type"),&Sdk::get_network_type);
 	ObjectTypeDB::bind_method(_MD("login"),&Sdk::login);
 	ObjectTypeDB::bind_method(_MD("login_custom", "extension"),&Sdk::login_custom);
 	ObjectTypeDB::bind_method(_MD("switch_login"),&Sdk::switch_login);
 	ObjectTypeDB::bind_method(_MD("logout"),&Sdk::logout);
 	ObjectTypeDB::bind_method(_MD("show_user_center"),&Sdk::show_user_center);
 	ObjectTypeDB::bind_method(_MD("submit_extra", "extra_data"),&Sdk::submit_extra);
 	ObjectTypeDB::bind_method(_MD("exit"),&Sdk::exit);
 	ObjectTypeDB::bind_method(_MD("pay", "pay_params"),&Sdk::pay);
 	ObjectTypeDB::bind_method(_MD("start_push"),&Sdk::start_push);
 	ObjectTypeDB::bind_method(_MD("stop_push"),&Sdk::stop_push);
 	ObjectTypeDB::bind_method(_MD("add_tags", "tags"),&Sdk::add_tags);
 	ObjectTypeDB::bind_method(_MD("remove_tags", "tags"),&Sdk::remove_tags);
 	ObjectTypeDB::bind_method(_MD("add_alias", "alias"),&Sdk::add_alias);
 	ObjectTypeDB::bind_method(_MD("remove_alias", "alias"),&Sdk::remove_alias);
 	ObjectTypeDB::bind_method(_MD("share", "share_params"),&Sdk::share);
 	ObjectTypeDB::bind_method(_MD("analytics", "params"),&Sdk::analytics);
 	ObjectTypeDB::bind_method(_MD("download", "url", "show_confirm", "force"),&Sdk::download);
 	ObjectTypeDB::bind_method(_MD("get_clipboard"),&Sdk::get_clipboard);
 	ObjectTypeDB::bind_method(_MD("set_clipboard", "text"),&Sdk::set_clipboard);
	ObjectTypeDB::bind_method(_MD("get_install_params", "text"),&Sdk::get_install_params);
}

Sdk *Sdk::get_singleton() {

	return instance;
}

Sdk::Sdk() {

	instance = this;
}

Sdk::~Sdk() {

	instance = NULL;
}

#endif // MODULE_SDK_ENABLED
