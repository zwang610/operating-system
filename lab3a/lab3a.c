#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#include "ext2_fs.h"

int dfd;
struct ext2_super_block superblock;

__u32 block_size = 0;

void get_time(time_t timer, char *buffer){
    time_t t = timer;
    struct tm time = *gmtime(&t);
    strftime(buffer, 25, "%m/%d/%y %H:%M:%S", &time);
}

void do_dirblock(char * block_buffer, __u32 inode_num, unsigned int i){
    unsigned int x = 0;//
    struct ext2_dir_entry *entry = (struct ext2_dir_entry *) block_buffer;
    while ((x + 4 < block_size) && entry->inode) {
        char *name = malloc(entry->name_len + 1);
        if (name == NULL){
            fprintf(stderr,"memory allocation error\n");
            exit(2);
        }
        memcpy(name, &entry->name, entry->name_len);
        name[entry->name_len] = 0;
        fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,'%s'\n",
            inode_num,
            i * block_size + x,
            entry->inode,
            entry->rec_len,
            entry->name_len,//
            name);
        x += entry->rec_len;
        entry = (struct ext2_dir_entry *) (block_buffer + x);
        free(name);
    }
}
void do_indirblock(char *block_buffer, __u32 b_num, int level, __u32 owner_inode, __u32 offset, int d_flag) {
	if (level == 0) { 
		return;
	}
	__u32 counter = 0; 
	__u32 *block_num = (__u32*) block_buffer;
	char *sub_block = malloc(block_size); 
	if (sub_block == NULL) {
		fprintf(stderr, "memory allocation error");
        exit(2);
	}
	
	while (((char *)block_num <= block_buffer + block_size - 4) ) {
		if (*block_num != 0) {
			fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n",
					owner_inode,
					level,
					offset + counter,
					b_num,
					*block_num);

			pread(dfd, sub_block, block_size, *block_num * block_size);
			do_indirblock(sub_block, *block_num, level - 1, owner_inode, offset + counter, d_flag);
			if (d_flag && level == 1) {
				do_dirblock(sub_block, owner_inode, offset + counter);
			}
		}
		if (level == 1) {
			counter++;
		} else if (level == 2) {
			counter += block_size / 4;
		} else {
			counter += block_size * block_size / 16;
		}
		block_num++;
	}
	free(sub_block);
}


void do_inode(__u32 inode_offset, __u32 inode_num){
    //I-node summary
    struct ext2_inode inode;
    pread(dfd, &inode, sizeof(struct ext2_inode),inode_offset);
    if (inode.i_mode == 0 || inode.i_links_count == 0) return;

    char filetype = '?';
    if (S_ISREG(inode.i_mode)) filetype = 'f';
	else if (S_ISDIR(inode.i_mode)) filetype = 'd';
	else if (S_ISLNK(inode.i_mode)) filetype = 's';

    __u16 filemode = inode.i_mode;
    filemode <<= 4;
    filemode >>= 4;
    char ctime[25], mtime[25], atime[25];
    get_time((time_t)inode.i_ctime, ctime);
    get_time((time_t)inode.i_mtime, mtime);
    get_time((time_t)inode.i_atime, atime);

    fprintf(stdout, "INODE,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d",
        inode_num,
        filetype,
        filemode,
        inode.i_uid,
        inode.i_gid,
        inode.i_links_count,
        ctime,
        mtime,
        atime,
        inode.i_size,
        inode.i_blocks);
    unsigned int i;
    for (i =0; i < 15; i++) fprintf (stdout, ",%d", inode.i_block[i]);
    fprintf(stdout, "\n");    
    
    //directory entries
    int d_flag = 0;
    
    
    char *block_buffer = malloc(block_size);
    if(block_buffer == NULL){
        fprintf(stderr, "memory allocation error\n");
        exit(2);
    }
    if (filetype == 'd'){
        d_flag = 1;
        for (i = 0; i < 12; i++){
            if(inode.i_block[i] == 0) break;
            pread(dfd, block_buffer, block_size, block_size * inode.i_block[i]);
            do_dirblock(block_buffer, inode_num, i);
        }
    }

    if (inode.i_block[12]!=0){
        pread(dfd, block_buffer, block_size, block_size * inode.i_block[12]);
        do_indirblock(block_buffer, inode.i_block[12], 1, inode_num,12,d_flag);
    }
    if (inode.i_block[13]!=0){
        pread(dfd, block_buffer, block_size, block_size * inode.i_block[13]);
        do_indirblock(block_buffer, inode.i_block[13], 2, inode_num, 12 + block_size / 4, d_flag);
    }
    if (inode.i_block[14]!=0){
        pread(dfd, block_buffer, block_size, block_size * inode.i_block[14]);
        do_indirblock(block_buffer, inode.i_block[14], 3, inode_num, 12 + block_size / 4 + block_size * block_size / 16, d_flag);
    }
}

