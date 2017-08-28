#ifndef MTL_MLOG_HPP_INCLUDED
#define MTL_MLOG_HPP_INCLUDED

// STL
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

//==============================================================================
// File configuration
//==============================================================================
static_assert(__cplusplus >= 201103L, "C++11 minimum required");

// if -pthread or -fopenmp provided only
#ifdef _REENTRANT
#   include <thread>
#   include <mutex>
#   define MTL_LOG_WITH_THREADS
#   define MTL_LOG_LOCK std::unique_lock<std::mutex> lck(MTL_LOG_NSPACE::Options::MUTEX)
#else
#   define MTL_LOG_LOCK do{}while(false)
#endif

// It allows user to customize this namespace if it collides with an actual one.
#ifndef MTL_LOG_NSPACE
#   define MTL_LOG_NSPACE mlog
#endif

// To export symbols if it doesn't exist yet.
#ifndef DECLSPEC
#   if defined(__WIN32__) || defined(__WINRT__)
#       define DECLSPEC __declspec(dllexport)
#   else
#       if defined(__GNUC__) && __GNUC__ >= 4
#           define DECLSPEC __attribute__((__visibility__("default")))
#       elif defined(__GNUC__) && __GNUC__ >= 2
#           define DECLSPEC __declspec(dllexport)
#       else
#           define DECLSPEC
#       endif
#   endif
#endif
//==============================================================================

//##############################################################################
// Macros
//##############################################################################
// Add new log methods here (tips : Xmacros)
// methodName, text, colorFromOptions, upperCaseName, getterSuffix
#define MTL_FOR_EACH_LOG_FEATURE(macro) \
    macro(info,    "INFO   ", C_GREEN,  INFO,    Info)\
    macro(warning, "WARNING", C_YELLOW, WARNING, Warning)\
    macro(error,   "ERROR  ", C_RED,    ERROR,   Error)\
    macro(fatal,   "FATAL  ", C_RED,    FATAL,   Fatal)\
    macro(debug,   "DEBUG  ", C_BLUE,   DEBUG,   Debug)\
    macro(trace,   "TRACE  ", C_BLUE,   TRACE,   Trace)


// Declares one log method.
#define MTL_LOG_DECLARE(methodName, b, c, d, e) \
template<typename... Args> DECLSPEC void methodName(const Args&... args);

// Declares one friend log method.
#define MTL_LOG_FRIEND(methodName, b, c, d, e)\
template<typename... Args> friend void MTL_LOG_NSPACE::methodName(const Args&... args);

// Implements one log method.
#define MTL_LOG_IMPLEMENTATION(methodName, header, color, methodSuffix, e)\
template<typename... Args>\
DECLSPEC void methodName(const Args&... args) {\
    MTL_LOG_LOCK;\
    if (MTL_LOG_NSPACE::Options::ENABLE_LOG && MTL_LOG_NSPACE::Options::ENABLE_##methodSuffix) {\
        MTL_LOG_NSPACE::details::printWithHeader(header, MTL_LOG_NSPACE::Options::color, args...);\
    }\
}

// Implements one log "if" method.
#define MTL_LOG_IF_IMPLEMENTATION(methodName, b, c, d, e)\
template<typename... Args>\
DECLSPEC bool methodName##_if(bool cond, const Args&... args) {\
    if (cond) {\
        MTL_LOG_NSPACE::methodName(args...);\
    }\
    return cond;\
}

// Declares one Options static variable for one log method.
#define MTL_STATIC_LOG_DECLARACTION(a, b, c, varSuffix, e)\
static bool ENABLE_##varSuffix;

// Allocates one Options static variable for one log method.
#define MTL_STATIC_LOG_ALLOCATION(a, b, c, varSuffix, e)\
template<typename T> bool MTL_LOG_NSPACE::details::StaticDeclarer<T>::ENABLE_##varSuffix = true;

