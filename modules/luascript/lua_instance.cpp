/*************************************************************************/
/*  lua_script.cpp                                                       */
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
#ifdef LUASCRIPT_ENABLED

#include <stdlib.h>
#include <string.h>
#include "lua_script.h"
#include "globals.h"
#include "global_constants.h"
//#include "gd_compiler.h"
#include "os/file_access.h"

//////////////////////////////
//		 INSTANCE		 //
//////////////////////////////


static Object *luaL_checkobject(lua_State *L, int idx) {

	size_t id = *(size_t *) luaL_checkudata(L, idx, "LuaObject");
	return ObjectDB::get_instance(id);
}

bool LuaInstance::set(const StringName& p_name, const Variant& p_value) {

	LUA_MULTITHREAD_GUARD();
	LuaScript *sptr=script.ptr();

	Variant v_name = p_name;
	const Variant *args[2]={&v_name, &p_value };

	while(sptr) {
		//member
		if(sptr->member_info.has(p_name)) {

			CharString name = ((String) p_name).utf8();
			// get instance's lua table field
			lua_State *L = LuaScriptLanguage::get_singleton()->get_state();
			if(l_get_object_table()) {

				lua_pushstring(L, name.get_data());
				l_push_variant(L, p_value);
				lua_rawset(L, -3);
				lua_pop(L, 1);
				return true;
			}
		}

		Variant ret;
		if(_call_script_func(sptr, this, "_set", args, 2, ret) == OK) {

			if(ret.get_type() == Variant::BOOL && ret.operator bool())
				return true;
		}
		sptr = sptr->_base;
	}

	return false;
}

bool LuaInstance::get(const StringName& p_name, Variant &r_ret) const {

	LUA_MULTITHREAD_GUARD();
	lua_State *L = LuaScriptLanguage::get_singleton()->get_state();
	CharString name = ((String) p_name).utf8();

	Variant v_name = p_name;
	const Variant *args[1]={&v_name };

	const LuaScript *sptr=script.ptr();
	while(sptr) {
		if(sptr->member_info.has(p_name)) {

			// get instance's lua table field
			if(l_get_object_table()) {

				lua_pushstring(L, name.get_data());
				lua_rawget(L, -2);
				if(!lua_isnil(L, -1)) {

					l_get_variant(L, -1, r_ret);
					lua_pop(L, 2);
					return true;
				}
				lua_pop(L, 2);
			}
		}

		// call script's '_get' method
		Variant ret;
		if(_call_script_func(sptr, this, "_get", args, 1, ret) == OK) {

			if(ret.get_type() != Variant::NIL) {

				r_ret=ret;
				return true;
			}
		}
		sptr = sptr->_base;
	}
	return false;

}

void LuaInstance::get_property_list(List<PropertyInfo> *p_properties) const {
	// exported members, not doen yet!
	const LuaScript *sptr=script.ptr();
	List<PropertyInfo> props;

	while(sptr) {

		// call script's '_get' method
		Variant ret;
		if(_call_script_func(sptr, this, "_get_property_list", NULL, 0, ret) == OK) {

			if(ret.get_type() != Variant::ARRAY) {

				ERR_EXPLAIN("Wrong type for _get_property list, must be an array of dictionaries.");
				ERR_FAIL();
			}
			Array arr = ret;
			for(int i=0;i<arr.size();i++) {

				Dictionary d = arr[i];
				ERR_CONTINUE(!d.has("name"));
				ERR_CONTINUE(!d.has("type"));
				PropertyInfo pinfo;
				pinfo.type = Variant::Type( d["type"].operator int());
				ERR_CONTINUE(pinfo.type<0 || pinfo.type>=Variant::VARIANT_MAX );
				pinfo.name = d["name"];
				ERR_CONTINUE(pinfo.name=="");
				if (d.has("hint"))
					pinfo.hint=PropertyHint(d["hint"].operator int());
				if (d.has("hint_string"))
					pinfo.hint_string=d["hint_string"];
				if (d.has("usage"))
					pinfo.usage=d["usage"];

				props.push_back(pinfo);
			}
		}

		for(Map<StringName,PropertyInfo>::Element *E=sptr->member_info.front();E;E=E->next()) {
			props.push_front(E->get());
		}
		sptr = sptr->_base;
	}

	props.invert();

	for (List<PropertyInfo>::Element *E=props.front();E;E=E->next()) {
		p_properties->push_back(E->get());
	}
}

