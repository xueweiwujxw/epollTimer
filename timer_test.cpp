// #include <timer.hpp>
#include "timer.hpp"
#include <iostream>

using namespace std;
using namespace Timer;

void hello(void *args) {
    cout << "hello" << endl;
}

void d2333(void *args) {
    cout << "2333" << endl;
}

int main(int argc, char const *argv[]) {
    timerManager tm;
    tm.loopstart();
    int a;
    while (cin >> a) {
        if (a == 1) {
            time_t t = time(&t);
            tm.addTimer(t + 15, &d2333);
        } else if (a == 2) {
            time_t t = time(&t);
            tm.addTimer(t + 5, &hello);
        } else if (a == -1) {
            tm.loopbreak();
            break;
        }
    }
    return 0;
}