// Creates a getter/setter couple for one log method.
#define MTL_LOG_GETTER_SETTER(a, b, c, varSuffix, funcSuffix)\
MTL_LOG_GET_SET(bool, funcSuffix, ENABLE_##varSuffix, is, Enabled, enable)

//
#define MTL_LOG_READ_FROM_FILE(a, b, c, d, funcSuffix)\
MTL_LOG_READ_BOOL(funcSuffix);

//
#define MTL_LOG_DUMP_TO_FILE(a, b, c, varSuffix, e)\
MTL_LOG_DUMP_LINE(dump, ENABLE_##varSuffix = 1);

//
#define MTL_LOG_DUMP_LINE(file, text) file << #text << std::endl;

//
#define MTL_LOG_READ_BOOL(varname) \
config >> foo >> equal >> value;\
MTL_LOG_NSPACE::Options::enable##varname(value);

//
#define MTL_LOG_GET_SET(type, name, option, prefixGet, suffixGet, prefixSet) \
static void prefixSet##name(type value) {\
    MTL_LOG_LOCK;\
    MTL_LOG_NSPACE::Options::option = value;\
}\
static type prefixGet##name##suffixGet(void) {\
    MTL_LOG_LOCK;\
    return MTL_LOG_NSPACE::Options::option;\
}
//##############################################################################

namespace MTL_LOG_NSPACE
{
    // Previous Declarations for methods which may access the Options ivars.----
    //! @{
    //! The different log methods, one function per log type.
    MTL_FOR_EACH_LOG_FEATURE(MTL_LOG_DECLARE)
    //! @}
    /**
     * @brief Loads the log configuration on @b fname if it exists, else it creates this file.
     * @param[in] fname The name of the configuration file.
     * @return @b true if the configuration is loaded, @b false otherwise.
     */
    inline DECLSPEC bool loadConfiguration(const std::string& fname);

    namespace details
    {
        /**
         * @brief Displays the log line (manage the header and the content).
         * @details This function is intended to be call by the log methods (such as info, warning, et).
         * @tparam Args... The types of the content you want to display.
         * @param[in] tag     This is the "type" of the log line (INFO for example).
         * @param[in] color   The color code to apply for the tag.
         * @param[in] args... The content to display.
         */
        template<typename... Args>
        void printWithHeader(const char* const tag, const char* const color, const Args&... args);


        class Header final
        {
            public:
                std::string pattern;
                Header(const char* const str) : pattern(str)
                {
                }
                Header& operator=(const std::string& str)
                {
                    this->pattern = str;
                    return *this;
                }
                operator std::string(void)
                {
                    return this->pattern;
                }
                std::ostream& print(std::ostream& out, const char *const type, const char *const color)
                {
                    const std::size_t size = this->pattern.size();
                    time_t tt = time(nullptr);
                    std::tm ltm = *localtime(&tt);

                    for(std::size_t i=0u;i<size;++i)
                    {
                        if (this->pattern[i] != '{')
                        {
                            out << this->pattern[i];
                            continue;
                        }
                        std::size_t closed = this->pattern.find('}', i);
                        if (closed != std::string::npos)
                        {
                            std::string tag(&this->pattern[i+1u], &this->pattern[closed]);
                            std::size_t oldI = i;
                            i = closed;
                            // Add your custom tags here :D
                            if (tag == "TYPE")
                            {
                                out << type;
                            }
                            else if (tag == "DATE")
                            {
                                out << std::setfill('0') << std::setw(2) << ltm.tm_mon+1 << '/';
                                out << std::setfill('0') << std::setw(2) << ltm.tm_mday  << '/';
                                out << ltm.tm_year + 1900;
                            }
                            else if (tag == "TIME")
                            {
                                out << std::setfill('0') << std::setw(2) << ltm.tm_hour << ':';
                                out << std::setfill('0') << std::setw(2) << ltm.tm_min  << ':';
                                out << std::setfill('0') << std::setw(2) << ltm.tm_sec;
                            }
                            else if (tag == "THREAD")
                            {
#ifdef MTL_LOG_WITH_THREADS
                                out << "0x" << std::hex << std::this_thread::get_id() << std::dec;
#endif
                            }
                            else
                            {
                                i = oldI;
                                out << this->pattern[i];
                            }
                        }
                    }
                    return out;
                }
        };


