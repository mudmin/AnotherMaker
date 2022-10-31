/*
 * Copyright (C) 2013 Frederic Vecoven
 *
 * This file is part of trs_hard
 *
 * trs_hard is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * trs_hard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _ACTION_H
#define _ACTION_H

#include "integer.h"

#define ACTION_DS1307_RELOAD	0x01
#define ACTION_TICK				0x02
#define ACTION_TRS				0x04


/* hard drive actions */
#define ACTION_HARD_SEEK		0x1
#define ACTION_HARD_READ		0x2
#define ACTION_HARD_WRITE		0x3

/* extra function actions : 0x80 + command2 */
#define ACTION_EXTRA		0x80


extern near BYTE action_flags;
extern near BYTE action_type;
extern near BYTE action_status;


#endif