void LuaInstance::get_method_list(List<MethodInfo> *p_list) const {

//	const LuaScript *sptr=script.ptr();
//	while(sptr) {
//
//		for (Map<StringName,GDFunction>::Element *E = sptr->member_functions.front();E;E=E->next()) {
//
//			MethodInfo mi;
//			mi.name=E->key();
//			for(int i=0;i<E->get().get_argument_count();i++)
//				mi.arguments.push_back(PropertyInfo(Variant::NIL,"arg"+itos(i)));
//			p_list->push_back(mi);
//		}
//		sptr = sptr->_base;
//	}

}

bool LuaInstance::has_method(const StringName& p_method) const {

//	const LuaScript *sptr=script.ptr();
//	while(sptr) {
//		const Map<StringName,GDFunction>::Element *E = sptr->member_functions.find(p_method);
//		if (E)
//			return true;
//		sptr = sptr->_base;
//	}
//
	return false;
}

int LuaInstance::_call_script(const LuaScript *sptr, const LuaInstance *inst, const char *p_method, const Variant** p_args, int p_argcount, bool p_ret) const
{
	LUA_MULTITHREAD_GUARD();

	lua_State *L = LuaScriptLanguage::get_singleton()->get_state();

	if(l_get_object_table()) {

		lua_pushstring(L, p_method);
		lua_rawget(L, -2);
		if(lua_isnil(L, -1)) {

			lua_pop(L, 2);
			lua_rawgeti(L, LUA_REGISTRYINDEX, sptr->ref);
			lua_pushstring(L, p_method);
			lua_rawget(L, -2);
			lua_remove(L, -2);
		}
	}

	if(lua_isfunction(L, -1)) {

		l_push_variant(L, inst->owner);
		for(int idx = 0; idx < p_argcount; idx++)
			l_push_variant(L, *p_args[idx]);
		//lua_pushcfunction(L, LuaScriptLanguage::panic);
		//lua_insert(L, -(p_argcount + 3));
		if(lua_pcall(L, p_argcount + 1, p_ret ? 1 : 0, 0/*-(p_argcount + 3)*/)) {

			const char *err = lua_tostring(L, -1);
			script->reportError("Call Error: %s, Function: %s", err, p_method);
			return ERR_SCRIPT_FAILED;
		}
		return OK;
	}
	lua_pop(L, 1);
	return ERR_SKIP;
}

int LuaInstance::_call_script_func(const LuaScript *sptr,const  LuaInstance *inst, const char *p_method, const Variant** p_args, int p_argcount) const
{
	LUA_MULTITHREAD_GUARD();

	lua_State *L = LuaScriptLanguage::get_singleton()->get_state();

	int top = lua_gettop(L);
	int ret = _call_script(sptr, inst, p_method, p_args, p_argcount, false);
	lua_settop(L, top);
	return ret;
}

int LuaInstance::_call_script_func(const LuaScript *sptr,const  LuaInstance *inst, const char *p_method, const Variant** p_args, int p_argcount, Variant& result) const
{
	LUA_MULTITHREAD_GUARD();

	lua_State *L = LuaScriptLanguage::get_singleton()->get_state();

	int top = lua_gettop(L);
	int ret = _call_script(sptr, inst, p_method, p_args, p_argcount, true);
	if(ret == OK) {

		lua_State *L = LuaScriptLanguage::get_singleton()->get_state();
		l_get_variant(L, -1, result);
	}
	lua_settop(L, top);
	return ret;
}

