#include"codecvt_utf.h"

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8

#include <locale>
#include <cstddef> // NULL, size_t
#include <cwchar>  // for mbstate_t
#include <cstdlib> // for multi-byte converson routines
#include <cassert>
#include <limits>

namespace tvt {


std::codecvt_base::result codecvt_utf16::do_out(
    std::mbstate_t & state,
    const wchar_t * first1,
    const wchar_t * last1,
    const wchar_t * & next1,
    char * first2,
    char * last2,
    char * & next2
) const {
    while(first1 != last1){
        // Per std::22.2.1.5.2/2, we can store no more that
        // last2-first2 characters. If we need to more encode
        // next internal char type, return 'partial'.
        if(static_cast<int>(sizeof(wchar_t)) > (last2 - first2)){
            next1 = first1;
            next2 = first2;
            return std::codecvt_base::partial;
        }
        * reinterpret_cast<wchar_t *>(first2) = * first1++;
        first2 += sizeof(wchar_t);
    }
    next1 = first1;
    next2 = first2;
    return std::codecvt_base::ok;
}

std::codecvt_base::result codecvt_utf16::do_in(
    std::mbstate_t & state,
    const char * first1,
    const char * last1,
    const char * & next1,
    wchar_t * first2,
    wchar_t * last2,
    wchar_t * & next2
) const {
    // Process input characters until we've run of them,
    // or the number of remaining characters is not
    // enough to construct another output character,
    // or we've run out of place for output characters.
    while(first2 != last2){
        // Have we converted all input characters? 
        // Return with 'ok', if so.
        if (first1 == last1)
             break;
        // Do we have less input characters than needed
        // for a single output character?        
        if(static_cast<int>(sizeof(wchar_t)) > (last1 - first1)){
            next1 = first1;
            next2 = first2;
            return std::codecvt_base::partial; 
        }
        *first2++ = * reinterpret_cast<const wchar_t *>(first1);
        first1 += sizeof(wchar_t);
    }
    next1 = first1;
    next2 = first2;
    return std::codecvt_base::ok;
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
//    utf-16 reverse

std::codecvt_base::result codecvt_utf16_reverse::do_out(
    std::mbstate_t & state,
    const wchar_t * first1,
    const wchar_t * last1,
    const wchar_t * & next1,
    char * first2,
    char * last2,
    char * & next2
) const {
    while(first1 != last1){
        // Per std::22.2.1.5.2/2, we can store no more that
        // last2-first2 characters. If we need to more encode
        // next internal char type, return 'partial'.
        if(static_cast<int>(sizeof(wchar_t)) > (last2 - first2)){
            next1 = first1;
            next2 = first2;
            return std::codecvt_base::partial;
        }
        * reinterpret_cast<wchar_t *>(first2) = * first1++;
        std::swap(*first2,*(first2+sizeof(wchar_t)/2)); // incompatible with ucs-4
        first2 += sizeof(wchar_t);
    }
    next1 = first1;
    next2 = first2;
    return std::codecvt_base::ok;
}

std::codecvt_base::result codecvt_utf16_reverse::do_in(
    std::mbstate_t & state,
    const char * first1,
    const char * last1,
    const char * & next1,
    wchar_t * first2,
    wchar_t * last2,
    wchar_t * & next2
) const {
    // Process input characters until we've run of them,
    // or the number of remaining characters is not
    // enough to construct another output character,
    // or we've run out of place for output characters.
    while(first2 != last2){
        // Have we converted all input characters? 
        // Return with 'ok', if so.
        if (first1 == last1)
             break;
        // Do we have less input characters than needed
        // for a single output character?        
        if(static_cast<int>(sizeof(wchar_t)) > (last1 - first1)){
            next1 = first1;
            next2 = first2;
            return std::codecvt_base::partial; 
        }
        *first2++ = * reinterpret_cast<const wchar_t *>(first1);
        std::swap(*(reinterpret_cast<char*>(first2) - 2),
            *(reinterpret_cast<char*>(first2) - 1));    // incompatible with ucs-4
        first1 += sizeof(wchar_t);
    }
    next1 = first1;
    next2 = first2;
    return std::codecvt_base::ok;
}

//////////////////////////////////////////////////////////////////////////////////
// Copyright ?2001 Ronald Garcia, Indiana University (garcia@osl.iu.edu)
// Andrew Lumsdaine, Indiana University (lums@osl.iu.edu).
// Distributed under the Boost Software License, Version 1.0. (See accompany-
// ing file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// utf8_codecvt_facet.hpp

// This header defines class utf8_codecvt_facet, derived fro 
// std::codecvt<wchar_t, char>, which can be used to convert utf8 data in
// files into wchar_t strings in the application.
//
// The header is NOT STANDALONE, and is not to be included by the USER.
// There are at least two libraries which want to use this functionality, and
// we want to avoid code duplication. It would be possible to create utf8
// library, but:
// - this requires review process first
// - in the case, when linking the a library which uses utf8 
//   (say 'program_options'), user should also link to the utf8 library.
//   This seems inconvenient, and asking a user to link to an unrevieved 
//   library is strange. 
// Until the above points are fixed, a library which wants to use utf8 must:
// - include this header from one of it's headers or sources
// - include the corresponding .cpp file from one of the sources
// - before including either file, the library must define
//   - BOOST_UTF8_BEGIN_NAMESPACE to the namespace declaration that must be used
//   - BOOST_UTF8_END_NAMESPACE to the code to close the previous namespace
//   - declaration.
//   - BOOST_UTF8_DECL -- to the code which must be used for all 'exportable'
//     symbols.
//
// For example, program_options library might contain:
//    #define BOOST_UTF8_BEGIN_NAMESPACE <backslash character> 
//             namespace boost { namespace program_options {
//    #define BOOST_UTF8_END_NAMESPACE }}
//    #define BOOST_UTF8_DECL BOOST_PROGRAM_OPTIONS_DECL
//    #include "../../detail/utf8/utf8_codecvt.cpp"
//
// Essentially, each library will have its own copy of utf8 code, in
// different namespaces. 

// Note:(Robert Ramey).  I have made the following alterations in the original
// code.
// a) Rendered utf8_codecvt<wchar_t, char>  with using templates
// b) Move longer functions outside class definition to prevent inlining
// and make code smaller
// c) added on a derived class to permit translation to/from current
// locale to utf8

//  See http://www.boost.org for updates, documentation, and revision history.

// archives stored as text - note these ar templated on the basic
// stream templates to accommodate wide (and other?) kind of characters
//
// note the fact that on libraries without wide characters, ostream is
// is not a specialization of basic_ostream which in fact is not defined
// in such cases.   So we can't use basic_ostream<OStream::char_type> but rather
// use two template parameters
//
// utf8_codecvt_facet
//   This is an implementation of a std::codecvt facet for translating 
//   from UTF-8 externally to UCS-4.  Note that this is not tied to
//   any specific types in order to allow customization on platforms
//   where wchar_t is not big enough.
//
// NOTES:  The current implementation jumps through some unpleasant hoops in
// order to deal with signed character types.  As a std::codecvt_base::result,
// it is necessary  for the ExternType to be convertible to unsigned  char.
// I chose not to tie the extern_type explicitly to char. But if any combination
// of types other than <wchar_t,char_t> is used, then std::codecvt must be
// specialized on those types for this to work.


// maximum lenght of a multibyte string
#define MB_LENGTH_MAX 8

std::codecvt_base::result codecvt_utf8::do_in(
    std::mbstate_t& state, 
    const char * from,
    const char * from_end,
    const char * & from_next,
    wchar_t * to, 
    wchar_t * to_end, 
    wchar_t*& to_next
) const{
    // Basic algorithm:  The first octet determines how many
    // octets total make up the UCS-4 character.  The remaining
    // "continuing octets" all begin with "10". To convert, subtract
    // the amount that specifies the number of octets from the first
    // octet.  Subtract 0x80 (1000 0000) from each continuing octet,
    // then mash the whole lot together.  Note that each continuing
    // octet only uses 6 bits as unique values, so only shift by
    // multiples of 6 to combine.
    while (from != from_end && to != to_end) {

        // Error checking   on the first octet
        if (invalid_leading_octet(*from)){
            from_next = from;
            to_next = to;
            return std::codecvt_base::error;
        }

        // The first octet is   adjusted by a value dependent upon 
        // the number   of "continuing octets" encoding the character
        const   int cont_octet_count = get_cont_octet_count(*from);
        const   wchar_t octet1_modifier_table[] =   {
            0x00, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc
        };

        // The unsigned char conversion is necessary in case char is
        // signed   (I learned this the hard way)
        wchar_t ucs_result = 
            (unsigned char)(*from++) - octet1_modifier_table[cont_octet_count];

        // Invariants   : 
        //   1) At the start of the loop,   'i' continuing characters have been
        //    processed 
        //   2) *from   points to the next continuing character to be processed.
        int i   = 0;
        while(i != cont_octet_count && from != from_end) {

            // Error checking on continuing characters
            if (invalid_continuing_octet(*from)) {
                from_next   = from;
                to_next =   to;
                return std::codecvt_base::error;
            }

            ucs_result *= (1 << 6); 

            // each continuing character has an extra (10xxxxxx)b attached to 
            // it that must be removed.
            ucs_result += (unsigned char)(*from++) - 0x80;
            ++i;
        }

        // If   the buffer ends with an incomplete unicode character...
        if (from == from_end && i   != cont_octet_count) {
            // rewind "from" to before the current character translation
            from_next = from - (i+1); 
            to_next = to;
            return std::codecvt_base::partial;
        }
        *to++   = ucs_result;
    }
    from_next = from;
    to_next = to;

    // Were we done converting or did we run out of destination space?
    if(from == from_end) return std::codecvt_base::ok;
    else return std::codecvt_base::partial;
}

std::codecvt_base::result codecvt_utf8::do_out(
    std::mbstate_t & state, const wchar_t * from,
    const wchar_t * from_end, const wchar_t*  & from_next,
    char * to, char * to_end, char * & to_next
) const{
    // RG - consider merging this table with the other one
    const wchar_t octet1_modifier_table[] = {
        0x00, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc
    };

    wchar_t max_wchar = (std::numeric_limits<wchar_t>::max)();
    while (from != from_end && to != to_end) {

        // Check for invalid UCS-4 character
        if (*from  > max_wchar) {
            from_next = from;
            to_next = to;
            return std::codecvt_base::error;
        }

        int cont_octet_count = get_cont_octet_out_count(*from);

        // RG  - comment this formula better
        int shift_exponent = (cont_octet_count) *   6;

        // Process the first character
        *to++ = static_cast<char>(octet1_modifier_table[cont_octet_count] +
            (unsigned char)(*from / (1 << shift_exponent)));

        // Process the continuation characters 
        // Invariants: At   the start of the loop:
        //   1) 'i' continuing octets   have been generated
        //   2) '*to'   points to the next location to place an octet
        //   3) shift_exponent is   6 more than needed for the next octet
        int i   = 0;
        while   (i != cont_octet_count && to != to_end) {
            shift_exponent -= 6;
            *to++ = static_cast<char>(0x80 + ((*from / (1 << shift_exponent)) % (1 << 6)));
            ++i;
        }
        // If   we filled up the out buffer before encoding the character
        if(to   == to_end && i != cont_octet_count) {
            from_next = from;
            to_next = to - (i+1);
            return std::codecvt_base::partial;
        }
        *from++;
    }
    from_next = from;
    to_next = to;
    // Were we done or did we run out of destination space
    if(from == from_end) return std::codecvt_base::ok;
    else return std::codecvt_base::partial;
}


    // How many char objects can I process to get <= max_limit
    // wchar_t objects?
int codecvt_utf8::do_length(
    const std::mbstate_t &,
    const char * from,
    const char * from_end, 
    std::size_t max_limit
    ) const{ 
    // RG - this code is confusing!  I need a better way to express it.
    // and test cases.

    // Invariants:
    // 1) last_octet_count has the size of the last measured character
    // 2) char_count holds the number of characters shown to fit
    // within the bounds so far (no greater than max_limit)
    // 3) from_next points to the octet 'last_octet_count' before the
    // last measured character.  
    int last_octet_count=0;
    std::size_t char_count = 0;
    const char* from_next = from;
    // Use "<" because the buffer may represent incomplete characters
    while (from_next+last_octet_count <= from_end && char_count <= max_limit) {
        from_next += last_octet_count;
        last_octet_count = (get_octet_count(*from_next));
        ++char_count;
    }
    return static_cast<int>(from_next-from_end);
}





/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// utf8_codecvt_facet.cpp

// Copyright ?2001 Ronald Garcia, Indiana University (garcia@osl.iu.edu)
// Andrew Lumsdaine, Indiana University (lums@osl.iu.edu). 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Please see the comments in <boost/detail/utf8_codecvt_facet.hpp> to
// learn how this file should be used.


// If we don't have wstring, then Unicode support 
// is not available anyway, so we don't need to even
// compiler this file. This also fixes the problem
// with mingw, which can compile this file, but will
// generate link error when building DLL.

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// implementation for wchar_t

namespace {
template<std::size_t s>
int get_cont_octet_out_count_impl(wchar_t word){
    if (word < 0x80) {
        return 0;
    }
    if (word < 0x800) {
        return 1;
    }
    return 2;
}

// note the following code will generate on some platforms where
// wchar_t is defined as UCS2.  The warnings are superfluous as
// the specialization is never instantitiated with such compilers.
template<>
int get_cont_octet_out_count_impl<4>(wchar_t word){
    if (word < 0x80) {
        return 0;
    }
    if (word < 0x800) {
        return 1;
    }
    if (word < 0x10000) {
        return 2;
    }
    if (word < 0x200000) {
        return 3;
    }
    if (word < 0x4000000) {
        return 4;
    }
    return 5;
}

} // namespace anonymous

// How many "continuing octets" will be needed for this word
// ==   total octets - 1.
int codecvt_utf8::get_cont_octet_out_count(
    wchar_t word
) const {
    return get_cont_octet_out_count_impl<sizeof(wchar_t)>(word);
}




/////////////////////////////////////////////////////////////////////
////// 判断 UTF-16 Little Endian \ UTF-16 Big Endian 或 UTF-8
// UTF-16 LE 返回 1, BE 返回 2, UTF-8 返回 3, 否则返回 0
// 若前两字节为 0xFFFE 则为 UTF-16 LE
//                0xFEFF 则为 UTF-16 BE
// 若前三字节为 0xEF 0xBB 0xBF 则为 UTF-8
// 否则，寻找第一个 >127 的字节。若找不到，则不是 Unicode
// 若找到，根据 UTF-8 标准判断后续字节，若满足一个字符，则继续寻找下一个 >127 的字节
// 若找不到，则是 UTF-8
// 若找到，不满足，则不是 Unicode。满足的话继续寻找，直到找到10个字符为止。
// 判断完毕后 stream 位置回到数据位置。若有 BOM 头则跳过去。

int IsStreamUnicode(std::wistream &wistrm){
    wchar_t buf[3] = {0};
    wistrm.read(buf,3);
    if(buf[0] == wchar_t(0xFF) && buf[1] == wchar_t(0xFE)){
        wistrm.clear();
        wistrm.seekg(2);
        wistrm.imbue(std::locale(wistrm.getloc(), new codecvt_utf16/*<wchar_t>*/));
        return 1;
    }
    if(buf[0] == wchar_t(0xFE) && buf[1] == wchar_t(0xFF)){
        wistrm.clear();
        wistrm.seekg(2);
        wistrm.imbue(std::locale(wistrm.getloc(), new codecvt_utf16_reverse/*<wchar_t>*/));
        return 2;
    }
    if(buf[0] == wchar_t(0xEF) && buf[1] == wchar_t(0xBB) && buf[2] == wchar_t(0xBF)){
        wistrm.imbue(std::locale(wistrm.getloc(), new codecvt_utf8));
        return 3;
    }

    wistrm.clear();
    wistrm.seekg(0);

    for(int i=0;i<11;++i){
        while(!wistrm.fail() && (buf[0] = wistrm.get()) < 127);
        if(wistrm.fail()){  // read error, clear failbit and return
            wistrm.clear();
            wistrm.seekg(0);
            if(i>0) break;
            else return 0;
        }
        if(buf[0] < 0xC0 || buf[0] > 0xEF){
            wistrm.seekg(0);
            return 0;
        }
        if((wistrm.get() & 0xC0) != 0x80){
            wistrm.seekg(0);
            return 0;
        }
        if((buf[0] & 0x20) && (wistrm.get() & 0xC0) != 0x80){
            wistrm.seekg(0);
            return 0;
        }
    }

    wistrm.seekg(0);
    wistrm.imbue(std::locale(wistrm.getloc(), new codecvt_utf8));
    return 3;
}











} // namespace tvt