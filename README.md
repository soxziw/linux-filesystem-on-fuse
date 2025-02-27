# linux-filesystem-on-fuse

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
