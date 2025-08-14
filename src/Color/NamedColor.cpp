//
//  NamedColor.cpp
//
//  Created by Roald Christesen on from 18.04.2016
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Color/NamedColor.hpp"


namespace Grain {

    const NamedColor NamedColor::g_gretag_macbeth_colors[NamedColor::kGretagMacbethColorCount] = {
            NamedColor("Dark Skin", 0.3703669795f, 0.2464789807f, 0.2012359808f),
            NamedColor("Light Skin", 0.7181048295f, 0.5002517739f, 0.4264286259f),
            NamedColor("Blue Sky", 0.2908522164f, 0.4041199359f, 0.5455558099f),
            NamedColor("Foliage", 0.287083238f, 0.3506675822f, 0.1866178378f),
            NamedColor("Blue Flower", 0.429983978f, 0.4224307622f, 0.6352178225f),
            NamedColor("Bluish Green", 0.3301289387f, 0.6994888228f, 0.6090943771f),
            NamedColor("Orange", 0.8287937743f, 0.3982452125f, 0.1195696956f),
            NamedColor("Purpish Blue", 0.2046540017f, 0.2802319371f, 0.6117189288f),
            NamedColor("Moderate Red", 0.7061417563f, 0.2324101625f, 0.3085069047f),
            NamedColor("Purple", 0.2864881361f, 0.1649195087f, 0.3436026551f),
            NamedColor("Yellow Green", 0.5679865721f, 0.6955214771f, 0.1538872358),
            NamedColor("Orange Yellow", 0.8646066987f, 0.562371252f, 0.0726482033),
            NamedColor("Blue", 0.1029068437f, 0.1827878233f, 0.5125047684),
            NamedColor("Green", 0.2364995804f, 0.5215075914f, 0.2120088502),
            NamedColor("Red", 0.6238193332f, 0.1143205921f, 0.1691004807),
            NamedColor("Yellow", 0.9103074693f, 0.7318379492f, 0),
            NamedColor("Magenta", 0.6817425803f, 0.236255436f, 0.5257190814f),
            NamedColor("Cyan", 0, 0.4640421149f, 0.6058594644f),
            NamedColor("White 95", 0.9507591363f, 0.9519798581f, 0.9390554665f),
            NamedColor("Neutral 80", 0.739360647f, 0.7474631876f, 0.7482719158f),
            NamedColor("Neutral 65", 0.563057908f, 0.5717860685f, 0.573540856f),
            NamedColor("Neutral 50", 0.3977111467f, 0.3996337835f, 0.4010986496f),
            NamedColor("Neutral 35", 0.25506981f, 0.260639353f, 0.2667887388f),
            NamedColor("Black 20", 0.1434958419f, 0.1454947738f, 0.149797818f)
    };


    const NamedColor NamedColor::g_crayola_colors[NamedColor::kCrayolaColorCount] = {
            NamedColor("Red", 0xC91111),
            NamedColor("Red Orange", 0xD84E09),
            NamedColor("Orange", 0xFF8000),
            NamedColor("Yellow", 0xF6EB20),
            NamedColor("Yellow Green", 0x51C201),
            NamedColor("Green", 0x1C8E0D),
            NamedColor("Sky Blue", 0x09C5F4),
            NamedColor("Blue", 0x2862B9),
            NamedColor("Violet (Purple)", 0x7E44BC),
            NamedColor("White", 0xFFFFFF),
            NamedColor("Brown", 0x943F07),
            NamedColor("Black", 0x000000),

            NamedColor("Aqua Green", 0x5BD2C0),
            NamedColor("Golden Yellow", 0xF6E120),
            NamedColor("Gray", 0x808080),
            NamedColor("Jade Green", 0x7E9156),
            NamedColor("Light Blue", 0x83AFDB),
            NamedColor("Magenta", 0xF863CB),
            NamedColor("Mahogany", 0xB44848),
            NamedColor("Peach", 0xF5D4B4),
            NamedColor("Pink", 0xFCA8CC),
            NamedColor("Tan", 0xCC8454),
            NamedColor("Light Brown", 0xBF6A1F),
            NamedColor("Yellow Orange", 0xFD9800),

            NamedColor("Bronze Yellow", 0xA78B00),
            NamedColor("Cool Gray", 0x788193),
            NamedColor("Dark Brown", 0x514E49),
            NamedColor("Green Blue", 0x098FAB),
            NamedColor("Lemon Yellow", 0xF4FA9F),
            NamedColor("Light Orange", 0xFED8B1),
            NamedColor("Maroon", 0xA32E12),
            NamedColor("Pine Green", 0x007872),
            NamedColor("Raspberry", 0xAA0570),
            NamedColor("Salmon", 0xFFD3CB),
            NamedColor("Slate", 0x7C7C99),
            NamedColor("Turquoise", 0x17BFDD),

            NamedColor("Bubble Gum", 0xFFC1CC),
            NamedColor("Cerulean", 0x006A93),
            NamedColor("Gold", 0x867200),
            NamedColor("Harvest Gold", 0xE2B631),
            NamedColor("Lime Green", 0x6EEB6E),
            NamedColor("Mango", 0xFFC800),
            NamedColor("Mauve", 0xCC99BA),
            NamedColor("Navy Blue", 0x00003B),
            NamedColor("Orchid", 0xBC6CAC),
            NamedColor("Pale Rose", 0xDCCCD7),
            NamedColor("Sand", 0xEBE1C2),
            NamedColor("Silver", 0xA6AAAE),
            NamedColor("Taupe", 0xB99685),
            NamedColor("Teal", 0x0086A7),

            NamedColor("Amethyst", 0x9966CC),
            NamedColor("Auro Metal Saurus", 0x6E7F80),
            NamedColor("Baby Blue", 0x0070FF),
            NamedColor("Ball Blue", 0x21ABCD),
            NamedColor("Dollar Bill", 0x85BB65),
            NamedColor("Electric Green", 0x32CD32),
            NamedColor("Guppie Green", 0x00FF7F),
            NamedColor("Meat Brown", 0xE5B73B),
            NamedColor("Platinum", 0xE2E3E4),
            NamedColor("Rose Red", 0xFF007F),
            NamedColor("Sandstorm", 0xECD540),
            NamedColor("Spiro Disco Ball", 0x0FC0FC),
            NamedColor("Toolbox", 0x746CC0),
            NamedColor("UFO Green", 0x3CD070)
    };

} // End of namespace Grain
