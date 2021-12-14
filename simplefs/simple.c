#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/buffer_head.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/version.h>
#include <linux/jbd2.h>
#include <linux/parser.h>
#include <linux/blkdev.h>

#include "super.h"

#ifndef f_dentry
#define f_dentry f_path.dentry
#endif

static DEFINE_MUTEX(simplefs_sb_lock);
static DEFINE_MUTEX(simplefs_inodes_mgmt_lock);
static DEFINE_MUTEX(simplefs_directory_children_update_lock);

static struct kmem_cache *sfs_inode_cachep;

void simplefs_sb_sync(struct super_block *vsb)
{
	struct buffer_head *bh = NULL;
	struct simplefs_super_block *sb = SIMPLEFS_SB(vsb);

	bh = sb_bread(vsb, SIMPLEFS_SUPERBLOCK_BLOCK_NUMBER);
	BUG_ON(!bh);

	bh->b_data = (char *)sb;
	mark_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	brelse(bh);
}

struct simplefs_inode *simplefs_inode_search(struct super_block *sb, struct simplefs_inode *start, struct simplefs_inode *search)
{
	uint64_t count = 0;
	while (start->inode_no != search->inode_no && count < SIMPLEFS_SB(sb)->inodes_count) {
		count++;
		start++;
	}

	if (start->inode_no == search->inode_no)
		return start;
	return NULL;
}

void simplefs_inode_add(struct super_block *vsb, struct simplefs_inode *inode)
{
	struct simplefs_super_block *sb = SIMPLEFS_SB(vsb);
	struct buffer_head *bh = NULL;
	struct simplefs_inode *inode_iterator = NULL;

	if (mutex_lock_interruptible(&simplefs_inodes_mgmt_lock))
		return;

	bh = sb_bread(vsb, SIMPLEFS_INODESTORE_BLOCK_NUMBER);
	BUG_ON(!bh);

	inode_iterator = (struct simplefs_inode *)bh->b_data;

	if (mutex_lock_interruptible(&simplefs_sb_lock))
		return;

	inode_iterator += sb->inodes_count;

	memcpy(inode_iterator, inode, sizeof(struct simplefs_inode));
	sb->inodes_count++;

	mark_buffer_dirty(bh);
	simplefs_sb_sync(vsb);
	brelse(bh);

	mutex_unlock(&simplefs_sb_lock);
	mutex_unlock(&simplefs_inodes_mgmt_lock);
}

int simplefs_sb_get_a_freeblock(struct super_block *vsb, uint64_t * out)
{
	struct simplefs_super_block *sb = SIMPLEFS_SB(vsb);
	int i;
	int ret = 0;

	if (mutex_lock_interruptible(&simplefs_sb_lock)) {
		ret = -EINTR;
		goto end;
	}

	for (i = 2; i < SIMPLEFS_MAX_FILESYSTEM_OBJECTS_SUPPORTED; i++)
		if (sb->free_blocks & (1 << i))
			break;

	if (unlikely(i == SIMPLEFS_MAX_FILESYSTEM_OBJECTS_SUPPORTED)) {
		ret = -ENOSPC;
		goto end;
	}

	*out = i;
	sb->free_blocks &= ~(1 << i);
	simplefs_sb_sync(vsb);

end:
	mutex_unlock(&simplefs_sb_lock);
	return ret;
}

static int simplefs_sb_get_objects_count(struct super_block *vsb, uint64_t * out)
{
	struct simplefs_super_block *sb = SIMPLEFS_SB(vsb);

	if (mutex_lock_interruptible(&simplefs_inodes_mgmt_lock))
		return -EINTR;

	*out = sb->inodes_count;
	mutex_unlock(&simplefs_inodes_mgmt_lock);
	return 0;
}

