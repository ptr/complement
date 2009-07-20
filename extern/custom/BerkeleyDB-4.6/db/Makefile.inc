# -*- makefile -*- Time-stamp: <09/03/19 09:11:25 ptr>

LIBNAME = db
MAJOR = 4
MINOR = 6
PATCH = 21

SRC_BTREE = \
        ${BERKELEYDB_SRC}/btree/bt_compare.c \
        ${BERKELEYDB_SRC}/btree/bt_conv.c \
        ${BERKELEYDB_SRC}/btree/bt_curadj.c \
        ${BERKELEYDB_SRC}/btree/bt_cursor.c \
        ${BERKELEYDB_SRC}/btree/bt_delete.c \
        ${BERKELEYDB_SRC}/btree/bt_method.c \
        ${BERKELEYDB_SRC}/btree/bt_open.c \
        ${BERKELEYDB_SRC}/btree/bt_put.c \
        ${BERKELEYDB_SRC}/btree/bt_rec.c \
        ${BERKELEYDB_SRC}/btree/bt_reclaim.c \
        ${BERKELEYDB_SRC}/btree/bt_recno.c \
        ${BERKELEYDB_SRC}/btree/bt_rsearch.c \
        ${BERKELEYDB_SRC}/btree/bt_search.c \
        ${BERKELEYDB_SRC}/btree/bt_split.c \
        ${BERKELEYDB_SRC}/btree/bt_stat.c \
        ${BERKELEYDB_SRC}/btree/bt_compact.c \
        ${BERKELEYDB_SRC}/btree/bt_upgrade.c \
        ${BERKELEYDB_SRC}/btree/btree_auto.c

SRC_BTREE_VRFY = \
        ${BERKELEYDB_SRC}/db/db_ovfl_vrfy.c \
        ${BERKELEYDB_SRC}/db/db_vrfy.c \
        ${BERKELEYDB_SRC}/db/db_vrfyutil.c \
        ${BERKELEYDB_SRC}/db/bt_verify.c

SRC_HASH = \
        ${BERKELEYDB_SRC}/hash/hash.c \
        ${BERKELEYDB_SRC}/hash/hash_auto.c \
        ${BERKELEYDB_SRC}/hash/hash_conv.c \
        ${BERKELEYDB_SRC}/hash/hash_dup.c \
        ${BERKELEYDB_SRC}/hash/hash_meta.c \
        ${BERKELEYDB_SRC}/hash/hash_method.c \
        ${BERKELEYDB_SRC}/hash/hash_open.c \
        ${BERKELEYDB_SRC}/hash/hash_page.c \
        ${BERKELEYDB_SRC}/hash/hash_rec.c \
        ${BERKELEYDB_SRC}/hash/hash_reclaim.c \
        ${BERKELEYDB_SRC}/hash/hash_stat.c \
        ${BERKELEYDB_SRC}/hash/hash_upgrade.c

SRC_HASH_VRFY = \
        ${BERKELEYDB_SRC}/hash/hash_verify.c

SRC_QUEUE = \
        ${BERKELEYDB_SRC}/qam/qam.c \
        ${BERKELEYDB_SRC}/qam/qam_auto.c \
        ${BERKELEYDB_SRC}/qam/qam_conv.c \
        ${BERKELEYDB_SRC}/qam/qam_files.c \
        ${BERKELEYDB_SRC}/qam/qam_method.c \
        ${BERKELEYDB_SRC}/qam/qam_open.c \
        ${BERKELEYDB_SRC}/qam/qam_rec.c \
        ${BERKELEYDB_SRC}/qam/qam_stat.c \
        ${BERKELEYDB_SRC}/qam/qam_upgrade.c

SRC_QUEUE_VRFY = \
        ${BERKELEYDB_SRC}/qam/qam_verify.c

SRC_LOCK = \
        ${BERKELEYDB_SRC}/lock/lock.c \
        ${BERKELEYDB_SRC}/lock/lock_deadlock.c \
        ${BERKELEYDB_SRC}/lock/lock_failchk.c \
        ${BERKELEYDB_SRC}/lock/lock_id.c \
        ${BERKELEYDB_SRC}/lock/lock_list.c \
        ${BERKELEYDB_SRC}/lock/lock_method.c \
        ${BERKELEYDB_SRC}/lock/lock_region.c \
        ${BERKELEYDB_SRC}/lock/lock_stat.c \
        ${BERKELEYDB_SRC}/lock/lock_timer.c \
        ${BERKELEYDB_SRC}/lock/lock_util.c

