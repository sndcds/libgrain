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

        void setDimension(Dimensiond& dimension) noexcept;
    };

    class TextWordLayoutLine : public Object {
        int32_t first_word_ = 0;
        int32_t word_count_ = 0;
        double width_ = 0.0;
    };

    class TextWordLayoutLayout : public Object {
        ObjectList<TextWordLayoutLine*> lines_;
        double width_ = 0.0;
        double height_ = 0.0;
    };


    class TextWordLayout : public Object {
    public:
        TextWordLayout() = default;
        ~TextWordLayout() = default;

        explicit TextWordLayout(const char* text, Font* font) noexcept;

        void setFont(const Font* font) noexcept;
        void setText(const char* text) noexcept;
        void addWord(const char* word_text) noexcept;

        void splitWords() noexcept;
        void wordMetrics() noexcept;
        void layout() noexcept;

    private:

    private:
        ObjectList<TextWordLayoutWord*> words_;
        const Font* font_{};
        const char* text_{};
    };

} // namespace Grain

#endif // GrainTextWordLayout_hpp