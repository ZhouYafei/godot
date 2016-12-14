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

class Sdk : public Object {

	OBJ_TYPE(Sdk, Object);

	int handler;
	String callback;

	static Sdk* instance;

	virtual void on_result(int p_code, const String& p_msg);

protected:
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

	static Sdk* get_singleton();

	Sdk();
	virtual ~Sdk();
};

#endif // SDK_H
#endif // MODULE_SDK_ENABLED