SRC_MUTEX = \
        ${BERKELEYDB_SRC}/mutex/mut_alloc.c \
        ${BERKELEYDB_SRC}/mutex/mut_failchk.c \
        ${BERKELEYDB_SRC}/mutex/mut_method.c \
        ${BERKELEYDB_SRC}/mutex/mut_region.c \
        ${BERKELEYDB_SRC}/mutex/mut_stat.c

SRC_REP = \
        ${BERKELEYDB_SRC}/rep/rep_auto.c \
        ${BERKELEYDB_SRC}/rep/rep_backup.c \
        ${BERKELEYDB_SRC}/rep/rep_elect.c \
        ${BERKELEYDB_SRC}/rep/rep_lease.c \
        ${BERKELEYDB_SRC}/rep/rep_log.c \
        ${BERKELEYDB_SRC}/rep/rep_method.c \
        ${BERKELEYDB_SRC}/rep/rep_record.c \
        ${BERKELEYDB_SRC}/rep/rep_region.c \
        ${BERKELEYDB_SRC}/rep/rep_stat.c \
        ${BERKELEYDB_SRC}/rep/rep_util.c \
        ${BERKELEYDB_SRC}/rep/rep_verify.c

SRC_REPMGR = \
        ${BERKELEYDB_SRC}/repmgr/getaddrinfo.c \
        ${BERKELEYDB_SRC}/repmgr/repmgr_elect.c \
        ${BERKELEYDB_SRC}/repmgr/repmgr_method.c \
        ${BERKELEYDB_SRC}/repmgr/repmgr_msg.c \
        ${BERKELEYDB_SRC}/repmgr/repmgr_net.c \
        ${BERKELEYDB_SRC}/repmgr/repmgr_posix.c \
        ${BERKELEYDB_SRC}/repmgr/repmgr_queue.c \
        ${BERKELEYDB_SRC}/repmgr/repmgr_sel.c \
        ${BERKELEYDB_SRC}/repmgr/repmgr_stat.c \
        ${BERKELEYDB_SRC}/repmgr/repmgr_util.c

SRC_CRYPTO = \
        ${BERKELEYDB_SRC}/crypto/aes_method.c \
        ${BERKELEYDB_SRC}/crypto/crypto.c \
        ${BERKELEYDB_SRC}/crypto/mersenne/mt19937db.c \
        ${BERKELEYDB_SRC}/crypto/rijndael/rijndael-alg-fst.c \
        ${BERKELEYDB_SRC}/crypto/rijndael/rijndael-api-fst.c

