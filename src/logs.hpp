#ifndef MTL_MLOG_HPP_INCLUDED
#define MTL_MLOG_HPP_INCLUDED

// STL
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>


static_assert(__cplusplus >= 201103L, "C++11 minimum required");

//------------------------------------------------------------------------------
// if -pthread or -fopenmp provided only
#ifdef _REENTRANT
#   define MTL_LOG_WITH_THREADS
#   include <thread>
#   include <mutex>
#   define MTL_LOG_LOCK std::unique_lock<std::mutex> lck(MTL_LOG_NAMESPACE::Options::MUTEX)
#else
#   define MTL_LOG_LOCK do{}while(false)
#endif

//------------------------------------------------------------------------------
// It allows user to customize this namespace if it collides with an actual one.
#ifndef MTL_LOG_NAMESPACE
#   define MTL_LOG_NAMESPACE mlog
#endif

//------------------------------------------------------------------------------
// To export symbols
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

namespace MTL_LOG_NAMESPACE
{
//------------------------------------------------------------------------------
// Add new methods here (tips : Xmacros)
#define MTL_FOR_EACH_LOG_FEATURE(macro) \
    macro(info,    "INFO   ", C_GREEN,  INFO,    Info)\
    macro(warning, "WARNING", C_YELLOW, WARNING, Warning)\
    macro(error,   "ERROR  ", C_RED,    ERROR,   Error)\
    macro(fatal,   "FATAL  ", C_RED,    FATAL,   Fatal)\
    macro(debug,   "DEBUG  ", C_BLUE,   DEBUG,   Debug)\
    macro(trace,   "TRACE  ", C_BLUE,   TRACE,   Trace)

//------------------------------------------------------------------------------
#define MTL_LOG_DECLARE(methodName, b, c, d, e) \
template<typename... Args> DECLSPEC void methodName(const Args&... args);

//------------------------------------------------------------------------------
#define MTL_LOG_FRIEND(methodName, b, c, d, e)\
template<typename... Args> friend void MTL_LOG_NAMESPACE::methodName(const Args&... args);

//------------------------------------------------------------------------------
#define MTL_LOG_IMPLEMENTATION(methodName, header, color, methodSuffix, e)\
template<typename... Args>\
DECLSPEC void methodName(const Args&... args) {\
    MTL_LOG_LOCK;\
    if (MTL_LOG_NAMESPACE::Options::ENABLE_LOG && MTL_LOG_NAMESPACE::Options::ENABLE_##methodSuffix) {\
        MTL_LOG_NAMESPACE::details::printWithHeader(header, MTL_LOG_NAMESPACE::Options::color, args...);\
    }\
}

//------------------------------------------------------------------------------
#define MTL_LOG_IF_IMPLEMENTATION(methodName, b, c, d, e)\
template<typename... Args>\
DECLSPEC bool methodName##_if(bool cond, const Args&... args) {\
    if (cond) {\
        MTL_LOG_NAMESPACE::methodName(args...);\
    }\
    return cond;\
}

//------------------------------------------------------------------------------
#define MTL_STATIC_LOG_DECLARACTION(a, b, c, varSuffix, e)\
static bool ENABLE_##varSuffix;

//------------------------------------------------------------------------------
#define MTL_STATIC_LOG_ALLOCATION(a, b, c, varSuffix, e)\
template<typename T> bool MTL_LOG_NAMESPACE::details::StaticDeclarer<T>::ENABLE_##varSuffix = true;

