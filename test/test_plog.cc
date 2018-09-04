#include "plog.h"
#include "gtest/gtest.h"
namespace {

class FastPipeText : public plog::PipeText {
public:
    FastPipeText() {
        timeout = std::chrono::milliseconds(10);
    }
};

TEST(SampleTestCase, SampleTest) {
    EXPECT_EQ(1 + 1, 2);
}

TEST(TestPlog, Process) {
    FastPipeText fastPipeText;
    const int txt_len = 1000;
    std::string txt;
    for (int i = 0; i < txt_len; i++) txt += 'a';
    txt += '\n';
    std::istringstream iss(txt);
    std::stringstream oss;
    fastPipeText.process(iss, oss);
    EXPECT_EQ(oss.str(), txt);
}

TEST(TestPlog, SetTruncateLine) {
    FastPipeText fastPipeText;
    fastPipeText.set_truncate_line(true);
    const int txt_len = 1000;
    std::string txt;
    for (int i = 0; i < txt_len; i++) txt += 'a';
    txt += '\n';
    std::istringstream iss(txt);
    std::stringstream oss;
    fastPipeText.process(iss, oss);
    EXPECT_NE(oss.str(), txt);
    EXPECT_EQ(oss.str().substr(0, 4), "... ");
}

}

