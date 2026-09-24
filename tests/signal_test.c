#include <stdio.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t running = 1;

void handle_sigint(int signal)
{
    (void)signal;

    running = 0;
}

int main(void)
{
    signal(SIGINT, handle_sigint);
    signal(SIGTERM, handle_sigint);

    printf("Process started.\n");
    printf("PID: %d\n", getpid());
    printf("Press Ctrl+C to stop.\n");

    while (running)
    {
        printf("Process is running...\n");
        sleep(2);
    }

    printf("Shutdown requested.\n");
    printf("Process exiting gracefully.\n");

    return 0;
}