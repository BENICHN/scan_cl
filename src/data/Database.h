//
// Created by benichn on 29/07/24.
//

#ifndef DATABASE_H
#define DATABASE_H

#include "book.h"

class Database
{
    Book _book = {
        "Test", "/home/benichn/prog/cpp/scan/testBook/sources", "/home/benichn/prog/cpp/scan/testBook/dest", {
            Page{
                rand(), PT_GRAY, "test.png", 1, {
                    true, QSize(3000, 5000),
                    60, 250, 10, 50, 234, 20, 240, 0.5, 15000, QSize(110, 12), 4000
                },
                {}, PST_IDLE
            },
            Page{
                rand(), PT_GRAY, "test.png", 1, {
                    true, QSize(3000, 5000),
                    60, 250, 10, 50, 234, 20, 240, 0.5, 15000, QSize(110, 12), 4000
                },
                {}, PST_IDLE
            },
            Page{
                rand(), PT_GRAY, "test.png", 1, {
                    true, QSize(3000, 5000),
                    60, 250, 10, 50, 234, 20, 240, 0.5, 15000, QSize(110, 12), 4000
                },
                {}, PST_IDLE
            },
            Page{
                rand(), PT_GRAY, "test.png", 1, {
                    true, QSize(3000, 5000),
                    60, 250, 10, 50, 234, 20, 240, 0.5, 15000, QSize(110, 12), 4000
                },
                {}, PST_IDLE
            },
            Page{
                rand(), PT_GRAY, "test.png", 1, {
                    true, QSize(3000, 5000),
                    60, 250, 10, 50, 234, 20, 240, 0.5, 15000, QSize(110, 12), 4000
                },
                {}, PST_IDLE
            },
            Page{
                rand(), PT_GRAY, "test.png", 1, {
                    true, QSize(3000, 5000),
                    60, 250, 10, 50, 234, 20, 240, 0.5, 15000, QSize(110, 12), 4000
                },
                {}, PST_IDLE
            },
            Page{
                rand(), PT_GRAY, "test.png", 1, {
                    true, QSize(3000, 5000),
                    60, 250, 10, 50, 234, 20, 240, 0.5, 15000, QSize(110, 12), 4000
                },
                {}, PST_IDLE
            },
            Page{
                rand(), PT_GRAY, "test.png", 1, {
                    true, QSize(3000, 5000),
                    60, 250, 10, 50, 234, 20, 240, 0.5, 15000, QSize(110, 12), 4000
                },
                {}, PST_IDLE
            }
        }
    };

public:
    [[nodiscard]] const Book* book() const { return &_book; }
};


#endif //DATABASE_H