static int simplefs_iterate(struct file *filp, struct dir_context *ctx) // iterate direntry
{
	loff_t off;
	struct inode *inode;
	struct super_block *sb;
	struct buffer_head *bh;
	struct simplefs_inode *sfs_inode;
	struct simplefs_dir_record *record;
	int i;

	off = ctx->pos;
	inode = filp->f_dentry->d_inode;
	sb = inode->i_sb;

	if (off)          // avoid infinite loop: since each directory can enter only once.
		return 0;

	sfs_inode = SIMPLEFS_INODE(inode);

    //printk("sfs_inode = %llx %llx %llx %llx \n", sfs_inode->mode, sfs_inode->inode_no, sfs_inode->data_block_number, sfs_inode->dir_children_count);
	if (unlikely(!S_ISDIR(sfs_inode->mode)))
		return -ENOTDIR;
		
	bh = sb_bread(sb, sfs_inode->data_block_number);
	BUG_ON(!bh);

	record = (struct simplefs_dir_record *)bh->b_data;
	for (i = 0; i < sfs_inode->dir_children_count; i++) {
		dir_emit(ctx, record->filename, SIMPLEFS_FILENAME_MAXLEN, record->inode_no, DT_UNKNOWN);
		ctx->pos += sizeof(struct simplefs_dir_record);
		off += sizeof(struct simplefs_dir_record);
		record++;
	}
	brelse(bh);

	return 0;
}


struct simplefs_inode *simplefs_get_inode(struct super_block *sb, uint64_t inode_no)
{
	struct simplefs_super_block *sfs_sb = SIMPLEFS_SB(sb);
	struct simplefs_inode *sfs_inode = NULL;
	struct simplefs_inode *inode_buffer = NULL;

	int i;
	struct buffer_head *bh;

	bh = sb_bread(sb, SIMPLEFS_INODESTORE_BLOCK_NUMBER);
	BUG_ON(!bh);

	sfs_inode = (struct simplefs_inode *)bh->b_data;

	for (i = 0; i < sfs_sb->inodes_count; i++) {
		if (sfs_inode->inode_no == inode_no) {
			inode_buffer = kmem_cache_alloc(sfs_inode_cachep, GFP_KERNEL);
			memcpy(inode_buffer, sfs_inode, sizeof(*inode_buffer));
			break;
		}
		sfs_inode++;
	}

	brelse(bh);
	return inode_buffer;
}

ssize_t simplefs_read(struct file * filp, char __user * buf, size_t len, loff_t * ppos)
{
	struct simplefs_inode *inode = SIMPLEFS_INODE(filp->f_path.dentry->d_inode);
	struct buffer_head *bh;

	char *buffer;
	int nbytes;

	if (*ppos >= inode->file_size)
		return 0;

	bh = sb_bread(filp->f_path.dentry->d_inode->i_sb, inode->data_block_number);

	if (!bh) 
		return 0;

	buffer = (char *)bh->b_data;
	nbytes = min((size_t) inode->file_size, len); // bug: fabricate inode->file_size

	if (copy_to_user(buf, buffer, nbytes)) { 
		brelse(bh);
		return -EFAULT;
	}

	brelse(bh);
	*ppos += nbytes;

	return nbytes;
}

int simplefs_inode_save(struct super_block *sb, struct simplefs_inode *sfs_inode)
{
	struct simplefs_inode *inode_iterator;
	struct buffer_head *bh;

	bh = sb_bread(sb, SIMPLEFS_INODESTORE_BLOCK_NUMBER);
	BUG_ON(!bh);

	if (mutex_lock_interruptible(&simplefs_sb_lock))
		return -EINTR;

	inode_iterator = simplefs_inode_search(sb, (struct simplefs_inode *)bh->b_data, sfs_inode);

	if (likely(inode_iterator)) {
		memcpy(inode_iterator, sfs_inode, sizeof(*inode_iterator));
		mark_buffer_dirty(bh);
		sync_dirty_buffer(bh);
	} else {
		mutex_unlock(&simplefs_sb_lock);
		return -EIO;
	}

	brelse(bh);
	mutex_unlock(&simplefs_sb_lock);

	return 0;
}

ssize_t simplefs_write(struct file * filp, const char __user * buf, size_t len, loff_t * ppos)
{
	struct inode *inode;
	struct simplefs_inode *sfs_inode;
	struct buffer_head *bh;
	struct super_block *sb;
	struct simplefs_super_block *sfs_sb;
	char *buffer;

	int retval;

	sb = filp->f_path.dentry->d_inode->i_sb;
	sfs_sb = SIMPLEFS_SB(sb);

	inode = filp->f_path.dentry->d_inode;
	sfs_inode = SIMPLEFS_INODE(inode);

	bh = sb_bread(filp->f_path.dentry->d_inode->i_sb, sfs_inode->data_block_number);

	if (!bh)
		return 0;

	buffer = (char *)bh->b_data;

	buffer += *ppos;
	if (copy_from_user(buffer, buf, len)) { // bug: no generic_write_checks
		brelse(bh);
		return -EFAULT;
	}
	*ppos += len;

	mark_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	brelse(bh);

	if (mutex_lock_interruptible(&simplefs_inodes_mgmt_lock))
		return -EINTR;

	sfs_inode->file_size = *ppos;
	retval = simplefs_inode_save(sb, sfs_inode);
	if (retval)
		len = retval;

	mutex_unlock(&simplefs_inodes_mgmt_lock);

	return len;
}

