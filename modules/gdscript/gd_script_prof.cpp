/*************************************************************************/
/*  gd_script.cpp                                                        */
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
#include "gd_script.h"

#ifdef ENABLE_PROFILER

#include "globals.h"
#include "global_constants.h"
#include "gd_compiler.h"
#include "os/os.h"
#include "os/file_access.h"
#include "io/file_access_encrypted.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

typedef struct FuncInfo {
	String path;
	int line;
	String name;
	uint64_t cost;
	uint64_t times;

	FuncInfo()
	{}

	FuncInfo(String& p_path, int p_line, String& p_name)
		: path(p_path)
		, line(p_line)
		, name(p_name)
		, cost(0)
		, times(0)
	{}

} FuncInfo;

#define MAX_STACK_LEVEL 1024
typedef struct Stack {
	FuncInfo *info;
	uint64_t enter;
} Stack;
static Stack stacks[MAX_STACK_LEVEL];
static int stack_level = 0;
//bool enable_profiler = false;

struct CostCompare {

	_FORCE_INLINE_ bool operator()(const FuncInfo* l,const FuncInfo* r) const {

		return l->cost > r->cost;
	}
};

void GDScriptLanguage::_profiler_start(GDFunction *p_function, int p_line) {

	//if(!enable_profiler)
	//	return;
	ERR_FAIL_COND(stack_level >= MAX_STACK_LEVEL);

	size_t id = (size_t) p_function;
	if(!func_infos.has(id)) {

		func_infos[id] = memnew(FuncInfo(
			p_function->get_script()->get_path(),
			p_line,
			p_function->get_name().operator String()
		));
	}

	Stack& stack = stacks[stack_level];
	QueryPerformanceCounter((LARGE_INTEGER *) &stack.enter);
	stack.info = func_infos[id];
	stack.info->times += 1;
	stack_level += 1;
}

void GDScriptLanguage::_profiler_end() {

	//if(!enable_profiler)
	//	return;
	ERR_FAIL_COND(stack_level == 0);

	Stack& stack = stacks[stack_level - 1];
	uint64_t now;
	QueryPerformanceCounter((LARGE_INTEGER *) &now);
	uint64_t cost = now - stack.enter;

	for(int level = stack_level - 2; level >= 0; level--) {

		Stack& prev = stacks[level];
		prev.enter += cost;
	}

	stack.info->cost += cost;
	stack_level -= 1;
}

void GDScriptLanguage::_profiler_dump() {

	if(func_infos.empty())
		return;

	uint64_t ticks_per_second;
	if( !QueryPerformanceFrequency((LARGE_INTEGER *)&ticks_per_second) )
		ticks_per_second = 1000;

	typedef List<FuncInfo *> FuncInfos;
	FuncInfos sort_funcs;

	const size_t *K = NULL;
	while(K = func_infos.next(K))
		sort_funcs.push_back(func_infos[*K]);

	sort_funcs.sort_custom<CostCompare>();

	FILE *fp = fopen("profiler.txt", "wb");

	//fwprintf()

	for(FuncInfos::Element *E = sort_funcs.front(); E; E = E->next()) {

		FuncInfo& info = *(E->get());
		// Divide by frequency to get the time in seconds
		uint64_t us = info.cost * 1000000L / ticks_per_second;

		//wprintf(L"file:%s func:%s line:%d\n\tcost\t%fs\n", info.path.ptr(), info.name.ptr(), info.line, us / 1000000.0);
		fwprintf(fp, L"file:%s func:%s line:%d\n\tcost\t%fs\ttimes:%lld\n",
			info.path.ptr(),
			info.name.ptr(),
			info.line,
			us / 1000000.0,
			info.times
		);
	}
	printf("Profiler file save at profiler.txt\n");
	fclose(fp);

	// cleanup profiler function infos
	{
		const size_t *K = NULL;
		while(K = func_infos.next(K))
			memdelete(func_infos[*K]);
		func_infos.clear();
	}
}
#else
void GDScriptLanguage::_profiler_start(GDFunction *p_function, int p_line) {
}

void GDScriptLanguage::_profiler_end() {
}

void GDScriptLanguage::_profiler_dump() {
}

#endif

