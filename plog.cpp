#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>
#include <atomic>

// Linux & MacOS
#include <sys/ioctl.h>
#include <unistd.h>

using namespace std;

class PipeText {
public:
    PipeText() : status(-1) {}

    int process(istream& is, ostream& os) {
        char c;

        thread thr(&PipeText::flush_if_idle, this);

        // Blocking read
        while (is.get(c)) {
            lock_guard<mutex> guard(mutex_);
            os << c;
            status = 0;
        }

        {
            lock_guard<mutex> guard(mutex_);
            status = -2;
        }
        thr.join();
        return 0;
    }
private:
    mutex mutex_;
    static const int timeout;

    /* int status
     * -2 : end of process
     * -1 : already flushed and no new data input
     * 0+ : seconds since last new data input
     */
    int status;
    
    void flush_if_idle() {
        while (true) {
            {
                lock_guard<mutex> guard(mutex_);
                //unsigned val = status.load();
                if (status == -2) break;
                if (status != -1) {
                    status++;
                    if (status >= timeout) {
                        status = -1;
                        flush_page();
                    }
                }
            }
            this_thread::sleep_for(chrono::milliseconds(1000));
        }
    }

    // Already locked
    void flush_page() {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        int nrow_to_append = w.ws_row;
        for (int i = 0; i < nrow_to_append; i++) cout<<endl;
    }
};

// Static variable
const int PipeText::timeout = 60;

int main(int argc, char** argv) {
    PipeText pipe_text;
    return pipe_text.process(cin, cout);
}