const struct file_operations simplefs_file_operations = {
	.read = simplefs_read,
	.write = simplefs_write,
};

const struct file_operations simplefs_dir_operations = {
	.owner = THIS_MODULE,
	.iterate = simplefs_iterate,
};

struct dentry *simplefs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags);
static int simplefs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl);
static int simplefs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode);

static struct inode_operations simplefs_inode_ops = {
	.create = simplefs_create,
	.lookup = simplefs_lookup,
	.mkdir = simplefs_mkdir,
};

static int simplefs_create_fs_object(struct inode *dir, struct dentry *dentry, umode_t mode)
{
	struct inode *inode;
	struct simplefs_inode *sfs_inode;
	struct super_block *sb;
	struct simplefs_inode *parent_dir_inode;
	struct buffer_head *bh;
	struct simplefs_dir_record *dir_contents_datablock;
	uint64_t count;
	int ret;

	if (mutex_lock_interruptible(&simplefs_directory_children_update_lock))
		return -EINTR;

	sb = dir->i_sb;
	ret = simplefs_sb_get_objects_count(sb, &count);
	if (ret < 0) {
		mutex_unlock(&simplefs_directory_children_update_lock);
		return ret;
	}

	if (unlikely(count >= SIMPLEFS_MAX_FILESYSTEM_OBJECTS_SUPPORTED)) {
		mutex_unlock(&simplefs_directory_children_update_lock);
		return -ENOSPC;
	}

	if (!S_ISDIR(mode) && !S_ISREG(mode)) {
		mutex_unlock(&simplefs_directory_children_update_lock);
		return -EINVAL;
	}

	inode = new_inode(sb);
	if (!inode) {
		mutex_unlock(&simplefs_directory_children_update_lock);
		return -ENOMEM;
	}

	inode->i_sb = sb;
	inode->i_op = &simplefs_inode_ops;
	inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);
	inode->i_ino = (count + SIMPLEFS_START_INO - SIMPLEFS_RESERVED_INODES + 1);

	sfs_inode = kmem_cache_alloc(sfs_inode_cachep, GFP_KERNEL);
	sfs_inode->inode_no = inode->i_ino;
	inode->i_private = sfs_inode;
	sfs_inode->mode = mode;

	if (S_ISDIR(mode)) {
		sfs_inode->dir_children_count = 0;
		inode->i_fop = &simplefs_dir_operations;
	} else if (S_ISREG(mode)) {
		sfs_inode->file_size = 0;
		inode->i_fop = &simplefs_file_operations;
	}

	ret = simplefs_sb_get_a_freeblock(sb, &sfs_inode->data_block_number);
	if (ret < 0) {
		mutex_unlock(&simplefs_directory_children_update_lock);
		return ret;
	}

	simplefs_inode_add(sb, sfs_inode);

	parent_dir_inode = SIMPLEFS_INODE(dir);
	bh = sb_bread(sb, parent_dir_inode->data_block_number);
	BUG_ON(!bh);

	dir_contents_datablock = (struct simplefs_dir_record *)bh->b_data;
	dir_contents_datablock += parent_dir_inode->dir_children_count;
	dir_contents_datablock->inode_no = sfs_inode->inode_no;
	strcpy(dir_contents_datablock->filename, dentry->d_name.name);

	mark_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	brelse(bh);

	if (mutex_lock_interruptible(&simplefs_inodes_mgmt_lock)) {
		mutex_unlock(&simplefs_directory_children_update_lock);
		return -EINTR;
	}

	parent_dir_inode->dir_children_count++;
	ret = simplefs_inode_save(sb, parent_dir_inode);
	if (ret) {
		mutex_unlock(&simplefs_inodes_mgmt_lock);
		mutex_unlock(&simplefs_directory_children_update_lock);
		return ret;
	}

	mutex_unlock(&simplefs_inodes_mgmt_lock);
	mutex_unlock(&simplefs_directory_children_update_lock);

	inode_init_owner(inode, dir, mode);
	d_add(dentry, inode);

	return 0;
}