//------------------------------------------------------------------------------
#define MTL_LOG_GETTER_SETTER(a, b, c, varSuffix, funcSuffix)\
MTL_LOG_GET_SET(bool, funcSuffix, ENABLE_##varSuffix, is, Enabled, enable)

//------------------------------------------------------------------------------
#define MTL_LOG_READ_FROM_FILE(a, b, c, d, funcSuffix)\
MTL_LOG_READ_BOOL(funcSuffix);

//------------------------------------------------------------------------------
#define MTL_LOG_DUMP_TO_FILE(a, b, c, varSuffix, e)\
MTL_LOG_DUMP_LINE(dump, ENABLE_##varSuffix = 1);

//------------------------------------------------------------------------------
#define MTL_LOG_DUMP_LINE(file, text) file << #text << std::endl;

//------------------------------------------------------------------------------
#define MTL_LOG_READ_BOOL(varname) \
config >> foo >> equal >> value;\
MTL_LOG_NAMESPACE::Options::enable##varname(value);

//------------------------------------------------------------------------------
#define MTL_LOG_CASE_TAG(foo, id) case id

//------------------------------------------------------------------------------
#define MTL_LOG_GET_SET(type, name, option, prefixGet, suffixGet, prefixSet) \
static void prefixSet##name(type value) {\
    MTL_LOG_LOCK;\
    MTL_LOG_NAMESPACE::Options::option = value;\
}\
static type prefixGet##name##suffixGet(void) {\
    MTL_LOG_LOCK;\
    return MTL_LOG_NAMESPACE::Options::option;\
}

    MTL_FOR_EACH_LOG_FEATURE(MTL_LOG_DECLARE)
    inline DECLSPEC bool loadConfiguration(const std::string& fname);

    namespace details
    {
        template<typename... Args>
        void printWithHeader(const char* const tag, const char* const color, const Args&... args);

        class Header final
        {
            public:
                Header(void) = delete;
                ~Header(void) = default;
                Header(const char* const str) : pattern(str)
                {
                    this->build();
                }
                Header& operator=(const std::string& str)
                {
                    this->pattern = str;
                    this->build();
                    return *this;
                }
                Header& operator=(const char* const str)
                {
                    return *this = std::string(str);
                }
                operator std::string(void)
                {
                    return this->pattern;
                }
                void display(std::ostream& out, const std::string& type, const char *const color,
                             const char *const nocolor, bool colorEnabled)
                {
                    for(const auto& p : this->chunks)
                    {
                        switch(p.first)
                        {
                            MTL_LOG_CASE_TAG("{DATE}", -1):
                            {
                                time_t  tt  = time(nullptr);
                                std::tm ltm = *localtime(&tt);
                                this->printTwoDigits(out, ltm.tm_mon+1) << '/';
                                this->printTwoDigits(out, ltm.tm_mday)  << '/';
                                out << ltm.tm_year + 1900;
                                break;
                            }
                            MTL_LOG_CASE_TAG("{TIME}", -4):
                            {
                                time_t  tt  = time(nullptr);
                                std::tm ltm = *localtime(&tt);
                                this->printTwoDigits(out, ltm.tm_hour) << ':';
                                this->printTwoDigits(out, ltm.tm_min) << ':';
                                this->printTwoDigits(out, ltm.tm_sec);
                                break;
                            }
                            MTL_LOG_CASE_TAG("{THREAD}", -3):
#ifdef MTL_LOG_WITH_THREADS
                                out << "0x" << std::hex << std::this_thread::get_id() << std::dec;
#endif
                                break;
                            MTL_LOG_CASE_TAG("{TYPE}", -2):
                                if (colorEnabled)
                                {
                                    out << color;
                                }
                                out << type;
                                if (colorEnabled)
                                {
                                    out << nocolor;
                                }
                                break;
                            default:
                                for(int i=p.first;i<p.second;++i)
                                {
                                    out << this->pattern.at(i);
                                }
                        }
                    }
                }
            private:
                std::string                      pattern;
                std::vector<std::pair<int, int>> chunks;
                inline std::ostream& printTwoDigits(std::ostream& out, int value)
                {
                    return out << ((value < 10) ? "0" : "") << value;
                }
                void splitAndMerge(int begin, int end, int id)
                {
                    for(auto it=this->chunks.begin();it!=this->chunks.end();++it)
                    {
                        if (it->second > begin)
                        {
                            std::pair<int, int> mid(id, static_cast<int>(std::string::npos));
                            decltype(mid)       last(begin+end, it->second);
                            it->second = begin;
                            it = this->chunks.insert(std::next(it), last);
                            this->chunks.insert(it, mid);
                            break;
                        }
                    }
                }
                void checkAndMergeIfFound(const std::string& tag, int id)
                {
                    std::size_t pos = this->pattern.find(tag);
                    if (pos != std::string::npos)
                    {
                        this->splitAndMerge(static_cast<int>(pos), static_cast<int>(tag.length()), id);
                    }
                }
                void build(void)
                {
                    this->chunks.clear();
                    this->chunks.push_back({0, static_cast<int>(this->pattern.size())});
                    this->checkAndMergeIfFound("{TYPE}",   -2);
                    this->checkAndMergeIfFound("{DATE}",   -1);
                    this->checkAndMergeIfFound("{THREAD}", -3);
                    this->checkAndMergeIfFound("{TIME}",   -4);
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
                static MTL_LOG_NAMESPACE::details::Header FORMAT;

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
                template<typename... Args>
                friend void printWithHeader(const char* const tag, const char* const color, const Args&... args);

                StaticDeclarer(void) = delete;
        };

        // Static attributes declarations
        MTL_FOR_EACH_LOG_FEATURE(MTL_STATIC_LOG_ALLOCATION)
        template<typename T> bool MTL_LOG_NAMESPACE::details::StaticDeclarer<T>::ENABLE_SPACING    = true;
        template<typename T> bool MTL_LOG_NAMESPACE::details::StaticDeclarer<T>::ENABLE_COLOR      = false;
        template<typename T> bool MTL_LOG_NAMESPACE::details::StaticDeclarer<T>::ENABLE_LOG        = true;
        template<typename T> bool MTL_LOG_NAMESPACE::details::StaticDeclarer<T>::ENABLE_HEADER     = true;
        template<typename T> bool MTL_LOG_NAMESPACE::details::StaticDeclarer<T>::ENABLE_ALPHA_BOOL = true;
        template<typename T> std::ostream* MTL_LOG_NAMESPACE::details::StaticDeclarer<T>::OUT = &std::cout;
        template<typename T>
        MTL_LOG_NAMESPACE::details::Header MTL_LOG_NAMESPACE::details::StaticDeclarer<T>::FORMAT = "[{TYPE} {DATE} {TIME}] :";
#ifdef MTL_LOG_WITH_THREADS
        template<typename T> std::mutex MTL_LOG_NAMESPACE::details::StaticDeclarer<T>::MUTEX;
#endif
    }

    class DECLSPEC Options final : public MTL_LOG_NAMESPACE::details::StaticDeclarer<MTL_LOG_NAMESPACE::Options>
    {
        public:
            Options(void) = delete;

            static void assertValidity(void)
            {
                if (MTL_LOG_NAMESPACE::Options::OUT == nullptr)
                {
                    throw std::ios_base::failure("OUT must not be nullptr !");
                }
                if (!MTL_LOG_NAMESPACE::Options::OUT->good())
                {
                    throw std::ios_base::failure("OUT must be a \"good()\" std::ostream* !");
                }
            }

            // is[...]Enabled()
            // enable[...]
            MTL_LOG_GET_SET(bool, Log,       ENABLE_LOG,        is, Enabled, enable)
            MTL_LOG_GET_SET(bool, Color,     ENABLE_COLOR,      is, Enabled, enable)
            MTL_LOG_GET_SET(bool, Spacing,   ENABLE_SPACING,    is, Enabled, enable)
            MTL_LOG_GET_SET(bool, AlphaBool, ENABLE_ALPHA_BOOL, is, Enabled, enable)
            MTL_LOG_GET_SET(bool, Header,    ENABLE_HEADER,     is, Enabled, enable)
            MTL_FOR_EACH_LOG_FEATURE(MTL_LOG_GETTER_SETTER)
            // get[...]
            // set[...]
            MTL_LOG_GET_SET(std::ostream*, OutputStream, OUT,    get, , set)
            MTL_LOG_GET_SET(std::string, Format,         FORMAT, get, , set)
    };

    namespace details
    {
        template<typename... Args>
        void printWithHeader(const char* const tag, const char* const color, const Args&... args)
        {
            MTL_LOG_NAMESPACE::Options::assertValidity();
            std::ostream& out = *MTL_LOG_NAMESPACE::Options::OUT;
            bool alphaBool    =  MTL_LOG_NAMESPACE::Options::ENABLE_ALPHA_BOOL;

            out << (alphaBool ? std::boolalpha : std::noboolalpha);
            if (MTL_LOG_NAMESPACE::Options::ENABLE_HEADER)
            {
                MTL_LOG_NAMESPACE::Options::FORMAT.display(out, tag, color,
                                                           MTL_LOG_NAMESPACE::Options::C_BLANK,
                                                           MTL_LOG_NAMESPACE::Options::ENABLE_COLOR);
            }

            auto space = (MTL_LOG_NAMESPACE::Options::ENABLE_SPACING) ? " " : "";
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
            MTL_LOG_NAMESPACE::Options::setFormat(foo);
            config.close();
            return true;
        }
        else
        {
            std::ofstream dump(fname);
            if (dump)
            {
                MTL_LOG_DUMP_LINE(dump, ENABLE_LOG:bool = 1);
                MTL_LOG_DUMP_LINE(dump, ENABLE_COLOR:bool = 0);
                MTL_LOG_DUMP_LINE(dump, ENABLE_SPACING:bool = 1);
                MTL_LOG_DUMP_LINE(dump, ENABLE_HEADER:bool = 1);
                MTL_LOG_DUMP_LINE(dump, ENABLE_ALPHA_BOOL:bool = 1);
                MTL_FOR_EACH_LOG_FEATURE(MTL_LOG_DUMP_TO_FILE)
                MTL_LOG_DUMP_LINE(dump, HEADER_FORMAT:string =[{TYPE} {DATE} {TIME}] :);
                dump.close();
            }
            return false;
        }
    }
}

#undef MTL_LOG_LOCK
#undef MTL_LOG_NAMESPACE
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
