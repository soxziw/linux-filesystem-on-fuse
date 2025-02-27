# linux-filesystem-on-fuse
## Project Overview
- **Goal**: Implement an ext-based file system  
- **Features**:  
  - Basic file manipulation via Linux commands (`cd`, `cat`, `touch`, `echo`, `grep`, `awk`, `sed`, `ls`)  
  - POSIX interface (`open`, `close`, `read`, `write`, `lseek`, `truncate`, `fstat`, `mount`, `unmount`)
  - Symbolic links operations (`symlink`, `readlink`, `link`, `unlink`)
  - Permissions (`chmod`, `chown`)
  - Compilers and application (`gcc`, `g++`, `gfortran`, `make`, `cmake`, `git`, `vi`)

## Software Architecture
![fs-arch](https://drive.usercontent.google.com/download?id=155CPURrF4GTFP4Dv3EWw4PkhmV_eNozC)

This pyramid diagram depicts the software stack for a FUSE-based filesystem. The bottom layers (C++ & Clang, CMake & Make) represent the core development and build tools. The filesystem implementation is structured into four layers (Layer0 to Layer3), with FUSE at the top to handle file system operations. Google Test is used for both unit tests (Layer0, Layer1) and integration tests (Layer2, Layer3). GitHub (CI/CD) is used for version control and continuous integration of the project.


## Implementation

### Layer 0
- Disk interface layer  
- Read/write directly to `/dev/vdb`  
- Read/write in 4K blocks  
- **Added feature**: Buffer cache with **LRU policy**  

### Layer 1 (DS Layer)
![storage-partition](https://drive.usercontent.google.com/download?id=1ev_FWpiAO0df3pGodo3gP-oKlwNVPdea)

This is the layout of a storage partition on a disk. The superblock (block 0) contains metadata about the file system. The i-list (blocks 1-5) stores inodes, which contain file metadata and pointers to data blocks. The data blocks (starting at block 7 and beyond) store the actual file content. The entire partition is a contiguous set of blocks, each 4KB in size, forming a structured way to manage storage.

![free-list](https://drive.usercontent.google.com/download?id=1v8XBQTyS2cvoo4JvXjOl6IlL_FJ0gtjN)

This  illustrates how free blocks are managed in a hierarchical structure. The superblock (101) points to a first-level block (614) that contains addresses of additional free blocks. These free blocks, in turn, store addresses of more free blocks, creating a multi-level indexing system. Each block is 4KB in size and contains 512 block address entries, forming a layered approach to managing available storage.

### Layer 2/3
- Layer 2 and 3 are not mutually exclusive  
- **Layer 2**:  
  - System calls  
  - Better debugging  
- **Layer 3**:  
  - FUSE operations  
  - Symlinks (FUSE handles loop circumstances for us)  
  - Permissions  

---

## Permission Support
- **Partially supported in Layer 3**  
  - `mkdir`, `rmdir`, `write`, `ls` (directory execution and read)  
  - `ln` (file link execution and read)
 
## Build
Build the project and run unit tests. (Exec `./build/fuseTests` for unit tests)
```bash
make build
```

Build the project, and run unit and E2E/integration tests. (Exec `./build/fuseTests` for unit tests and `./build/fuseIntegration` for integration tests)
```bash
make release
```

Clean build results. (Delete `./build/` directory)
```bash
make clean
```

## E2E / Integration tests

> My tests will use the Linux POSIX interface and I will expect your file system to generate the same results when a test runs on a Linux file system and on your file system. Thus you should consider writing as many (and as varied) tests using the POSIX interface and comparing the tests when run on a native Linux file system to one run on yours.
> 
> I will also test your file system using "standard" Linux system utilities and tools. Examples include, but are not limited to, the various language compilers (gcc, g++, gfortran, etc.), git, make, bash, grep, awk, sed, ls, and find. You should consider writing tests that use these utilities to access files on your file system as well. At this point you should also write stress tests that do lots of operations with different sizes and offsets to make sure that your file system doesn't have a latent bug or two.
> 
> -Prof. Rich Wolski

### Command line
```bash
// Compilers
gcc / g++ / gfortran / ...

// Access file and dir
mkdir
rm / rm -f
touch
test
echo >
git
make
grep
awk
sed
ls
find
```

### POSIX interface
```c
// File descriptor operations
int open(const char *pathname, int flags);
int close(int fd);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
off_t lseek(int fd, off_t offset, int whence);
int fsync(int fd);
int ftruncate(int fd, off_t length);

// File status and attributes
int stat(const char *pathname, struct stat *statbuf);
int fstat(int fd, struct stat *statbuf);
int lstat(const char *pathname, struct stat *statbuf);
int chmod(const char *pathname, mode_t mode);
int chown(const char *pathname, uid_t owner, gid_t group);
mode_t umask(mode_t mask);

// Directory operations
DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);
int mkdir(const char *pathname, mode_t mode);
int rmdir(const char *pathname);

// Symbolic links and file system operations
int symlink(const char *target, const char *linkpath);
ssize_t readlink(const char *pathname, char *buf, size_t bufsiz);
int link(const char *oldpath, const char *newpath);
int unlink(const char *pathname);
int rename(const char *oldpath, const char *newpath);

// File system mount and unmount
int mount(const char *source, const char *target, const char *filesystemtype,
          unsigned long mountflags, const void *data);
int umount(const char *target);

// File system information
int statfs(const char *path, struct statfs *buf);
int fstatfs(int fd, struct statfs *buf);

// Disk/device operations
int ioctl(int fd, unsigned long request, ...);
```
