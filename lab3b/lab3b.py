#!/usr/bin/python
import sys
import csv

class Superblock:
    def __init__(self, sbdata):
        self.sbdata = dict(
            blocks_count = int(sbdata[1]),
            inodes_count = int(sbdata[2]), 
            block_size = int(sbdata[3]), 
            inode_size = int(sbdata[4]), 
            blocks_per_group = int(sbdata[5]), 
            inodes_per_group = int(sbdata[6]), 
            first_ino = int(sbdata[7])
        )

class Group:
    def __init__(self,gdata):
        self.gdata = dict(
            group_num = int(gdata[1]),
            block_counts = int(gdata[2]),
            inode_counts = int(gdata[3]),
            free_blocks = int(gdata[4]),
            free_inodes = int(gdata[5]),
            block_bitmap = int(gdata[6]),
            inode_bitmap = int(gdata[7]),
            inode_table = int(gdata[8])
        )

class Inode:
    def __init__(self, indata):
        self.indata = dict(
            inode_num = int(indata[1]),
            filetype = indata[2],
            filemode = indata[3], 
            uid = int(indata[4]),
            gid = int(indata[5]),
            links_count = int(indata[6]),
            ctime = indata[7],
            mtime = indata[8],
            atime = indata[9],
            size = int(indata[10]),
            blocks = int(indata[11]),
            direct_blocks = [int(x) for x in indata[12:24]],
            indirect1_block = int(indata[24]),
            indirect2_block = int(indata[25]),
            indirect3_block = int(indata[26])
        )

class Dirent:
    def __init__(self, dirdata):
        self.dirdata = dict(
            inode_num = int(dirdata[1]),
            offset = int(dirdata[2]),
            entry_inode = int(dirdata[3]),
            entry_length = int(dirdata[4]),
            name_length = int(dirdata[5]),
            name = dirdata[6].rstrip()
        )

class IndirectReference:
    def __init__(self, indirdata):
        self.indirdata = dict(
            owner_inode = int(indirdata[1]),
            level = int(indirdata[2]),
            offset = int(indirdata[3]),
            block_num = int(indirdata[4]),
            block_num_ptr = int(indirdata[5])
        ) 


super_block = ''
group_list = []
bfree_list = []
ifree_list = []
inode_list = []
dir_list = []
indir_list = []

try: 
    inputf = open(sys.argv[1], "r")
except: 
    sys.stderr.write('cannot open file\n')
    sys.exit(1)

data = csv.reader(inputf)
for line in data:
    if line[0] == 'SUPERBLOCK':
        super_block = Superblock(line)
    elif line[0] == 'GROUP':
        group_list.append(Group(line))
    elif line[0] == 'BFREE':
        bfree_list.append(int(line[1]))
    elif line[0] == 'IFREE':
        ifree_list.append(int(line[1]))
    elif line[0] == 'INODE':
        inode_list.append(Inode(line))
    elif line[0] == 'DIRENT':
        dir_list.append(Dirent(line))
    elif line[0] == 'INDIRECT':
        indir_list.append(IndirectReference(line))

error_flag = False

block_list = set()
block_entry = {} 
if super_block.sbdata['block_size'] > 1024:
    super_block_id = 0
else:
    super_block_id = 1

inode_table_size = int((super_block.sbdata['inodes_count'] * super_block.sbdata['inode_size'] - 1 ) / super_block.sbdata['block_size']) +1

#Block Consistency Audits
def print_blockptr(blocktype, block_num, inode_num, offset):
    if block_num not in range (super_block.sbdata['blocks_count']):
        print("INVALID {} {} IN INODE {} AT OFFSET {}".format(blocktype, block_num, inode_num, offset))
        error_flag = True
    elif block_num < (4 + inode_table_size + super_block_id) and block_num != 0:
        print("RESERVED {} {} IN INODE {} AT OFFSET {}".format(blocktype, block_num, inode_num, offset))  
        error_flag = True