Variant LuaInstance::call(const StringName& p_method,const Variant** p_args,int p_argcount,Variant::CallError &r_error) {
	//printf("calling %ls:%i method %ls\n", script->get_path().c_str(), -1, String(p_method).c_str());
	LuaScript *sptr=script.ptr();

	Variant result;
	while(sptr) {
		switch(_call_script_func(sptr, this, ((String) p_method).utf8().get_data(), p_args, p_argcount, result)) {
		case OK:
			r_error.error=Variant::CallError::CALL_OK;
			return result;
		case FAILED:
			r_error.error=Variant::CallError::CALL_ERROR_INVALID_METHOD;
			return Variant();
		case ERR_SKIP:
			break;
		}
		sptr = sptr->_base;
	}
	r_error.error=Variant::CallError::CALL_ERROR_INVALID_METHOD;
	return Variant();
}

void LuaInstance::call_multilevel(const StringName& p_method,const Variant** p_args,int p_argcount) {

	LuaScript *sptr=script.ptr();
	Variant::CallError ce;

	while(sptr) {
		_call_script_func(sptr, this, ((String) p_method).utf8().get_data(), p_args, p_argcount);
		sptr = sptr->_base;
	}
}


void LuaInstance::_ml_call_reversed(LuaScript *sptr,const StringName& p_method,const Variant** p_args,int p_argcount) {
	if (sptr->_base)
		_ml_call_reversed(sptr->_base,p_method,p_args,p_argcount);

	_call_script_func(sptr, this, ((String) p_method).utf8().get_data(), p_args, p_argcount);
}

void LuaInstance::call_multilevel_reversed(const StringName& p_method,const Variant** p_args,int p_argcount) {

	if (script.ptr()) {
		_ml_call_reversed(script.ptr(),p_method,p_args,p_argcount);
	}
}

void LuaInstance::notification(int p_notification) {

	//notification is not virutal, it gets called at ALL levels just like in C.
	Variant value=p_notification;
	const Variant *args[1]={&value };

	LuaScript *sptr=script.ptr();
	while(sptr) {
		_call_script_func(sptr, this, "_notification", args, 1);
		sptr = sptr->_base;
	}
}

Ref<Script> LuaInstance::get_script() const {

	return script;
}

ScriptLanguage *LuaInstance::get_language() {

	return LuaScriptLanguage::get_singleton();
}

int LuaInstance::l_extends(lua_State *L)
{
	LUA_MULTITHREAD_GUARD();

	// self -> LuaObject
	Object *obj = luaL_checkobject(L, 1);
	const char *type = luaL_checkstring(L, 2);

	if(obj->get_type() == type)
		lua_pushboolean(L, 1);
	else if(ObjectTypeDB::is_type(obj->get_type_name(), type))
		lua_pushboolean(L, 1);
	else if(obj->get_script_instance() && obj->get_script_instance()->get_language() == LuaScriptLanguage::get_singleton()) {

		LuaInstance *ins = static_cast<LuaInstance*>(obj->get_script_instance());
		LuaScript *cmp = ins->script.ptr();
		bool found=false;
		while(cmp) {
			// path check
			//  res://xx.lua euqal to res:://xx.luac
			if(cmp->path.find(type) == 0)
			{
				found = true;
				break;
			}
			cmp=cmp->_base;
		}
		lua_pushboolean(L, found ? 1 : 0);
	}
	else
		lua_pushboolean(L, 0);

	return 1;
}

