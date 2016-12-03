/*!The Treasure Box Library
 * 
 * TBox is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * 
 * TBox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with TBox; 
 * If not, see <a href="http://www.gnu.org/licenses/"> http://www.gnu.org/licenses/</a>
 * 
 * Copyright (C) 2009 - 2017, ruki All rights reserved.
 *
 * @author      ruki
 * @file        core.h
 * @ingroup     coroutine
 *
 */
#ifndef TB_COROUTINE_STACKLESS_CORE_H
#define TB_COROUTINE_STACKLESS_CORE_H

/* //////////////////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"

/* //////////////////////////////////////////////////////////////////////////////////////
 * macros
 */

/*
 * Copyright (c) 2004-2005, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

// get core from coroutine
#define tb_lo_core(co)                      ((tb_lo_core_ref_t)(co))

// get core state
#define tb_lo_core_state(co)                (tb_lo_core(co)->state)

// set core state
#define tb_lo_core_state_set(co, val)       tb_lo_core(co)->state = (val)

// get core level
#define tb_lo_core_level(co)                (tb_lo_core(co)->level)

// increase core level
#define tb_lo_core_level_inc(co)            tb_lo_core(co)->level++

// decrease core level
#define tb_lo_core_level_dec(co)            tb_lo_core(co)->level--

#ifdef TB_COMPILER_IS_GCC
/*
 * Implementation of local continuations based on the "Labels as
 * values" feature of gcc
 * 
 * @author Adam Dunkels <adam@sics.se>
 *
 * This implementation of local continuations is based on a special
 * feature of the GCC C compiler called "labels as values". This
 * feature allows assigning pointers with the address of the code
 * corresponding to a particular C label.
 *
 * For more information, see the GCC documentation:
 * http://gcc.gnu.org/onlinedocs/gcc/Labels-as-Values.html
 */
#   define tb_lo_core_init(co)      tb_lo_core(co)->branch = tb_null; tb_lo_core(co)->state = TB_STATE_READY; tb_lo_core(co)->level = 0
#   define tb_lo_core_resume(co) \
    if (tb_lo_core(co)->branch) \
    { \
        goto *(tb_lo_core(co)->branch); \
    } \
    else

#   define tb_lo_core_record(co) \
    do \
    { \
        __tb_mconcat_ex__(__tb_lo_core_label, __tb_line__): \
        tb_lo_core(co)->branch = &&__tb_mconcat_ex__(__tb_lo_core_label, __tb_line__); \
        \
    } while(0)

#   define tb_lo_core_exit(co)      tb_lo_core(co)->branch = tb_null, tb_lo_core(co)->state = TB_STATE_END, tb_lo_core_level_dec(co)

#else

/*
 * Implementation of local continuations based on switch() statment
 *
 * @author Adam Dunkels <adam@sics.se>
 *
 * This implementation of local continuations uses the C switch()
 * statement to resume execution of a function somewhere inside the
 * function's body. The implementation is based on the fact that
 * switch() statements are able to jump directly into the bodies of
 * control structures such as if() or while() statmenets.
 *
 * This implementation borrows heavily from Simon Tatham's coroutines
 * implementation in C:
 * http://www.chiark.greenend.org.uk/~sgtatham/coroutines.html
 *
 *
 *
 * WARNING! the implementation using switch() does not work if an
 * core_set() is done within another switch() statement!
 */
#   define tb_lo_core_init(co)      tb_lo_core(co)->branch = 0; tb_lo_core(co)->state = TB_STATE_READY; tb_lo_core(co)->level = 0
#   define tb_lo_core_resume(co)    switch (tb_lo_core(co)->branch) case 0:
#   define tb_lo_core_record(co)    tb_lo_core(co)->branch = (tb_uint16_t)__tb_line__; case __tb_line__:
#   define tb_lo_core_exit(co)      tb_lo_core(co)->branch = 0, tb_lo_core(co)->state = TB_STATE_END, tb_lo_core_level_dec(co)
#endif

/* //////////////////////////////////////////////////////////////////////////////////////
 * types
 */

// the stackless coroutine core type
#ifdef TB_COMPILER_IS_GCC
typedef struct __tb_lo_core_t
{
    // the code branch
    tb_pointer_t        branch;

    /* the state
     *
     * TB_STATE_READY
     * TB_STATE_SUSPEND
     * TB_STATE_END
     */
    tb_uint8_t          state;

    // the level of child coroutines (root: 1)
    tb_uint8_t          level;

}tb_lo_core_t, *tb_lo_core_ref_t;
#else
typedef struct __tb_lo_core_t
{
    // the code branch
    tb_uint16_t         branch;

    /* the state
     *
     * TB_STATE_READY
     * TB_STATE_SUSPEND
     * TB_STATE_END
     */
    tb_uint8_t          state;

    // the level of child coroutines (root: 1)
    tb_uint8_t          level;

}tb_lo_core_t, *tb_lo_core_ref_t;
#endif

#endif
