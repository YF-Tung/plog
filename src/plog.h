#ifndef PLOG_H_
#define PLOG_H_
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

namespace plog {

class PipeText {
public:
    PipeText();

    int process(std::istream&, std::ostream&);

    void set_truncate_line(bool b);

protected:
    bool truncate_line = false;
    std::mutex mutex_;
    std::chrono::milliseconds timeout = std::chrono::milliseconds(60000);
    std::vector<char> buffer;
    std::ostream* m_pos = nullptr;
    bool m_stop = false;

    std::chrono::time_point<std::chrono::steady_clock> m_last_output_time;
    bool m_flushed;

    void write_to_output(char c); 
    void flush_if_idle();

    // Already locked
    void flush_page();

    int get_page_height() const;

    int get_page_width() const;
};

} // namespace plog
#endif  //PLOG_H_

