/*************************************************************************/
/*  grid_container.cpp                                                   */
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
#include "grid_container.h"


void GridContainer::_notification(int p_what) {

	switch(p_what) {

		case NOTIFICATION_SORT_CHILDREN: {

			Map<int,int> col_minw;
			Map<int,int> row_minh;
			Set<int> col_expanded;
			Set<int> row_expanded;

			int sep=get_constant("separation");
			bool center_sep=get_constant("center_separation") != 0;

			int idx=0;
			int max_row=0;
			int max_col=0;

			Size2 size = get_size();


			for(int i=0;i<get_child_count();i++) {

				Control *c = get_child(i)->cast_to<Control>();
				if (!c || !c->is_visible())
					continue;

				int row = idx / columns;
				int col = idx % columns;

				Size2i ms = c->get_combined_minimum_size();
				if (col_minw.has(col))
					col_minw[col] = MAX(col_minw[col],ms.width);
				else
					col_minw[col]=ms.width;

				if (row_minh.has(row))
					row_minh[row] = MAX(row_minh[row],ms.height);
				else
					row_minh[row]=ms.height;


				if (c->get_h_size_flags()&SIZE_EXPAND)
					col_expanded.insert(col);
				if (c->get_v_size_flags()&SIZE_EXPAND)
					row_expanded.insert(row);

				max_col=MAX(col,max_col);
				max_row=MAX(row,max_row);
				idx++;
			}

			Size2 ms;
			int expand_rows=0;
			int expand_cols=0;

			for (Map<int,int>::Element *E=col_minw.front();E;E=E->next()) {
				ms.width+=E->get();
				if (col_expanded.has(E->key()))
					expand_cols++;
			}

			for (Map<int,int>::Element *E=row_minh.front();E;E=E->next()) {
				ms.height+=E->get();
				if (row_expanded.has(E->key()))
					expand_rows++;
			}

			ms.height+=sep*max_row;
			ms.width+=sep*max_col;

			int row_expand = expand_rows?(size.y-ms.y)/expand_rows:0;
			int col_expand = expand_cols?(size.x-ms.x)/expand_cols:0;


			int col_ofs=0;
			int row_ofs=0;
			idx=0;

			for(int i=0;i<get_child_count();i++) {

				Control *c = get_child(i)->cast_to<Control>();
				if (!c || !c->is_visible())
					continue;
				int row = idx / columns;
				int col = idx % columns;

				if (col==0) {
					col_ofs=0;
					if (row>0 && row_minh.has(row-1))
						row_ofs+=row_minh[row-1]+sep+(row_expanded.has(row-1)?row_expand:0);
				}

				if (c->is_visible()) {
					Size2 s;
					if (col_minw.has(col))
						s.width=col_minw[col];
					if (row_minh.has(row))
						s.height=row_minh[col];

					if (row_expanded.has(row))
						s.height+=row_expand;
					if (col_expanded.has(col))
						s.width+=col_expand;

					Point2 p(col_ofs,row_ofs);
					if (center_sep)
						p+=Point2(sep/2,sep/2);

					fit_child_in_rect(c,Rect2(p,s));

				}

				if (col_minw.has(col)) {
					col_ofs+=col_minw[col]+sep+(col_expanded.has(col)?col_expand:0);
				}

				idx++;
			}



		} break;
	}

}

void GridContainer::set_columns(int p_columns) {

	columns=p_columns;
	queue_sort();

}

int GridContainer::get_columns() const{

	return columns;
}

void GridContainer::_bind_methods(){

	ObjectTypeDB::bind_method(_MD("set_columns","columns"),&GridContainer::set_columns);
	ObjectTypeDB::bind_method(_MD("get_columns"),&GridContainer::get_columns);

	ADD_PROPERTY( PropertyInfo(Variant::INT,"columns",PROPERTY_HINT_RANGE,"1,1024,1"),_SCS("set_columns"),_SCS("get_columns"));
}

Size2 GridContainer::get_minimum_size() const {

	Map<int,int> col_minw;
	Map<int,int> row_minh;

	int sep=get_constant("separation");
	bool center_sep=get_constant("center_separation") != 0;

	int idx=0;
	int max_row=0;
	int max_col=0;

	for(int i=0;i<get_child_count();i++) {

		Control *c = get_child(i)->cast_to<Control>();
		if (!c || !c->is_visible())
			continue;
		int row = idx / columns;
		int col = idx % columns;
		Size2i ms = c->get_combined_minimum_size();
		if (col_minw.has(col))
			col_minw[col] = MAX(col_minw[col],ms.width);
		else
			col_minw[col]=ms.width;

		if (row_minh.has(row))
			row_minh[row] = MAX(row_minh[row],ms.height);
		else
			row_minh[row]=ms.height;
		max_col=MAX(col,max_col);
		max_row=MAX(row,max_row);
		idx++;
	}

	Size2 ms;

	for (Map<int,int>::Element *E=col_minw.front();E;E=E->next()) {
		ms.width+=E->get();
	}

	for (Map<int,int>::Element *E=row_minh.front();E;E=E->next()) {
		ms.height+=E->get();
	}

	ms.height+=sep*max_row;
	ms.width+=sep*max_col;

	if (center_sep)
		ms+=Point2(sep,sep);

	return ms;

}


GridContainer::GridContainer() {

	columns=1;
}
