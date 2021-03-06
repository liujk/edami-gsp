/* -*- mode: cc-mode; tab-width: 2; -*- */

/**
 * @file  gsp_common.h
 *
 * @brief Common type definitions used within project
 *
 * @author: Tomasz Gebarowski <gebarowski@gmail.com>
 * @date: Fri May 18 15:13:25 2012
 */

#include "gsp_common.h"

/* Documented in header */
std::vector<std::string> * Tokenize(std::string line,
                          char delimeter)
{
    /* Constants */
    static const size_t kNpos = -1;

    /* @brief Vector of read tokens */
    std::vector<std::string> *tokens = new std::vector<std::string>();

    do {
        std::string::size_type current_index = line.find_first_of(delimeter);

        if (current_index != kNpos )
        {
            std::string token = line.substr(0,
                                       current_index);

            /* Append token to  vector */
            tokens->push_back(token);

            /* Remainder */
            line = line.substr(current_index+1,
                               line.length());
        }
        else
        {
            tokens->push_back(line);
            break;
        }
    } while (line.length() > 0);

    return tokens;
}
