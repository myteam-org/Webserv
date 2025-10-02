class FdUtils {
    public:
        static int set_nonblock_and_cloexec(int fd);
        static void safe_fd_close(int fd);
};
