//
//  TextWordLayout.cpp
//
//  Created by Roald Christesen on from 04.09.2026
//  Copyright (C) 2026 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 04.09.2026
//

#include "Graphic/TextWordLayout.hpp"

#include <netdb.h>


namespace Grain {

    TextWordLayout::TextWordLayout(const char* text, Font* font) noexcept {
        setFont(font);
        setText(text);
    }


    void TextWordLayout::setFont(const Font* font) noexcept {
        font_ = font;
    }


    void TextWordLayout::setText(const char* text) noexcept {
        text_ = text;
    }


    void TextWordLayout::addWord(const char* str) noexcept {
        auto word = new(std::nothrow) TextWordLayoutWord();
        if (word) {
            word->text_ = str;
            words_.push(word);
        }
    }


    void TextWordLayout::splitWords() noexcept {
        if (!text_) {
            return;
        }

        words_.clear();

        const char* word_start = text_;
        const char* p = text_;

        while (*p != '\0') {
            if (*p == ' ') {
                // End of word
                if (p > word_start) {
                    addWord(std::string(word_start, p - word_start).c_str());
                }

                ++p;
                word_start = p;
                continue;
            }

            // UTF-8 soft hyphen: C2 AD
            if (
                static_cast<unsigned char>(p[0]) == 0xC2 &&
                static_cast<unsigned char>(p[1]) == 0xAD
            ) {
                // End of word
                if (p > word_start) {
                    addWord(std::string(word_start, p - word_start).c_str());
                }

                p += 2;
                word_start = p;
                continue;
            }

            ++p;
        }

        // Last word
        if (p > word_start) {
            addWord(std::string(word_start, p - word_start).c_str());
        }

        for (auto word : words_) {
            std::cout << word->text_ << std::endl;
        }
    }


    void TextWordLayout::wordMetrics() noexcept {
        if (!font_) {
            return;
        }

        for (auto word : words_) {
            auto dimension = font_->textDimension(word->text_.utf8());
            word->rect_.width_ = dimension.width_;
            word->rect_.height_ = dimension.height_;
        }
    }


    void TextWordLayout::layout() noexcept {
        if (!text_ || !font_) {
            return;
        }

        splitWords();
        wordMetrics();

        int32_t i = 0;
        for (auto word : words_) {
            std::cout << i++ << ": " << word->text_ << ", " << word->rect_ << std::endl;
        }
    }

} // namespace Grain

