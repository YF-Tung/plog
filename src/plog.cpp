#include "plog.h"

namespace plog {

PipeText::PipeText() {}

int PipeText::process(std::istream& is, std::ostream& os) {
    m_flushed = false;
    m_last_output_time = std::chrono::steady_clock::now();
    char c;

    std::thread thr(&PipeText::flush_if_idle, this);

    // Blocking read
    while (is.get(c)) write_to_output(c, os);
    thr.join();
    return 0;
}
void PipeText::set_truncate_line(bool b) { truncate_line = b; }

void PipeText::write_to_output(char c, std::ostream& os) {
    static const int TRUNCATE_EXTRA = 5;
    static const int MIN_PRINT_LEN = 5;
    if (!truncate_line) {
        std::lock_guard<std::mutex> guard(mutex_);
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
            std::lock_guard<std::mutex> guard(mutex_);
            if (to_skip > 0) os << "... ";
            for (int i = to_skip; i < buffer.size(); i++) os << buffer[i];
            buffer.clear();
        }
    }
    m_flushed = false;
    m_last_output_time = std::chrono::steady_clock::now();
}

void PipeText::flush_if_idle() {
    while (true) {
        bool flushed;
        std::chrono::time_point<std::chrono::steady_clock> last_output_time;
        {
            std::lock_guard<std::mutex> guard(mutex_);
            flushed = m_flushed;
            last_output_time = m_last_output_time;
        }
        if (flushed) std::this_thread::sleep_for(timeout);
        else {
            auto time_diff = std::chrono::steady_clock::now() - last_output_time;
            if (time_diff > timeout) flush_page();
            else std::this_thread::sleep_for(timeout - time_diff);
        }
    }
}

// Already locked
void PipeText::flush_page() {
    int nrow_to_append = get_page_height();
    for (int i = 0; i < nrow_to_append; i++) std::cout<<std::endl;
    std::lock_guard<std::mutex> guard(mutex_);
    m_flushed = true;
}

int PipeText::get_page_height() const {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row;
}

int PipeText::get_page_width() const {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

// Static variable
const std::chrono::seconds PipeText::timeout = std::chrono::seconds(60);

} // namespace plog