static int simplefs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode)
{
	return simplefs_create_fs_object(dir, dentry, S_IFDIR | mode);
}

static int simplefs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
{
	return simplefs_create_fs_object(dir, dentry, mode);
}

static struct inode *simplefs_iget(struct super_block *sb, int ino)
{
	struct inode *inode;
	struct simplefs_inode *sfs_inode;

	sfs_inode = simplefs_get_inode(sb, ino);

	inode = new_inode(sb);
	inode->i_ino = ino;
	inode->i_sb = sb;
	inode->i_op = &simplefs_inode_ops;

	if (S_ISDIR(sfs_inode->mode))
		inode->i_fop = &simplefs_dir_operations;
	else if (S_ISREG(sfs_inode->mode))
		inode->i_fop = &simplefs_file_operations;
	else
		printk(KERN_ERR "Unknown inode type. Neither a directory nor a file");

	inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);
	inode->i_private = sfs_inode;

	return inode;
}

struct dentry *simplefs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags)
{
	struct simplefs_inode *parent = SIMPLEFS_INODE(parent_inode);
	struct super_block *sb = parent_inode->i_sb;
	struct buffer_head *bh;
	struct simplefs_dir_record *record;
	int i;

	bh = sb_bread(sb, parent->data_block_number);
	record = (struct simplefs_dir_record *)bh->b_data;
	for (i = 0; i < parent->dir_children_count; i++) {
		if (!strcmp(record->filename, child_dentry->d_name.name)) {
			struct inode *inode = simplefs_iget(sb, record->inode_no);
			inode_init_owner(inode, parent_inode, SIMPLEFS_INODE(inode)->mode);
			d_add(child_dentry, inode);
			return NULL;
		}
		record++;
	}
	return NULL;
}

void simplefs_destory_inode(struct inode *inode)
{
	struct simplefs_inode *sfs_inode = SIMPLEFS_INODE(inode);
	kmem_cache_free(sfs_inode_cachep, sfs_inode);
}

static void simplefs_put_super(struct super_block *sb)
{
}

static const struct super_operations simplefs_sops = {
	.destroy_inode = simplefs_destory_inode,
	.put_super = simplefs_put_super,
};

int simplefs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct inode *root_inode;
	struct buffer_head *bh;
	struct simplefs_super_block *sb_disk;

	bh = sb_bread(sb, 0);
	BUG_ON(!bh);

	sb_disk = (struct simplefs_super_block *)bh->b_data;
	sb->s_magic = SIMPLEFS_MAGIC;
	sb->s_fs_info = sb_disk;
	sb->s_maxbytes = SIMPLEFS_DEFAULT_BLOCK_SIZE;
	sb->s_op = &simplefs_sops;

	root_inode = new_inode(sb);
	root_inode->i_ino = SIMPLEFS_ROOTDIR_INODE_NUMBER;
	inode_init_owner(root_inode, NULL, S_IFDIR);
	root_inode->i_sb = sb;
	root_inode->i_op = &simplefs_inode_ops;
	root_inode->i_fop = &simplefs_dir_operations;
	root_inode->i_atime = root_inode->i_mtime = root_inode->i_ctime = current_time(root_inode);
	root_inode->i_private = simplefs_get_inode(sb, SIMPLEFS_ROOTDIR_INODE_NUMBER);

	sb->s_root = d_make_root(root_inode);

	return 0;
}

static struct dentry *simplefs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data)
{
	return mount_bdev(fs_type, flags, dev_name, data, simplefs_fill_super);
}

static void simplefs_kill_superblock(struct super_block *sb)
{
	kill_block_super(sb);
}

struct file_system_type simplefs_fs_type = {
	.owner = THIS_MODULE,
	.name = "simplefs",
	.mount = simplefs_mount,
	.kill_sb = simplefs_kill_superblock,
	.fs_flags = FS_REQUIRES_DEV,
};

static int simplefs_init(void)
{
	sfs_inode_cachep = kmem_cache_create("sfs_inode_cache", sizeof(struct simplefs_inode), 0, (SLAB_RECLAIM_ACCOUNT| SLAB_MEM_SPREAD), NULL);
	return register_filesystem(&simplefs_fs_type);
}

static void simplefs_exit(void)
{
	unregister_filesystem(&simplefs_fs_type);
	kmem_cache_destroy(sfs_inode_cachep);
}

module_init(simplefs_init);
module_exit(simplefs_exit);

MODULE_LICENSE("GPL");