#ifndef MTL_LOGS_LIGHT_HPP_INCLUDED
#define MTL_LOGS_LIGHT_HPP_INCLUDED
#ifndef MTL_LIGHT_LOGS_DISABLED
#include <iostream>
namespace mlog {
    namespace details {
        class PerlLogger final {
                PerlLogger(void)                               = delete;
                PerlLogger(const PerlLogger& other)            = delete;
                PerlLogger(PerlLogger&& other)                 = delete;
                PerlLogger& operator=(const PerlLogger& other) = delete;
                PerlLogger& operator=(PerlLogger&& other)      = delete;
            public:
                explicit PerlLogger(const char* const type) {
                    std::cout << '[' << type << "] :";
                }
                ~PerlLogger(void) {
                    std::cout << std::endl;
                }
                template<typename T> PerlLogger&& operator,(const T& t) && {
                    std::cout << ' ' << t;
                    return std::move(*this);
                }
        };
    }
}
#   define mlog_info    mlog::__details::PerlLogger("INFO   "),
#   define mlog_warning mlog::__details::PerlLogger("WARNING"),
#   define mlog_error   mlog::__details::PerlLogger("ERROR  "),
#   define mlog_debug   mlog::__details::PerlLogger("DEBUG  "),
#else
#   define mlog_info    //
#   define mlog_warning //
#   define mlog_error   //
#   define mlog_debug   //
#endif
#endif
