#pragma once

#include "imports.hpp"

#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <vector>

namespace tiny_pointers {

/** \brief Parses the given file, producing a list of tokens in lowercase containing only alphanumeric
 * characters, which are appended to the passed vector.
 */
inline void load_words(std::filesystem::path file_path, std::vector<std::string>& words)
{
    std::ifstream ifs{file_path};
    BATT_CHECK(ifs.good());

    while (ifs.good()) {
        std::string w;
        ifs >> w;

        boost::algorithm::to_lower(w);
        w.erase(std::remove_if(w.begin(), w.end(),
                               [](char ch) {
                                   return !std::isalnum(ch);
                               }),
                w.end());

        if (!w.empty()) {
            words.emplace_back(std::move(w));
        }
    }
}

/** \brief Parses the given file, returning a list of tokens in lowercase containing only alphanumeric
 * characters.
 */
inline std::vector<std::string> load_words(std::filesystem::path file_path)
{
    std::vector<std::string> words;

    load_words(file_path, words);

    return words;
}

/** \brief Same as load_words, but path is relative to this project's git repo root dir.
 */
inline std::vector<std::string> load_words_rel(std::filesystem::path rel_file_path)
{
    return load_words(std::filesystem::path{__FILE__}.parent_path().parent_path().parent_path() /
                      rel_file_path);
}

/** \brief Same as load_words, but path is relative to this project's git repo root dir.
 */
inline void load_words_rel(std::filesystem::path rel_file_path, std::vector<std::string>& words)
{
    return load_words(
        std::filesystem::path{__FILE__}.parent_path().parent_path().parent_path() / rel_file_path, words);
}

/** \brief Returns a random key composed of `word_count` dictionary words.
 */
inline std::string random_key(std::default_random_engine& rng, usize word_count = 2)
{
    static std::vector<std::string> words =
        tiny_pointers::load_words_rel(std::filesystem::path{"data"} / "words");
    static std::uniform_int_distribution<usize> pick_word{0, words.size() - 1};

    std::ostringstream oss;
    bool first = true;

    for (usize i = 0; i < word_count; ++i) {
        if (!first) {
            oss << " ";
        } else {
            first = false;
        }
        oss << words[pick_word(rng)];
    }

    return std::move(oss).str();
}

}  //namespace tiny_pointers