int LuaInstance::l_methodbind_wrapper(lua_State *L)
{
	LUA_MULTITHREAD_GUARD();

	MethodBind *mb = (MethodBind *) lua_touserdata(L, lua_upvalueindex(1));
	// self -> LuaObject
	Object *obj = luaL_checkobject(L, 1);

	Variant ret;
	Variant::CallError err;

	int top = lua_gettop(L);
	if(top >= 2) {

		Variant *vars = memnew_arr(Variant, top - 1);
		Variant *args[128];
		for(int idx = 2; idx <= top; idx++) {

			Variant& var = vars[idx - 2];
			args[idx - 2] = &var;
			l_get_variant(L, idx, var);
		}
		ret = mb->call(obj, (const Variant **) args, top - 1, err);
		memdelete_arr(vars);
	} else {

		ret = mb->call(obj, NULL, 0, err);
	}
	switch(err.error) {
	case Variant::CallError::CALL_OK:
		l_push_variant(L, ret);
		return 1;
	case Variant::CallError::CALL_ERROR_INVALID_METHOD:
		luaL_error(L, "Invalid method");
		break;
	case Variant::CallError::CALL_ERROR_INVALID_ARGUMENT:
		luaL_error(L, "Invalid arguments");
		break;
	case Variant::CallError::CALL_ERROR_TOO_MANY_ARGUMENTS:
		luaL_error(L, "Too many arguments");
		break;
	case Variant::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS:
		luaL_error(L, "Too few arguments");
		break;
	case Variant::CallError::CALL_ERROR_INSTANCE_IS_NULL:
		luaL_error(L, "Instance is null");
		break;
	}
	return 0;
}

int LuaInstance::meta__gc(lua_State *L)
{
	LUA_MULTITHREAD_GUARD();

	Object *obj = (Object *) luaL_checkobject(L, 1);
	if(obj == NULL)
		return 0;

	//LuaInstance *inst = dynamic_cast<LuaInstance *>(obj->get_script_instance());
	//if(inst != NULL && !inst->gc_delete)
	//	obj->set_script_instance(NULL);

	// dec refcount if obj is resource
	if(obj->is_type_ptr(Reference::get_type_ptr_static())) {
		Reference *ref = obj->cast_to<Reference>();
		if(ref->unreference())
			memdelete(ref);
	}
	lua_pushnil(L);
	lua_setmetatable(L, 1);
	return 1;
}

int LuaInstance::meta__tostring(lua_State *L)
{
	LUA_MULTITHREAD_GUARD();

	// self -> LuaObject
	Object *obj = luaL_checkobject(L, 1);

	char buf[128];
	sprintf(buf, "%s: 0x%p", obj->get_type().utf8().get_data(), obj);
	lua_pushstring(L, buf);

	return 1;
}

int LuaInstance::meta__index(lua_State *L)
{
	LUA_MULTITHREAD_GUARD();

	// self -> LuaObject
	Object *obj = luaL_checkobject(L, 1);
	LuaInstance *inst = dynamic_cast<LuaInstance *>(obj->get_script_instance());
	// get symbol from lua instance table
	if(inst != NULL && inst->l_get_object_table()) {

		lua_pushvalue(L, 2);
		lua_rawget(L, -2);
		if(!lua_isnil(L, -1)) {

			lua_insert(L, -3);
			lua_pop(L, 2);
			return 1;
		}
		lua_pop(L, 2);
	}

	// get symbol from c++ method binds
	lua_getmetatable(L, 1);
	lua_getfield(L, -1, ".methods");
	lua_pushvalue(L, 2);
	lua_gettable(L, -2);
	if(!lua_isnil(L, -1)) {

		lua_insert(L, -3);
		lua_pop(L, 2);
		return 1;
	}
	lua_pop(L, 3);

	// get symbol from script proto

	// 1. if self is LuaScript, try get value from self->ref[key]
	LuaScript *sptr = obj->cast_to<LuaScript>();
	if(sptr != NULL) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, sptr->ref);
		lua_pushvalue(L, 2);
		lua_rawget(L, -2);
		if(!lua_isnil(L, -1)) {

			lua_insert(L, -3);
			lua_pop(L, 2);
			return 1;
		}
		lua_pop(L, 2);
	}
	// 2. get value from self->get_script()->ref[key]
	if(inst != NULL) {
		LuaScript *sptr = inst->script.ptr();
		while(sptr != NULL) {

			lua_rawgeti(L, LUA_REGISTRYINDEX, sptr->ref);
			lua_pushvalue(L, 2);
			lua_rawget(L, -2);
			if(!lua_isnil(L, -1)) {

				lua_insert(L, -3);
				lua_pop(L, 2);
				return 1;
			}
			lua_pop(L, 2);
			// inherit(extends) script
			sptr = sptr->_base;
		}
	}
	const char *name = lua_tostring(L, 2);
	// get symbol from c++
	if(name == NULL)
		return 0;

	// get object's property
	bool success = false;
	Variant var = obj->get(name, &success);
	if(success) {

		l_push_variant(L, var);
		return 1;
	}
	// get class constant
	success = false;
	int constant = ObjectTypeDB::get_integer_constant(obj->get_type_name(), name, &success);
	if(success) {

		lua_pushinteger(L, constant);
		return 1;
	}
	// get method bind
	MethodBind *mb = ObjectTypeDB::get_method(obj->get_type_name(), name);
	if(mb != NULL) {

		lua_pushlightuserdata(L, mb);
		lua_pushcclosure(L, l_methodbind_wrapper, 1);

		if(inst && inst->l_get_object_table()) {

			lua_pushvalue(L, 2);
			lua_pushvalue(L, -3);
			lua_rawset(L, -3);
			lua_pop(L, 1);
		}
		return 1;
	}
	return 0;
}

