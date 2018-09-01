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
    PipeText() {}

    int process(istream& is, ostream& os) {
        m_flushed = false;
        m_last_output_time = chrono::steady_clock::now();
        char c;

        thread thr(&PipeText::flush_if_idle, this);

        // Blocking read
        while (is.get(c)) write_to_output(c, os);
        thr.join();
        return 0;
    }
    void set_truncate_line(bool b) { truncate_line = b; }
private:
    bool truncate_line = false;
    mutex mutex_;
    static const chrono::seconds timeout;
    vector<char> buffer;

    chrono::time_point<chrono::steady_clock> m_last_output_time;
    bool m_flushed;

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
        m_flushed = false;
        m_last_output_time = chrono::steady_clock::now();
    }
    
    void flush_if_idle() {
        while (true) {
            bool flushed;
            chrono::time_point<chrono::steady_clock> last_output_time;
            {
                lock_guard<mutex> guard(mutex_);
                flushed = m_flushed;
                last_output_time = m_last_output_time;
            }
            if (flushed) this_thread::sleep_for(timeout);
            else {
                auto time_diff = chrono::steady_clock::now() - last_output_time;
                if (time_diff > timeout) flush_page();
                else this_thread::sleep_for(timeout - time_diff);
            }
        }
    }

    // Already locked
    void flush_page() {
        int nrow_to_append = get_page_height();
        for (int i = 0; i < nrow_to_append; i++) cout<<endl;
        lock_guard<mutex> guard(mutex_);
        m_flushed = true;
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
const chrono::seconds PipeText::timeout = chrono::seconds(60);


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

