/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996,2007 Oracle.  All rights reserved.
 *
 * $Id: db_byteorder.c,v 12.7 2007/05/17 15:14:55 bostic Exp $
 */

#include "db_config.h"

#include "db_int.h"

/*
 * __db_isbigendian --
 *	Return 1 if big-endian (Motorola and Sparc), not little-endian
 *	(Intel and Vax).  We do this work at run-time, rather than at
 *	configuration time so cross-compilation and general embedded
 *	system support is simpler.
 *
 * PUBLIC: int __db_isbigendian __P((void));
 */
int
__db_isbigendian()
{
	union {					/* From Harbison & Steele.  */
		long l;
		char c[sizeof(long)];
	} u;

	u.l = 1;
	return (u.c[sizeof(long) - 1] == 1);
}

/*
 * __db_byteorder --
 *	Return if we need to do byte swapping, checking for illegal
 *	values.
 *
 * PUBLIC: int __db_byteorder __P((DB_ENV *, int));
 */
int
__db_byteorder(dbenv, lorder)
	DB_ENV *dbenv;
	int lorder;
{
	int is_bigendian;

	is_bigendian = __db_isbigendian();

	switch (lorder) {
	case 0:
		break;
	case 1234:
		if (is_bigendian)
			return (DB_SWAPBYTES);
		break;
	case 4321:
		if (!is_bigendian)
			return (DB_SWAPBYTES);
		break;
	default:
		__db_errx(dbenv,
	    "unsupported byte order, only big and little-endian supported");
		return (EINVAL);
	}
	return (0);
}
