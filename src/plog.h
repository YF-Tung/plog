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
    PipeText();

    int process(istream&, ostream&);

    void set_truncate_line(bool b);

private:
    bool truncate_line = false;
    mutex mutex_;
    static const chrono::seconds timeout;
    vector<char> buffer;

    chrono::time_point<chrono::steady_clock> m_last_output_time;
    bool m_flushed;

    void write_to_output(char c, ostream& os); 
    void flush_if_idle();

    // Already locked
    void flush_page();

    int get_page_height() const;

    int get_page_width() const;
};