inodelink_arr = {}
inode_index = []
for inode in inode_list:
    inode_index.append(inode.indata['inode_num'])
    inodelink_arr[inode.indata['inode_num']] = inode.indata['links_count']
    offset = 0
    inode_num = inode.indata['inode_num']
    for block_num in inode.indata['direct_blocks']:
        if block_num == 0: 
            continue
        if block_num not in block_entry:
            block_entry[block_num] = [(inode_num, offset, 'BLOCK')]
        elif block_num in block_entry:
            block_entry[block_num].append((inode_num, offset,'BLOCK'))
        block_list.add(block_num)
        if not ((inode.indata['filetype'] == 's') and (inode.indata['size'] <= 60)):
            print_blockptr('BLOCK', block_num, inode_num, offset)
        offset += 1
    if not ((inode.indata['filetype'] == 's') and (inode.indata['size'] <= 60)):
        print_blockptr('INDIRECT BLOCK',inode.indata['indirect1_block'], inode_num, 12)
        print_blockptr('DOUBLE INDIRECT BLOCK',inode.indata['indirect2_block'], inode_num, 268)
        print_blockptr('TRIPLE INDIRECT BLOCK',inode.indata['indirect3_block'], inode_num, 65804)

    if ((inode.indata['indirect1_block'] != 0) and (inode.indata['indirect1_block'] not in block_entry)):
        block_entry[inode.indata['indirect1_block']] = [(inode_num, 12, 'INDIRECT BLOCK')]
    elif ((inode.indata['indirect1_block'] != 0) and (inode.indata['indirect1_block'] in block_entry)):
        block_entry[inode.indata['indirect1_block']].append((inode_num, 12, 'INDIRECT BLOCK'))
    if ((inode.indata['indirect2_block'] != 0) and (inode.indata['indirect2_block'] not in block_entry)):
        block_entry[inode.indata['indirect2_block']] = [(inode_num, 268, 'DOUBLE INDIRECT BLOCK')]
    elif ((inode.indata['indirect2_block'] != 0) and (inode.indata['indirect2_block'] in block_entry)):
        block_entry[inode.indata['indirect2_block']].append((inode_num, 268, 'DOUBLE INDIRECT BLOCK'))
    if ((inode.indata['indirect3_block'] != 0) and (inode.indata['indirect3_block'] not in block_entry)):
        block_entry[inode.indata['indirect3_block']] = [(inode_num, 65804, 'TRIPLE INDIRECT BLOCK')]
    elif ((inode.indata['indirect3_block'] != 0) and (inode.indata['indirect3_block'] in block_entry)):
        block_entry[inode.indata['indirect3_block']].append((inode_num, 65804, 'TRIPLE INDIRECT BLOCK'))

for indirent in indir_list:
    level = indirent.indirdata['level']
    b_num = indirent.indirdata['block_num_ptr']
    i_num = indirent.indirdata['block_num']
    offset = indirent.indirdata['offset']
    block_list.add(b_num)
    block_list.add(i_num)
    blocktype = ''
    if level == 1:
        blocktype = 'INDIRECT BLOCK'
    elif level == 2:
        blocktype = 'DOUBLE INDIRECT BLOCK'
    elif level == 3:
        blocktype = 'Triple INDIRECT BLOCK'

    if b_num not in block_entry:
        block_entry[b_num] = [(i_num, offset, blocktype)]
    print_blockptr(blocktype, b_num, i_num, offset)

for j in range(1,super_block.sbdata['blocks_count']):
    if j > (3 + inode_table_size + super_block_id):
        if (j in bfree_list) and (j in block_list):
            print("ALLOCATED BLOCK {} ON FREELIST".format(j))
            error_flag = True
        if (j not in bfree_list) and (j not in block_list):
            print("UNREFERENCED BLOCK {}".format(j))
            error_flag = True

for (block_num, entry) in block_entry.items():
    if len(entry) > 1:
        for (inode, offset, blocktype) in entry:
            print('DUPLICATE {} {} IN INODE {} AT OFFSET {}'.format(blocktype, block_num, inode, offset))
            error_flag = True
    

#I-node Allocation Audits
for i in range(super_block.sbdata['inodes_count']):
    if (i in inode_index) and (i in ifree_list):
        print('ALLOCATED INODE {} ON FREELIST'.format(i))
    elif (i not in inode_index) and (i not in ifree_list) and (i > super_block.sbdata['first_ino'] - 1):
        print('UNALLOCATED INODE {} NOT ON FREELIST'.format(i))


#Directory Consistency Audits
dirlink_arr = {}
parent_of_child = {}

for d in dir_list:
    child = d.dirdata['entry_inode']
    parent = d.dirdata['inode_num']
    name = d.dirdata['name']
    if (name != "'.'") and (name != "'..'"):
        parent_of_child[child] = parent
parent_of_child[2] = 2

for dirent in dir_list:
    child = dirent.dirdata['entry_inode']
    parent = dirent.dirdata['inode_num']
    name = dirent.dirdata['name']
    if (child not in range (1, super_block.sbdata['inodes_count'])):
        print('DIRECTORY INODE {} NAME {} INVALID INODE {}'.format(parent,name,child))
        error_flag = True
    elif child not in inode_index:
        print('DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}'.format(parent,name,child))
        error_flag = True
    elif child not in dirlink_arr:
        dirlink_arr[child] = 1
    elif child in dirlink_arr:
        dirlink_arr[child] += 1

    if name == "'.'" and parent != child:
        print("DIRECTORY INODE {} NAME '.' LINK TO INODE {} SHOULD BE {}".format(parent,child,parent))
        error_flag = True
    elif name == "'..'" and child != parent_of_child[parent]:
        print("DIRECTORY INODE {} NAME '..' LINK TO INODE {} SHOULD BE {}".format(parent,child,parent_of_child[parent]) )
        error_flag = True

for (n, c) in inodelink_arr.items():
    if c != dirlink_arr.get(n,0):
        print('INODE {} HAS {} LINKS BUT LINKCOUNT IS {}'.format(n,dirlink_arr.get(n,0),c))
        error_flag = True

if error_flag : 
    sys.exit(2)

sys.exit(0)

 