int main(int argc, const char *argv[]){
    unsigned int i, j, k;
    if (argc != 2){
        fprintf(stderr, "invalid argument\n");
        exit(1);
    }

    const char *dname = argv[1];
    dfd = open(dname, O_RDONLY);
    if (dfd < 0){
        fprintf(stderr, "cannot open disk\n");
        exit(1);
    }

    //process superblock
    block_size = 1024 << superblock.s_log_block_size;
    pread(dfd, &superblock, sizeof(superblock), 1024);
    fprintf(stdout, "SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n",
        superblock.s_blocks_count, 
        superblock.s_inodes_count, 
        block_size,
        superblock.s_inode_size, 
        superblock.s_blocks_per_group, 
        superblock.s_inodes_per_group,
        superblock.s_first_ino);

    //process group
    __u32 ngroups = (superblock.s_blocks_count-1) / superblock.s_blocks_per_group + 1;
    char *block_bitmap = malloc(block_size);
    if (block_bitmap == NULL) {
        fprintf(stderr, "memory allocate error");
        exit(2);
    }
    char *inode_bitmap = malloc(block_size);
    if (inode_bitmap == NULL) {
        fprintf(stderr, "memory allocate error");
        exit(2);
    }
    for (j = 0; j < ngroups ; j++){
        //group summary
        struct ext2_group_desc group;
        pread(dfd, &group, sizeof(group),2048+j*sizeof(group));
        __u32 block_count = (j == ngroups - 1) ? (superblock.s_blocks_count - j * superblock.s_blocks_per_group) : superblock.s_blocks_per_group;
        __u32 inode_count = (j == ngroups - 1) ? (superblock.s_inodes_count - j * superblock.s_inodes_per_group) : superblock.s_inodes_per_group;
        fprintf(stdout, "GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n",
            j,
            block_count,
            inode_count,
            group.bg_free_blocks_count,
            group.bg_free_inodes_count,
            group.bg_block_bitmap,
            group.bg_inode_bitmap,
            group.bg_inode_table);     

        //free block
        pread(dfd, block_bitmap, block_size, group.bg_block_bitmap * block_size);
        __u32 block_cnt = 1;
        for (i = 0; i < block_size; i++){
            if (block_cnt > block_count) break;
            __u32 bit = 1;
            for (k = 0; k < 8; k++){
                if (block_cnt > block_count) break;
                if (!(bit & block_bitmap[i])) fprintf(stdout, "BFREE,%d\n", j * superblock.s_blocks_per_group + block_cnt);
                bit = bit << 1;
                block_cnt++;
            }
        }
        //free inode
        pread(dfd, inode_bitmap, block_size, group.bg_inode_bitmap * block_size);
        __u32 inode_cnt = 1;
        for (i = 0; i < block_size; i++){
            if(inode_cnt > inode_count) break;
            __u32 bit = 1;
            for (k = 0; k < 8; k++){
                if(inode_cnt > inode_count) break;
                if(!(bit & inode_bitmap[i])) fprintf(stdout, "IFREE,%d\n", j * superblock.s_inodes_per_group + inode_cnt);
                else{
                    do_inode(group.bg_inode_table * block_size + sizeof(struct ext2_inode) * (inode_cnt - 1), inode_cnt + superblock.s_inodes_per_group * j);
                }   
                bit = bit << 1;
                inode_cnt++;
            }
        }
        

    }
    free(block_bitmap);
    free(inode_bitmap);
    close(dfd);
}