int LuaInstance::meta__newindex(lua_State *L)
{
	LUA_MULTITHREAD_GUARD();

	// self -> LuaObject
	Object *obj = luaL_checkobject(L, 1);
	ScriptInstance *sci = obj->get_script_instance();
	if(sci != NULL) {

		LuaInstance *inst = dynamic_cast<LuaInstance *>(sci);
		if(inst != NULL && inst->l_get_object_table()) {

			lua_pushvalue(L, 2);
			lua_pushvalue(L, 3);
			lua_rawset(L, -3);
			lua_pop(L, 1);
		}
	}

	return 0;
}

void LuaInstance::setup()
{
	LUA_MULTITHREAD_GUARD();

	lua_State *L = LuaScriptLanguage::get_singleton()->get_state();
	luaL_newmetatable(L, "LuaObject"); 
	{
		static luaL_Reg meta_methods[] = {
			{ "__gc", meta__gc },
			{ "__index", meta__index },
			{ "__newindex", meta__newindex },
			{ "__tostring", meta__tostring },
			{ NULL, NULL },
		};
		luaL_setfuncs(L, meta_methods, 0);

		lua_newtable(L);
		static luaL_Reg methods[] = {
			{ "extends", l_extends },
			{ NULL, NULL },
		};
		luaL_setfuncs(L, methods, 0);
		lua_setfield(L, -2, ".methods");
	}
	lua_pop(L, 1);

	luaL_newmetatable(L, "LuaVariant");
	{
		typedef struct {
			const char *meta;
			Variant::Operator op;
		} eval;

		static eval evaluates[] = {
			{ "__eq", Variant::OP_EQUAL },
			{ "__add", Variant::OP_ADD },
			{ "__sub", Variant::OP_SUBSTRACT },
			{ "__mul", Variant::OP_MULTIPLY },
			{ "__div", Variant::OP_DIVIDE },
			{ "__mod", Variant::OP_MODULE },
			{ "__lt", Variant::OP_LESS },
			{ "__le", Variant::OP_LESS_EQUAL },
		};

		for(int idx = 0; idx < sizeof(evaluates) / sizeof(evaluates[0]); idx++) {

			eval& ev = evaluates[idx];
			lua_pushstring(L, ev.meta);
			lua_pushinteger(L, ev.op);
			lua_pushcclosure(L, meta_bultins__evaluate, 1);
			lua_rawset(L, -3);
		}

		static luaL_Reg meta_methods[] = {
			{ "__gc", meta_bultins__gc },
			{ "__index", meta_bultins__index },
			{ "__newindex", meta_bultins__newindex },
			{ "__tostring", meta_bultins__tostring },
			{ "__pairs", meta_bultins__pairs },
			{ NULL, NULL },
		};
		luaL_setfuncs(L, meta_methods, 0);

		lua_newtable(L);
		static luaL_Reg methods[] = {
			{ NULL, NULL },
		};
		luaL_setfuncs(L, methods, 0);
		lua_setfield(L, -2, ".methods");
	}
	lua_pop(L, 1);

	/* create object ptr -> udata mapping table */
	lua_pushstring(L, "gdlua_ubox");
	lua_newtable(L);
	/* make weak value metatable for ubox table to allow userdata to be
	   garbage-collected */
	lua_newtable(L);
	lua_pushliteral(L, "__mode");
	lua_pushliteral(L, "v");
	lua_rawset(L, -3);			   /* stack: string ubox mt */
	lua_setmetatable(L, -2);  /* stack: string ubox */
	lua_rawset(L, LUA_REGISTRYINDEX);
}

