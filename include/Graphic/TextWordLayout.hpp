//
//  TextWordLayout.hpp
//
//  Created by Roald Christesen on from 04.09.2026
//  Copyright (C) 2026 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 04.09.2026
//

#ifndef GrainTextWordLayout_hpp
#define GrainTextWordLayout_hpp

#include "String/String.hpp"
#include "2d/Rect.hpp"
#include "2d/Dimension.hpp"
#include "Graphic/Font.hpp"

namespace Grain {

    class TextWordLayoutWord : public Object {
        friend class TextWordLayout;
        String text_;
        Rectd rect_;

    public:
        [[nodiscard]] const String& text() const noexcept { return text_; }
        [[nodiscard]] const Rectd& rect() const noexcept { return rect_; }
    };

    class TextWordLayoutLine : public Object {
        friend class TextWordLayout;
        int64_t first_word_ = 0;
        int64_t word_count_ = 0;
        double width_ = 0.0;
    public:
        [[nodiscard]] int64_t firstWord() const noexcept { return first_word_; }
        [[nodiscard]] int64_t wordCount() const noexcept { return word_count_; }
        [[nodiscard]] double width() const noexcept { return width_; }
    };

    class TextWordLayoutLayout : public Object {
        friend class TextWordLayout;
        ObjectList<TextWordLayoutLine*> lines_;
        double width_ = 0.0;
        double height_ = 0.0;
    public:
        [[nodiscard]] int64_t lineCount() const noexcept { return lines_.size(); }
        [[nodiscard]] const TextWordLayoutLine* line(int64_t index) const noexcept {
            return lines_.elementAtIndex(index);
        }
        [[nodiscard]] double width() const noexcept { return width_; }
        [[nodiscard]] double height() const noexcept { return height_; }
    };


    class TextWordLayout : public Object {
    public:
        TextWordLayout() = default;
        ~TextWordLayout() = default;

        explicit TextWordLayout(const char* text, Font* font) noexcept;

        void setFont(const Font* font) noexcept;
        // Text and font must remain alive until layout() has finished.
        void setText(const char* text) noexcept;
        void addWord(const char* word_text) noexcept;

        void splitWords() noexcept;
        void wordMetrics() noexcept;
        // Shrink the greedy wrap until its line count would increase, keeping
        // the preceding result. Words are never split. This is a heuristic,
        // not a global minimization of differences between line widths.
        void layout() noexcept;
        void layout(double width, double height) noexcept;
        // Width must be positive; height may be zero. Both must be finite.
        void setDimension(double width, double height) noexcept;

        [[nodiscard]] const TextWordLayoutLayout& result() const noexcept { return result_; }
        // Height does not change the wrap; overflow remains available in result().
        [[nodiscard]] bool fits() const noexcept { return fits_; }
        [[nodiscard]] int64_t wordCount() const noexcept { return words_.size(); }
        [[nodiscard]] const TextWordLayoutWord* word(int64_t index) const noexcept {
            return words_.elementAtIndex(index);
        }

    private:
        // Measure a greedy wrap, optionally storing its lines and word positions.
        int64_t wrap(double width, double space_width, double line_height,
                     bool store, double& longest) noexcept;
        void clearResult() noexcept;

        ObjectList<TextWordLayoutWord*> words_;
        const Font* font_{};
        const char* text_{};
        Dimensiond dimension_;
        TextWordLayoutLayout result_;
        bool fits_ = false;
    };

} // namespace Grain

#endif // GrainTextWordLayout_hpp
