#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "io/input/write/buffer.hpp"
#include "io/input/writer/writer.hpp"

//---------------------------------------------
// WriteResult 生成ヘルパ（必要なら調整）
//---------------------------------------------
#define WR_OK(n)  types::ok<std::size_t>(static_cast<std::size_t>(n))
// 失敗系も試すなら AppError の作り方に合わせてここを定義
// #define WR_ERR(e) types::err<error::AppError>(e)

//---------------------------------------------
// テスト用 Writer: 事前に決めたサイズだけ書く
//---------------------------------------------
class FakeWriter : public io::IWriter {
public:
    // plan[i] バイトを書いて返す（n より大きければ min を使う）
    explicit FakeWriter(std::vector<std::size_t> plan = {})
        : plan_(plan), idx_(0) {}

     io::IWriter::WriteResult write(const char* p, std::size_t n) {
        std::size_t w;
        if (idx_ < plan_.size()) {
            w = std::min(plan_[idx_++], n);
        } else {
            // プランが尽きたら全部書ける想定
            w = n;
        }
        written_.insert(written_.end(), p, p + w);
        return WR_OK(w);
    }

    std::string writtenAsString() const {
        return std::string(written_.begin(), written_.end());
    }

    const std::vector<char>& raw() const { return written_; }

private:
    std::vector<std::size_t> plan_;
    std::size_t idx_;
    std::vector<char> written_;
};

//---------------------------------------------
// テスト
//---------------------------------------------

TEST(WriteBufferTest, InitiallyEmpty) {
    FakeWriter fw;
    WriteBuffer buf(fw);
    EXPECT_TRUE(buf.isEmpty());
}

TEST(WriteBufferTest, AppendMakesItNonEmptyAndFlushAllClears) {
    FakeWriter fw;
    WriteBuffer buf(fw);

    std::string msg = "hello world";
    buf.append(msg);
    EXPECT_FALSE(buf.isEmpty());

    auto r = buf.flush();
    ASSERT_TRUE(r.isOk());
    EXPECT_EQ(r.unwrap(), msg.size());
    EXPECT_TRUE(buf.isEmpty());
    EXPECT_EQ(fw.writtenAsString(), msg);
}

TEST(WriteBufferTest, PartialWritesAccumulateTotalAndStopOnZero) {
    // 1回目で 3 バイト、2回目で 4 バイト、3回目 0（今は書けない）
    FakeWriter fw({3, 4, 0});
    WriteBuffer buf(fw);

    const std::string data = "ABCDEFGHIJ"; // 10 bytes
    buf.append(data);

    auto r = buf.flush();
    ASSERT_TRUE(r.isOk());
    EXPECT_EQ(r.unwrap(), 7u);         // 3 + 4 = 7
    EXPECT_FALSE(buf.isEmpty());      // まだ 3 バイト残っている

    // Writer が吐いた内容は先頭 7 バイト
    EXPECT_EQ(fw.writtenAsString(), data.substr(0, 7));
}

TEST(WriteBufferTest, MultipleFlushesDrainTheBuffer) {
    // 最初 5 バイトだけ書ける → 残りは次回フラッシュで全書き
    FakeWriter fw({5,0});
    WriteBuffer buf(fw);

    const std::string data = "abcdefghij"; // 10 bytes
    buf.append(data);

    auto r1 = buf.flush();
    ASSERT_TRUE(r1.isOk());
    EXPECT_EQ(r1.unwrap(), 5u);
    EXPECT_FALSE(buf.isEmpty());
    EXPECT_EQ(fw.writtenAsString(), data.substr(0, 5));

    auto r2 = buf.flush();
    ASSERT_TRUE(r2.isOk());
    EXPECT_EQ(r2.unwrap(), 5u);
    EXPECT_TRUE(buf.isEmpty());
    EXPECT_EQ(fw.writtenAsString(), data); // 最終的に全体一致
}

TEST(WriteBufferTest, LargeBufferWithMidwayCompactionStillWritesAll) {
    // compaction 条件: head_ > 8192 かつ head_*2 > buf_.size()
    // 例: 20,000 バイト積み、最初に 12,000 書く → 12,000*2 > 20,000 で前詰めが走る
    const std::size_t N = 20000;
    std::string big;
    big.resize(N);
    for (size_t i = 0; i < N; ++i) big[i] = static_cast<char>('A' + (i % 26));

    FakeWriter fw({12000, 0}); // 1回目 12,000、以降は全部書ける想定
    WriteBuffer buf(fw);
    buf.append(big);

    auto r1 = buf.flush();
    ASSERT_TRUE(r1.isOk());
    EXPECT_EQ(r1.unwrap(), 12000u);
    EXPECT_FALSE(buf.isEmpty()); // まだ 8,000 残り

    auto r2 = buf.flush();
    ASSERT_TRUE(r2.isOk());
    EXPECT_EQ(r2.unwrap(), 8000u);
    EXPECT_TRUE(buf.isEmpty());

    // 全データが順序通りに書かれていること
    EXPECT_EQ(fw.writtenAsString(), big);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
