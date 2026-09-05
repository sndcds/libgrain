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

#include <algorithm>
#include <cmath>


namespace Grain {

    TextWordLayout::TextWordLayout(const char* text, Font* font) noexcept {
        setFont(font);
        setText(text);
    }


    void TextWordLayout::setFont(const Font* font) noexcept {
        font_ = font;
        clearResult();
    }


    void TextWordLayout::setText(const char* text) noexcept {
        text_ = text;
        words_.clear();
        clearResult();
    }


    void TextWordLayout::addWord(const char* str) noexcept {
        clearResult();
        auto word = new(std::nothrow) TextWordLayoutWord();
        if (word) {
            word->text_ = str;
            if (!words_.push(word)) {
                delete word;
            }
        }
    }


    void TextWordLayout::splitWords() noexcept {
        words_.clear();
        clearResult();
        if (!text_) {
            return;
        }

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

            ++p;
        }

        // Last word
        if (p > word_start) {
            addWord(std::string(word_start, p - word_start).c_str());
        }
    }


    void TextWordLayout::wordMetrics() noexcept {
        clearResult();
        if (!font_) {
            return;
        }

        for (auto word : words_) {
            auto dimension = font_->textDimension(word->text_.utf8());
            word->rect_.width_ = dimension.width_;
            word->rect_.height_ = dimension.height_;
        }
    }


    void TextWordLayout::clearResult() noexcept {
        result_.lines_.clear();
        result_.width_ = 0.0;
        result_.height_ = 0.0;
        fits_ = false;
    }


    void TextWordLayout::setDimension(double width, double height) noexcept {
        dimension_.width_ = width;
        dimension_.height_ = height;
        clearResult();
    }


    void TextWordLayout::layout(double width, double height) noexcept {
        setDimension(width, height);
        layout();
    }


    int64_t TextWordLayout::wrap(double width, double space_width, double line_height,
                                bool store, double& longest) noexcept {
        longest = 0.0;
        int64_t line_count = 0;
        int64_t index = 0;
        while (index < words_.size()) {
            const auto first = index;
            double line_width = 0.0;
            while (index < words_.size()) {
                auto word = words_.elementAtIndex(index);
                const double x = index == first ? 0.0 : line_width + space_width;
                const double next_width = x + word->rect_.width_;
                // Always consume at least one word, including overwide words.
                if (index > first && next_width > width) {
                    break;
                }
                if (store) {
                    word->rect_.x_ = x;
                    word->rect_.y_ = line_count * line_height;
                }
                line_width = next_width;
                ++index;
            }
            if (store) {
                auto line = new(std::nothrow) TextWordLayoutLine();
                if (!line) {
                    return -1;
                }
                line->first_word_ = first;
                line->word_count_ = index - first;
                line->width_ = line_width;
                if (!result_.lines_.push(line)) {
                    delete line;
                    return -1;
                }
            }
            longest = std::max(longest, line_width);
            ++line_count;
        }
        return line_count;
    }


    void TextWordLayout::layout() noexcept {
        clearResult();
        if (!text_ || !font_) {
            return;
        }

        splitWords();
        if (!std::isfinite(dimension_.width_) || dimension_.width_ <= 0.0 ||
            !std::isfinite(dimension_.height_) || dimension_.height_ < 0.0) {
            return;
        }
        if (words_.size() == 0) {
            fits_ = true;
            return;
        }
        wordMetrics();
        const double space_width = font_->textDimension(" ").width_;
        double line_height = font_->lineHeight();
        if (!std::isfinite(space_width) || space_width < 0.0 ||
            !std::isfinite(line_height) || line_height < 0.0) {
            return;
        }
        for (auto word : words_) {
            if (!std::isfinite(word->rect_.width_) || word->rect_.width_ < 0.0 ||
                !std::isfinite(word->rect_.height_) || word->rect_.height_ < 0.0) {
                return;
            }
            line_height = std::max(line_height, word->rect_.height_);
        }
        if (line_height <= 0.0) {
            return; // No usable font metrics.
        }

        int64_t line_count_status = words_.size();
        double width = dimension_.width_;
        double accepted_width = width;
        while (true) {
            double longest = 0.0;
            const auto count = wrap(width, space_width, line_height, false, longest);
            if (count > line_count_status) {
                break; // Keep the last wrap with the original line count.
            }
            line_count_status = count;
            accepted_width = width;
            const double next_width = longest - space_width / 2.0;
            // Single-word lines cannot be improved. Also guard against zero
            // spaces and floating-point rounding preventing further progress.
            if (count == words_.size() || next_width <= 0.0 || next_width >= width) {
                break;
            }
            width = next_width;
        }

        double longest = 0.0;
        const auto count = wrap(accepted_width, space_width, line_height, true, longest);
        if (count < 0) {
            clearResult();
            return;
        }
        result_.width_ = longest;
        result_.height_ = count * line_height;
        fits_ = result_.width_ <= dimension_.width_ && result_.height_ <= dimension_.height_;
    }

} // namespace Grain