        template<typename T> class StaticDeclarer
        {
            protected:
                MTL_FOR_EACH_LOG_FEATURE(MTL_STATIC_LOG_DECLARACTION)
                static bool ENABLE_COLOR;
                static bool ENABLE_SPACING;
                static bool ENABLE_LOG;
                static bool ENABLE_HEADER;
                static bool ENABLE_ALPHA_BOOL;
                static std::ostream* OUT;
                static MTL_LOG_NSPACE::details::Header FORMAT;

            private:
#ifdef MTL_LOG_WITH_THREADS
                static std::mutex MUTEX;
#endif
                static constexpr const char *const C_YELLOW = "\033[1;33m";
                static constexpr const char *const C_RED    = "\033[1;31m";
                static constexpr const char *const C_GREEN  = "\033[1;32m";
                static constexpr const char *const C_BLUE   = "\033[1;36m";
                static constexpr const char *const C_BLANK  = "\033[0m";

                MTL_FOR_EACH_LOG_FEATURE(MTL_LOG_FRIEND)
                friend bool MTL_LOG_NSPACE::loadConfiguration(const std::string& fname);
                template<typename... Args>
                friend void MTL_LOG_NSPACE::details::printWithHeader(const char* const tag,
                                                                     const char* const color,
                                                                     const Args&... args);

                StaticDeclarer(void) = delete;
        };

        // Static attributes declarations
        MTL_FOR_EACH_LOG_FEATURE(MTL_STATIC_LOG_ALLOCATION)
        template<typename T> bool MTL_LOG_NSPACE::details::StaticDeclarer<T>::ENABLE_SPACING    = true;
        template<typename T> bool MTL_LOG_NSPACE::details::StaticDeclarer<T>::ENABLE_COLOR      = false;
        template<typename T> bool MTL_LOG_NSPACE::details::StaticDeclarer<T>::ENABLE_LOG        = true;
        template<typename T> bool MTL_LOG_NSPACE::details::StaticDeclarer<T>::ENABLE_HEADER     = true;
        template<typename T> bool MTL_LOG_NSPACE::details::StaticDeclarer<T>::ENABLE_ALPHA_BOOL = true;
        template<typename T> std::ostream* MTL_LOG_NSPACE::details::StaticDeclarer<T>::OUT = &std::cout;
        template<typename T>
        MTL_LOG_NSPACE::details::Header MTL_LOG_NSPACE::details::StaticDeclarer<T>::FORMAT = "[{TYPE} {DATE} {TIME}] :";
#ifdef MTL_LOG_WITH_THREADS
        template<typename T> std::mutex MTL_LOG_NSPACE::details::StaticDeclarer<T>::MUTEX;
#endif
    }

    class DECLSPEC Options final : public MTL_LOG_NSPACE::details::StaticDeclarer<MTL_LOG_NSPACE::Options>
    {
        public:
            Options(void) = delete;

            static void assertValidity(void)
            {
                if (MTL_LOG_NSPACE::Options::OUT == nullptr)
                {
                    throw std::ios_base::failure("OUT must not be nullptr !");
                }
                if (!MTL_LOG_NSPACE::Options::OUT->good())
                {
                    throw std::ios_base::failure("OUT must be a \"good()\" std::ostream* !");
                }
            }

            MTL_LOG_GET_SET(bool, Log,       ENABLE_LOG,        is, Enabled, enable)
            MTL_LOG_GET_SET(bool, Color,     ENABLE_COLOR,      is, Enabled, enable)
            MTL_LOG_GET_SET(bool, Spacing,   ENABLE_SPACING,    is, Enabled, enable)
            MTL_LOG_GET_SET(bool, AlphaBool, ENABLE_ALPHA_BOOL, is, Enabled, enable)
            MTL_LOG_GET_SET(bool, Header,    ENABLE_HEADER,     is, Enabled, enable)
            MTL_FOR_EACH_LOG_FEATURE(MTL_LOG_GETTER_SETTER)
            MTL_LOG_GET_SET(std::ostream*, OutputStream, OUT,    get, , set)
            MTL_LOG_GET_SET(std::string, Format,         FORMAT, get, , set)
    };