bool LuaInstance::l_get_object_table() const
{
	LUA_MULTITHREAD_GUARD();
	lua_State *L = LuaScriptLanguage::get_singleton()->get_state();

	Object *obj = owner;

	lua_pushstring(L, "gdlua_ubox");
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushlightuserdata(L, obj);
	lua_rawget(L, -2);
	if(lua_isuserdata(L, -1)) {

		lua_getfenv(L, -1);
		if(lua_istable(L, -1)) {

			lua_insert(L, -3);
			lua_pop(L, 2);
			return true;
		}
		lua_pop(L, 1);
	}
	lua_pop(L, 2);
	return false;
}

int LuaInstance::init(bool p_ref)
{
	LUA_MULTITHREAD_GUARD();
	lua_State *L = LuaScriptLanguage::get_singleton()->get_state();

	Object *obj = owner;
	lua_pushstring(L, "gdlua_ubox");
	lua_rawget(L, LUA_REGISTRYINDEX);
	{
		// key
		lua_pushlightuserdata(L, obj);
		// value
		size_t *ptr = (size_t *) lua_newuserdata(L, sizeof(size_t));
		// add refcount if obj(self) is resource
		if(obj->is_type_ptr(Reference::get_type_ptr_static())) {
			Reference *ref = obj->cast_to<Reference>();
			ref->reference();
		}
		*ptr = owner->get_instance_ID();
		luaL_getmetatable(L, "LuaObject");
		lua_setmetatable(L, -2);
		// object's env table
		lua_newtable(L);
		{
			lua_pushvalue(L, -2);
			lua_setfield(L, -2, ".c_instance");
		}
		lua_setfenv(L, -2);
		if(p_ref) {
			lua_pushvalue(L, -1);
			ref = luaL_ref(L, LUA_REGISTRYINDEX);
		}
	}
	lua_rawset(L, -3);
	lua_pop(L, 1);

	if(script.ptr() != NULL)
	{
		if(_call_script_func(script.ptr(), this, "_init", NULL, 0) == FAILED)
			return FAILED;
	}
	return OK;
}

LuaInstance::LuaInstance() {
	owner=NULL;
	base_ref=false;
	gc_delete=false;
	ref=LUA_NOREF;
}

LuaInstance::~LuaInstance() {
	LUA_MULTITHREAD_GUARD();

	if (script.is_valid() && owner) {
		script->instances.erase(owner);
	}
	if(lang != NULL) {
		lua_State *L = lang->get_state();
		if(l_get_object_table()) {

			// get c instance
			lua_getfield(L, -1, ".c_instance");

			Object *self = luaL_checkobject(L, -1);
			if(self->is_type_ptr(Reference::get_type_ptr_static())) {
				Reference *ref = self->cast_to<Reference>();
				if(ref->unreference())
					memdelete(ref);
			} else {
				// delete userdata Variant
				//memdelete(self);
			}
			// set object's metatable to nil
			lua_pushnil(L);
			lua_setmetatable(L, -2);
			lua_pop(L, 1);

			lua_pushnil(L);
			lua_setfield(L, -2, ".c_instance");
			lua_pop(L, 1);
		}
		if(ref != LUA_NOREF) {

			luaL_unref(L, LUA_REGISTRYINDEX, ref);
			ref = LUA_NOREF;
		}
	}
}

#endif
