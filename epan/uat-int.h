/*
 *  uat-int.h
 *
 *  $Id$
 *
 *  User Accessible Tables
 *  Mantain an array of user accessible data strucures
 *  Internal interface
 *
 * (c) 2007, Luis E. Garcia Ontanon <luis.ontanon@gmail.com>
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 2001 Gerald Combs
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
#ifndef _UAT_INT_H_
#define _UAT_INT_H_

#include "uat.h"

typedef struct _uat_fld_rep_t uat_fld_rep_t;
typedef struct _uat_rep_t uat_rep_t;

typedef void (*uat_rep_fld_free_cb_t)(uat_fld_rep_t*);
typedef void (*uat_rep_free_cb_t)(uat_rep_t*);

typedef struct _uat_fld_t {
	char* name;
	uat_text_mode_t mode;
	uat_fld_chk_cb_t chk_cb;
	uat_fld_set_cb_t set_cb;
	uat_fld_tostr_cb_t tostr_cb;
	
	guint colnum;
	uat_fld_rep_t* rep;
	uat_rep_fld_free_cb_t free_rep;
	
	struct _uat_fld_t* next;
} uat_fld_t;

struct _uat_t {
	char* name;	
	size_t record_size;
	char* filename;
	void** user_ptr;
	guint* nrows_p;
	uat_copy_cb_t copy_cb;
	uat_update_cb_t update_cb;
	uat_free_cb_t free_cb;

	uat_fld_t* fields;
	guint ncols;
	GArray* user_data;
	gboolean finalized;
	
	uat_rep_t* rep;
	uat_rep_free_cb_t free_rep;
};

gchar* uat_get_actual_filename(uat_t* uat, gboolean for_writing);
void uat_init(void);
void uat_reset(void);
void* uat_add_record(uat_t*, const void* orig_rec_ptr);
void uat_remove_record_idx(uat_t*, guint rec_idx);
void uat_destroy(uat_t*);
gboolean uat_save(uat_t* dt, char** error);
gboolean uat_load(uat_t* dt, char** error);

#define UAT_UPDATE(uat) do { *((uat)->user_ptr) = (void*)((uat)->user_data->data); *((uat)->nrows_p) = (uat)->user_data->len; } while(0)

#endif
