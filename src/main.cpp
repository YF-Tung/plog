#include "plog.h"

int show_usage(char bin_name[]) {
    std::cerr << "Usage: " << bin_name << " [-t]" << std::endl;
    return 1;
}

int main(int argc, char** argv) {
    plog::PipeText pipe_text;
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
    return pipe_text.process(std::cin, std::cout);
}

