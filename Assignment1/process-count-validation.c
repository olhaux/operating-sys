#include <unistd.h>

int main() {
    int N = 3;
    for (int i = 0; i < N; i++) {
        fork();
        fork();
    }
    sleep(30);
}