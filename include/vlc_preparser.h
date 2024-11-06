/*****************************************************************************
 * preparser.h
 *****************************************************************************
 * Copyright (C) 1999-2023 VLC authors and VideoLAN
 *
 * Authors: Samuel Hocevar <sam@zoy.org>
 *          Clément Stenac <zorglub@videolan.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifndef VLC_PREPARSER_H
#define VLC_PREPARSER_H 1

#include <vlc_input_item.h>

/**
 * @defgroup vlc_preparser Preparser
 * @ingroup input
 * @{
 * @file
 * VLC Preparser API
 */

/**
 * Preparser opaque structure.
 *
 * The preparser object will retrieve the meta data of any given input item in
 * an asynchronous way.
 * It will also issue art fetching requests.
 */
typedef struct vlc_preparser_t vlc_preparser_t;
typedef size_t vlc_preparser_req_id;

#define VLC_PREPARSER_REQ_ID_INVALID 0

#define VLC_PREPARSER_TYPE_PARSE            0x01
#define VLC_PREPARSER_TYPE_FETCHMETA_LOCAL  0x02
#define VLC_PREPARSER_TYPE_FETCHMETA_NET    0x04
#define VLC_PREPARSER_TYPE_FETCHMETA_ALL \
    (VLC_PREPARSER_TYPE_FETCHMETA_LOCAL|VLC_PREPARSER_TYPE_FETCHMETA_NET)

#define VLC_PREPARSER_OPTION_INTERACT 0x1000
#define VLC_PREPARSER_OPTION_SUBITEMS 0x2000

/**
 * This function creates the preparser object and thread.
 *
 * @param obj the parent object
 * @param max_threads the maximum number of threads used to parse, must be >= 1
 * @param timeout timeout of the preparser, 0 for no limits.
 * @param types a combination of VLC_PREPARSER_TYPE_* flags, it is used to
 * setup the executors for each domain. Its possible to select more than one
 * types
 * @return a valid preparser object or NULL in case of error
 */
VLC_API vlc_preparser_t *vlc_preparser_New( vlc_object_t *obj,
                                            unsigned max_threads,
                                            vlc_tick_t timeout, int types );

/**
 * This function enqueues the provided item to be preparsed or fetched.
 *
 * The input item is retained until the preparsing is done or until the
 * preparser object is deleted.
 *
 * @param preparser the preparser object
 * @param item a valid item to preparse
 * @param type_option a combination of VLC_PREPARSER_TYPE_* and
 * VLC_PREPARSER_OPTION_* flags. The type must be in the set specified in
 * vlc_preparser_New() (it is possible to select less types).
 * @param cbs callback to listen to events (can't be NULL)
 * @param cbs_userdata opaque pointer used by the callbacks
 * @param id unique id provided by the caller. This is can be used to cancel
 * the request with vlc_preparser_Cancel()
 * @return VLC_PREPARSER_REQ_ID_INVALID in case of error, or a valid id if the
 * item was scheduled for preparsing. If this returns an
 * error, the on_preparse_ended will *not* be invoked
 */
VLC_API vlc_preparser_req_id
vlc_preparser_Push( vlc_preparser_t *preparser, input_item_t *item, int type_option,
                    const input_item_parser_cbs_t *cbs, void *cbs_userdata );

/**
 * This function cancel all preparsing requests for a given id
 *
 * @param preparser the preparser object
 * @param id unique id returned by vlc_preparser_Push(),
 * VLC_PREPARSER_REQ_ID_INVALID to cancels all tasks
 * @return number of tasks cancelled
 */
VLC_API size_t vlc_preparser_Cancel( vlc_preparser_t *preparser,
                                     vlc_preparser_req_id id );

/**
 * This function destroys the preparser object and thread.
 *
 * @param preparser the preparser object
 * All pending input items will be released.
 */
VLC_API void vlc_preparser_Delete( vlc_preparser_t *preparser );

/**
 * This function deactivates the preparser
 *
 * All pending requests will be removed, and it will block until the currently
 * running entity has finished (if any).
 *
 * @param preparser the preparser object
 */
VLC_API void vlc_preparser_Deactivate( vlc_preparser_t *preparser );

/**
 * Do not use, libVLC only fonction, will be removed soon
 */
VLC_API void vlc_preparser_SetTimeout( vlc_preparser_t *preparser,
                                       vlc_tick_t timeout ) VLC_DEPRECATED;

/** @} vlc_preparser */

#endif

