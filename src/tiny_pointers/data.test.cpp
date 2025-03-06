#include <tiny_pointers/data.hpp>
//
#include <tiny_pointers/data.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <unordered_set>

namespace {

TEST(DataTest, Load)
{
    auto data_dir = std::filesystem::path{"data"};

    std::vector<std::string> words;

    tiny_pointers::load_words_rel(data_dir / "words", words);
    tiny_pointers::load_words_rel(data_dir / "calgary" / "bib", words);
    tiny_pointers::load_words_rel(data_dir / "calgary" / "book1", words);
    tiny_pointers::load_words_rel(data_dir / "calgary" / "book2", words);
    tiny_pointers::load_words_rel(data_dir / "calgary" / "news", words);

    std::unordered_set<std::string_view> word_set{words.begin(), words.end()};

    EXPECT_GT(word_set.size(), 0);
    EXPECT_GT(words.size(), word_set.size());

    std::cerr << BATT_INSPECT(words.size()) << BATT_INSPECT(word_set.size()) << std::endl;
}

}  //namespace