    namespace details
    {
        template<typename... Args>
        void printWithHeader(const char* const tag, const char* const color, const Args&... args)
        {
            MTL_LOG_NSPACE::Options::assertValidity();
            std::ostream& out = *MTL_LOG_NSPACE::Options::OUT;
            bool alphaBool    =  MTL_LOG_NSPACE::Options::ENABLE_ALPHA_BOOL;

            out << (alphaBool ? std::boolalpha : std::noboolalpha);
            if (MTL_LOG_NSPACE::Options::ENABLE_HEADER)
            {
                bool wantColor = MTL_LOG_NSPACE::Options::ENABLE_COLOR;
                out << (wantColor ? color : "");
                MTL_LOG_NSPACE::Options::FORMAT.print(out, tag, color);
                out << (wantColor ? MTL_LOG_NSPACE::Options::C_BLANK : "");
            }

            auto space = (MTL_LOG_NSPACE::Options::ENABLE_SPACING) ? " " : "";
            auto foo   = {(out << space << args, '\0')...};
            (void)foo;
            out << std::endl;
        }
    }

    MTL_FOR_EACH_LOG_FEATURE(MTL_LOG_IMPLEMENTATION)
    MTL_FOR_EACH_LOG_FEATURE(MTL_LOG_IF_IMPLEMENTATION)

    inline DECLSPEC bool loadConfiguration(const std::string& fname)
    {
        std::ifstream config(fname);
        if (config)
        {
            std::string foo;
            char equal;
            bool value;
            MTL_LOG_READ_BOOL(Log);
            MTL_LOG_READ_BOOL(Color);
            MTL_LOG_READ_BOOL(Spacing);
            MTL_LOG_READ_BOOL(Header);
            MTL_LOG_READ_BOOL(AlphaBool);
            MTL_FOR_EACH_LOG_FEATURE(MTL_LOG_READ_FROM_FILE)
            config >> foo >> equal;
            std::getline(config, foo);
            MTL_LOG_NSPACE::Options::setFormat(foo);
            config.close();
            return true;
        }
        else
        {
            std::ofstream dump(fname);
            if (dump)
            {
                MTL_LOG_DUMP_LINE(dump, ENABLE_LOG = 1);
                MTL_LOG_DUMP_LINE(dump, ENABLE_COLOR = 0);
                MTL_LOG_DUMP_LINE(dump, ENABLE_SPACING = 1);
                MTL_LOG_DUMP_LINE(dump, ENABLE_HEADER = 1);
                MTL_LOG_DUMP_LINE(dump, ENABLE_ALPHA_BOOL = 1);
                MTL_FOR_EACH_LOG_FEATURE(MTL_LOG_DUMP_TO_FILE)
                MTL_LOG_DUMP_LINE(dump, HEADER_FORMAT =[{TYPE} {DATE} {TIME}] :);
                dump.close();
            }
            return false;
        }
    }
}

#undef MTL_LOG_LOCK
#undef MTL_LOG_NSPACE
#undef MTL_FOR_EACH_LOG_FEATURE
#undef MTL_LOG_DECLARE
#undef MTL_LOG_FRIEND
#undef MTL_LOG_IMPLEMENTATION
#undef MTL_LOG_IF_IMPLEMENTATION
#undef MTL_STATIC_LOG_DECLARACTION
#undef MTL_STATIC_LOG_ALLOCATION
#undef MTL_LOG_GETTER_SETTER
#undef MTL_LOG_READ_FROM_FILE
#undef MTL_LOG_DUMP_TO_FILE
#undef MTL_LOG_DUMP_LINE
#undef MTL_LOG_READ_BOOL
#undef MTL_LOG_READ_BOOL
#undef MTL_LOG_GET_SET
#ifdef MTL_LOG_WITH_THREADS
#   undef MTL_LOG_WITH_THREADS
#endif

#endif
