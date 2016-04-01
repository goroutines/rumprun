/*	$NetBSD: ulfs_quota.h,v 1.5 2014/06/28 22:27:51 dholland Exp $	*/
/*  from NetBSD: ufs_quota.h,v 1.21 2012/02/18 06:13:23 matt Exp  */

/*
 * Copyright (c) 1982, 1986, 1990, 1993, 1995
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Robert Elz at The University of Melbourne.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)ufs_quota.c	8.5 (Berkeley) 5/20/95
 */
#include <ufs/lfs/ulfs_quota1.h>
#include <ufs/lfs/ulfs_quota2.h>

struct quotakcursor; /* from <sys/quotactl.h> */


/* link to this quota in the quota inode (for QUOTA2) */
struct dq2_desc {
	uint64_t dq2_lblkno; /* logical disk block holding this quota */
	u_int    dq2_blkoff; /* offset in disk block holding this quota */
};

/*
 * The following structure records disk usage for a user or group on a
 * filesystem. There is one allocated for each quota that exists on any
 * filesystem for the current user or group. A cache is kept of recently
 * used entries.
 * Field markings and the corresponding locks:
 * h:	dqlock
 * d:	dq_interlock
 *
 * Lock order is: dq_interlock -> dqlock
 *                dq_interlock -> dqvp
 */
struct dquot {
	LIST_ENTRY(dquot) dq_hash;	/* h: hash list */
	u_int16_t dq_flags;		/* d: flags, see below */
	u_int16_t dq_type;		/* d: quota type of this dquot */
	u_int32_t dq_cnt;		/* h: count of active references */
	u_int32_t dq_id;		/* d: identifier this applies to */
	struct	ulfsmount *dq_ump;	/* d: filesystem this is taken from */
	kmutex_t dq_interlock;		/* d: lock this dquot */
	union {
		struct dqblk dq1_dqb;	/* d: actual usage & quotas */
		struct dq2_desc dq2_desc; /* d: pointer to quota data */
	} dq_un;
};

/*
 * Flag values.
 */
#define	DQ_MOD		0x04		/* this quota modified since read */
#define	DQ_FAKE		0x08		/* no limits here, just usage */
#define	DQ_WARN(ltype)	(0x10 << ltype)	/* has been warned about "type" limit */
/*
 * Shorthand notation.
 */
#define	dq_bhardlimit	dq_un.dq1_dqb.dqb_bhardlimit
#define	dq_bsoftlimit	dq_un.dq1_dqb.dqb_bsoftlimit
#define	dq_curblocks	dq_un.dq1_dqb.dqb_curblocks
#define	dq_ihardlimit	dq_un.dq1_dqb.dqb_ihardlimit
#define	dq_isoftlimit	dq_un.dq1_dqb.dqb_isoftlimit
#define	dq_curinodes	dq_un.dq1_dqb.dqb_curinodes
#define	dq_btime	dq_un.dq1_dqb.dqb_btime
#define	dq_itime	dq_un.dq1_dqb.dqb_itime

#define dq2_lblkno	dq_un.dq2_desc.dq2_lblkno
#define dq2_blkoff	dq_un.dq2_desc.dq2_blkoff
/*
 * If the system has never checked for a quota for this file, then it is
 * set to NODQUOT.  Once a write attempt is made the inode pointer is set
 * to reference a dquot structure.
 */
#define	NODQUOT		NULL

extern kmutex_t lfs_dqlock;
extern kcondvar_t lfs_dqcv;
/*
 * Quota name to error message mapping.
 */
extern const char *lfs_quotatypes[ULFS_MAXQUOTAS];

int  lfs_getinoquota(struct inode *);
int  lfs_dqget(struct vnode *, u_long, struct ulfsmount *, int, struct dquot **);
void lfs_dqref(struct dquot *);
void lfs_dqrele(struct vnode *, struct dquot *);
void lfs_dqflush(struct vnode *);

int lfs_chkdq1(struct inode *, int64_t, kauth_cred_t, int);
int lfs_chkiq1(struct inode *, int32_t, kauth_cred_t, int);
int lfs_q1sync(struct mount *);
int lfs_dq1get(struct vnode *, u_long, struct ulfsmount *, int, struct dquot *);
int lfs_dq1sync(struct vnode *, struct dquot *);
int lfsquota1_handle_cmd_get(struct ulfsmount *, const struct quotakey *,
    struct quotaval *);
int lfsquota1_handle_cmd_put(struct ulfsmount *, const struct quotakey *,
    const struct quotaval *);
int lfsquota1_handle_cmd_quotaon(struct lwp *, struct ulfsmount *, int,
    const char *);
int lfsquota1_handle_cmd_quotaoff(struct lwp *, struct ulfsmount *, int);

int lfs_chkdq2(struct inode *, int64_t, kauth_cred_t, int);
int lfs_chkiq2(struct inode *, int32_t, kauth_cred_t, int);
int lfsquota2_handle_cmd_get(struct ulfsmount *, const struct quotakey *,
    struct quotaval *);
int lfsquota2_handle_cmd_put(struct ulfsmount *, const struct quotakey *,
    const struct quotaval *);
int lfsquota2_handle_cmd_del(struct ulfsmount *, const struct quotakey *);
int lfsquota2_handle_cmd_cursorget(struct ulfsmount *, struct quotakcursor *,
    struct quotakey *, struct quotaval *, unsigned, unsigned *);
int lfsquota2_handle_cmd_cursoropen(struct ulfsmount *, struct quotakcursor *);
int lfsquota2_handle_cmd_cursorclose(struct ulfsmount *, struct quotakcursor *);
int lfsquota2_handle_cmd_cursorskipidtype(struct ulfsmount *, struct quotakcursor *,
    int);
int lfsquota2_handle_cmd_cursoratend(struct ulfsmount *, struct quotakcursor *,
    int *);
int lfsquota2_handle_cmd_cursorrewind(struct ulfsmount *, struct quotakcursor *);
int lfs_q2sync(struct mount *);
int lfs_dq2get(struct vnode *, u_long, struct ulfsmount *, int, struct dquot *);
int lfs_dq2sync(struct vnode *, struct dquot *);