SRC_C = \
        ${BERKELEYDB_SRC}/db185/db185.c \
        ${BERKELEYDB_SRC}/mutex/mut_tas.c \
        ${BERKELEYDB_SRC}/mutex/mut_pthread.c \
        $(SRC_BTREE) \
        $(SRC_HASH) \
        $(SRC_HASH_VRFY) \
        $(SRC_QUEUE) \
        $(SRC_QUEUE_VRFY) \
        $(SRC_REP) \
        $(SRC_REPMGR) \
        $(SRC_BTREE_VRFY) \
        $(SRC_LOCK) \
        $(SRC_MUTEX) \
        $(SRC_CRYPTO)  \
        ${BERKELEYDB_SRC}/db/crdel_auto.c \
        ${BERKELEYDB_SRC}/db/crdel_rec.c \
        ${BERKELEYDB_SRC}/clib/ctime.c \
        ${BERKELEYDB_SRC}/db/db.c \
        ${BERKELEYDB_SRC}/db/db_am.c \
        ${BERKELEYDB_SRC}/db/db_auto.c \
        ${BERKELEYDB_SRC}/common/db_byteorder.c \
        ${BERKELEYDB_SRC}/db/db_cam.c \
        ${BERKELEYDB_SRC}/db/db_cds.c \
        ${BERKELEYDB_SRC}/db/db_conv.c \
        ${BERKELEYDB_SRC}/db/db_dispatch.c \
        ${BERKELEYDB_SRC}/db/db_dup.c \
        ${BERKELEYDB_SRC}/common/db_err.c \
        ${BERKELEYDB_SRC}/common/db_getlong.c \
        ${BERKELEYDB_SRC}/common/db_idspace.c \
        ${BERKELEYDB_SRC}/db/db_iface.c \
        ${BERKELEYDB_SRC}/db/db_join.c \
        ${BERKELEYDB_SRC}/common/db_log2.c \
        ${BERKELEYDB_SRC}/db/db_meta.c \
        ${BERKELEYDB_SRC}/db/db_method.c \
        ${BERKELEYDB_SRC}/db/db_open.c \
        ${BERKELEYDB_SRC}/db/db_overflow.c \
        ${BERKELEYDB_SRC}/db/db_pr.c \
        ${BERKELEYDB_SRC}/db/db_rec.c \
        ${BERKELEYDB_SRC}/db/db_reclaim.c \
        ${BERKELEYDB_SRC}/db/db_remove.c \
        ${BERKELEYDB_SRC}/db/db_rename.c \
        ${BERKELEYDB_SRC}/db/db_ret.c \
        ${BERKELEYDB_SRC}/db/db_setid.c \
        ${BERKELEYDB_SRC}/db/db_setlsn.c \
        ${BERKELEYDB_SRC}/common/db_shash.c \
        ${BERKELEYDB_SRC}/db/db_stati.c \
        ${BERKELEYDB_SRC}/db/db_truncate.c \
        ${BERKELEYDB_SRC}/db/db_upg.c \
        ${BERKELEYDB_SRC}/db/db_upg_opd.c \
        ${BERKELEYDB_SRC}/dbm/dbm.c \
        ${BERKELEYDB_SRC}/dbreg/dbreg.c \
        ${BERKELEYDB_SRC}/dbreg/dbreg_auto.c \
        ${BERKELEYDB_SRC}/dbreg/dbreg_rec.c \
        ${BERKELEYDB_SRC}/dbreg/dbreg_stat.c \
        ${BERKELEYDB_SRC}/dbreg/dbreg_util.c \
        ${BERKELEYDB_SRC}/env/env_alloc.c \
        ${BERKELEYDB_SRC}/env/env_config.c \
        ${BERKELEYDB_SRC}/env/env_failchk.c \
        ${BERKELEYDB_SRC}/env/env_file.c \
        ${BERKELEYDB_SRC}/env/env_globals.c \
        ${BERKELEYDB_SRC}/env/env_method.c \
        ${BERKELEYDB_SRC}/env/env_name.c \
        ${BERKELEYDB_SRC}/env/env_open.c \
        ${BERKELEYDB_SRC}/env/env_recover.c \
        ${BERKELEYDB_SRC}/env/env_region.c \
        ${BERKELEYDB_SRC}/env/env_register.c \
        ${BERKELEYDB_SRC}/env/env_stat.c \
        ${BERKELEYDB_SRC}/fileops/fileops_auto.c \
        ${BERKELEYDB_SRC}/fileops/fop_basic.c \
        ${BERKELEYDB_SRC}/fileops/fop_rec.c \
        ${BERKELEYDB_SRC}/fileops/fop_util.c \
        ${BERKELEYDB_SRC}/hash/hash_func.c \
        ${BERKELEYDB_SRC}/hmac/hmac.c \
        ${BERKELEYDB_SRC}/hsearch/hsearch.c \
        ${BERKELEYDB_SRC}/log/log.c \
        ${BERKELEYDB_SRC}/log/log_archive.c \
        ${BERKELEYDB_SRC}/log/log_compare.c \
        ${BERKELEYDB_SRC}/log/log_debug.c \
        ${BERKELEYDB_SRC}/log/log_get.c \
        ${BERKELEYDB_SRC}/log/log_method.c \
        ${BERKELEYDB_SRC}/log/log_put.c \
        ${BERKELEYDB_SRC}/log/log_stat.c \
        ${BERKELEYDB_SRC}/common/mkpath.c \
        ${BERKELEYDB_SRC}/mp/mp_alloc.c \
        ${BERKELEYDB_SRC}/mp/mp_bh.c \
        ${BERKELEYDB_SRC}/mp/mp_fget.c \
        ${BERKELEYDB_SRC}/mp/mp_fmethod.c \
        ${BERKELEYDB_SRC}/mp/mp_fopen.c \
        ${BERKELEYDB_SRC}/mp/mp_fput.c \
        ${BERKELEYDB_SRC}/mp/mp_fset.c \
        ${BERKELEYDB_SRC}/mp/mp_method.c \
        ${BERKELEYDB_SRC}/mp/mp_mvcc.c \
        ${BERKELEYDB_SRC}/mp/mp_region.c \
        ${BERKELEYDB_SRC}/mp/mp_register.c \
        ${BERKELEYDB_SRC}/mp/mp_resize.c \
        ${BERKELEYDB_SRC}/mp/mp_stat.c \
        ${BERKELEYDB_SRC}/mp/mp_sync.c \
        ${BERKELEYDB_SRC}/mp/mp_trickle.c \
        ${BERKELEYDB_SRC}/os/os_abort.c \
        ${BERKELEYDB_SRC}/os/os_abs.c \
        ${BERKELEYDB_SRC}/os/os_alloc.c \
        ${BERKELEYDB_SRC}/os/os_clock.c \
        ${BERKELEYDB_SRC}/os/os_config.c \
        ${BERKELEYDB_SRC}/os/os_dir.c \
        ${BERKELEYDB_SRC}/os/os_errno.c \
        ${BERKELEYDB_SRC}/os/os_fid.c \
        ${BERKELEYDB_SRC}/os/os_flock.c \
        ${BERKELEYDB_SRC}/os/os_fsync.c \
        ${BERKELEYDB_SRC}/os/os_getenv.c \
        ${BERKELEYDB_SRC}/os/os_handle.c \
        ${BERKELEYDB_SRC}/os/os_map.c \
        ${BERKELEYDB_SRC}/common/os_method.c \
        ${BERKELEYDB_SRC}/os/os_mkdir.c \
        ${BERKELEYDB_SRC}/os/os_oflags.c \
        ${BERKELEYDB_SRC}/os/os_open.c \
        ${BERKELEYDB_SRC}/os/os_pid.c \
        ${BERKELEYDB_SRC}/os/os_region.c \
        ${BERKELEYDB_SRC}/os/os_rename.c \
        ${BERKELEYDB_SRC}/os/os_root.c \
        ${BERKELEYDB_SRC}/os/os_rpath.c \
        ${BERKELEYDB_SRC}/os/os_rw.c \
        ${BERKELEYDB_SRC}/os/os_seek.c \
        ${BERKELEYDB_SRC}/os/os_sleep.c \
        ${BERKELEYDB_SRC}/os/os_spin.c \
        ${BERKELEYDB_SRC}/os/os_stat.c \
        ${BERKELEYDB_SRC}/os/os_tmpdir.c \
        ${BERKELEYDB_SRC}/os/os_truncate.c \
        ${BERKELEYDB_SRC}/os/os_uid.c \
        ${BERKELEYDB_SRC}/os/os_unlink.c \
        ${BERKELEYDB_SRC}/os/os_yield.c \
        ${BERKELEYDB_SRC}/sequence/seq_stat.c \
        ${BERKELEYDB_SRC}/sequence/sequence.c \
        ${BERKELEYDB_SRC}/hmac/sha1.c \
        ${BERKELEYDB_SRC}/clib/snprintf.c \
        ${BERKELEYDB_SRC}/txn/txn.c \
        ${BERKELEYDB_SRC}/txn/txn_auto.c \
        ${BERKELEYDB_SRC}/txn/txn_chkpt.c \
        ${BERKELEYDB_SRC}/txn/txn_failchk.c \
        ${BERKELEYDB_SRC}/txn/txn_method.c \
        ${BERKELEYDB_SRC}/txn/txn_rec.c \
        ${BERKELEYDB_SRC}/txn/txn_recover.c \
        ${BERKELEYDB_SRC}/txn/txn_region.c \
        ${BERKELEYDB_SRC}/txn/txn_stat.c \
        ${BERKELEYDB_SRC}/txn/txn_util.c \
        ${BERKELEYDB_SRC}/xa/xa.c \
        ${BERKELEYDB_SRC}/xa/xa_db.c \
        ${BERKELEYDB_SRC}/xa/xa_map.c

