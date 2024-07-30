//
// Created by benichn on 29/07/24.
//

#include "book.h"

bool Page::isOld() const
{
    return settings != lastStep.settings; // !
}

const Page* Book::get(const int id) const
{
    for (const Page& page : pages)
    {
        if (page.id == id)
        {
            return &page;
        }
    }
    return nullptr;
}

int Book::id(const int index) const
{
    return pages[index].id;
}
