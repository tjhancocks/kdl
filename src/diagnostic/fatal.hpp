//
// Created by Tom Hancocks on 03/04/2020.
//

#if !defined(KDL_FATAL_HPP)
#define KDL_FATAL_HPP

#include <iostream>
#include <string>
#include "parser/lexeme.hpp"

namespace kdl { namespace log {

    /**
     * Reports a fatal error to the standard error pipe of the process and terminates.
     * @param lx The lexeme in which this error is related to.
     * @param code The exit code to be raised.
     * @param message A message to show to the user about why the error occured.
     */
    __attribute__((noreturn))
    auto fatal_error(const kdl::lexeme& lx, const int code, const std::string message) -> void
    {
        std::cerr << lx.location() << " - " << message << std::endl;
        exit(code);
    }

}};

#endif //KDL_FATAL_HPP
