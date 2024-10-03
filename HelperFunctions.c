

void preciseSleep(int seconds) {
    struct timespec req, rem;

    // Set the timespec structure with the requested sleep time
    req.tv_sec = seconds;
    req.tv_nsec = 0;  // No nanoseconds since we are working with whole seconds

    // Use clock_nanosleep with CLOCK_MONOTONIC for precision and robustness
    int ret = clock_nanosleep(CLOCK_REALTIME, 0, &req, &rem);

    if (ret != 0) {
        if (ret == EINTR) {
            // Interrupted by a signal handler, display remaining time
            printf("Sleep interrupted. Remaining: %ld seconds\n", rem.tv_sec);
        }
        else {
            // Handle other potential errors
            perror("clock_nanosleep failed");
        }
    }
}