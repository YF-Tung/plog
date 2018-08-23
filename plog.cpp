#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>

// Linux & MacOS
#include <sys/ioctl.h>
#include <unistd.h>
#include <getopt.h>

using namespace std;

class PipeText {
public:
    PipeText() : status(-1) {}

    int process(istream& is, ostream& os) {
        char c;

        thread thr(&PipeText::flush_if_idle, this);

        // Blocking read
        while (is.get(c)) {
            write_to_output(c, os);
            status = 0;
        }

        {
            lock_guard<mutex> guard(mutex_);
            status = -2;
        }
        thr.join();
        return 0;
    }
    void set_truncate_line(bool b) { truncate_line = b; }
private:
    bool truncate_line = false;
    mutex mutex_;
    static const int timeout;
    vector<char> buffer;

    /* int status
     * -2 : end of process
     * -1 : already flushed and no new data input
     * 0+ : seconds since last new data input
     */
    int status;

    void write_to_output(char c, ostream& os) {
        static const int TRUNCATE_EXTRA = 5;
        static const int MIN_PRINT_LEN = 5;
        if (!truncate_line) {
            lock_guard<mutex> guard(mutex_);
            os << c;
        } else {
            buffer.push_back(c);

            // TODO: unicode issue? Dos issue?
            if (c == '\n') {
                int pw = get_page_width();
                int to_skip = 0;
                if ( pw < buffer.size()) {
                    to_skip = buffer.size() - pw + TRUNCATE_EXTRA;
                    if (to_skip > buffer.size() - MIN_PRINT_LEN)
                        to_skip = buffer.size() - MIN_PRINT_LEN;
                }
                lock_guard<mutex> guard(mutex_);
                if (to_skip > 0) os << "... ";
                for (int i = to_skip; i < buffer.size(); i++) os << buffer[i];
                buffer.clear();
            }
        }
    }
    
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
        int nrow_to_append = get_page_height();
        for (int i = 0; i < nrow_to_append; i++) cout<<endl;
    }

    int get_page_height() const {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        return w.ws_row;
    }

    int get_page_width() const {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        return w.ws_col;
    }
};

// Static variable
const int PipeText::timeout = 60;


int show_usage(char bin_name[]) {
    cerr << "Usage: " << bin_name << " [-t]" << endl;
    return 1;
}

int main(int argc, char** argv) {
    PipeText pipe_text;
    int opt;
    while ((opt = getopt(argc, argv, "t")) != -1) {
        switch (opt) {
            case 't':
                pipe_text.set_truncate_line(true);
                break;
            default: /* '?' */
                return show_usage(argv[0]);
        }
    }
    return pipe_text.process(cin, cout);
}

