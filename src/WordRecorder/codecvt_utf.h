#ifndef _CODECVT_UTF_H_
#define _CODECVT_UTF_H_

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8

//#include <locale>
//#include <cstddef> // NULL, size_t
//#include <cwchar>  // for mbstate_t
//#include <cstdlib> // for multi-byte converson routines
//#include <cassert>
//#include <limits>

#include<fstream>

namespace tvt {

class codecvt_utf16 : public std::codecvt<wchar_t, char, std::mbstate_t>{

    virtual std::codecvt_base::result do_out(
        std::mbstate_t & state,
        const wchar_t * first1,
        const wchar_t * last1,
        const wchar_t * & next1,
        char * first2,
        char * last2,
        char * & next2
    ) const;

    virtual std::codecvt_base::result do_in(
        std::mbstate_t & state,
        const char * first1,
        const char * last1,
        const char * & next1,
        wchar_t * first2,
        wchar_t * last2,
        wchar_t * & next2
    ) const;

    virtual int do_encoding( ) const throw( ){
        return sizeof(wchar_t) / sizeof(char);
    }
    virtual int do_max_length( ) const throw( ){
        return do_encoding();
    }

};

//    utf-16 reverse

class codecvt_utf16_reverse/*<wchar_t>*/ : public std::codecvt<wchar_t, char, std::mbstate_t>{

    virtual std::codecvt_base::result
    do_out(
        std::mbstate_t & state,
        const wchar_t * first1,
        const wchar_t * last1,
        const wchar_t * & next1,
        char * first2,
        char * last2,
        char * & next2
    ) const;

    virtual std::codecvt_base::result
    do_in(
        std::mbstate_t & state,
        const char * first1,
        const char * last1,
        const char * & next1,
        wchar_t * first2,
        wchar_t * last2,
        wchar_t * & next2
    ) const;

    virtual int do_encoding( ) const throw( ){
        return sizeof(wchar_t) / sizeof(char);
    }
    virtual int do_max_length( ) const throw( ){
        return do_encoding();
    }

};


// maximum lenght of a multibyte string
#define MB_LENGTH_MAX 8

struct codecvt_utf8 : public std::codecvt<wchar_t, char, std::mbstate_t> {
public:
    explicit codecvt_utf8(std::size_t no_locale_manage=0)
        : std::codecvt<wchar_t, char, std::mbstate_t>(no_locale_manage) {}

protected:
    virtual std::codecvt_base::result do_in(
        std::mbstate_t& state, 
        const char * from,
        const char * from_end,
        const char * & from_next,
        wchar_t * to, 
        wchar_t * to_end, 
        wchar_t*& to_next
    ) const;

    virtual std::codecvt_base::result do_out(
        std::mbstate_t & state, const wchar_t * from,
        const wchar_t * from_end, const wchar_t*  & from_next,
        char * to, char * to_end, char * & to_next
    ) const;

    bool invalid_continuing_octet(unsigned char octet_1) const {
        return (octet_1 < 0x80|| 0xbf< octet_1);
    }

    bool invalid_leading_octet(unsigned char octet_1)   const {
        return (0x7f < octet_1 && octet_1 < 0xc0) ||
            (octet_1 > 0xfd);
    }

    // continuing octets = octets except for the leading octet
    static unsigned int get_cont_octet_count(unsigned   char lead_octet) {
        return get_octet_count(lead_octet) - 1;
    }

    static unsigned int get_octet_count(unsigned char   lead_octet){
        // if the 0-bit (MSB) is 0, then 1 character
        if (lead_octet <= 0x7f) return 1;

        // Otherwise the count number of consecutive 1 bits starting at MSB
        // assert(0xc0 <= lead_octet && lead_octet <= 0xfd);

        if (0xc0 <= lead_octet && lead_octet <= 0xdf) return 2;
        else if (0xe0 <= lead_octet && lead_octet <= 0xef) return 3;
        else if (0xf0 <= lead_octet && lead_octet <= 0xf7) return 4;
        else if (0xf8 <= lead_octet && lead_octet <= 0xfb) return 5;
        else return 6;
    }

    // How many "continuing octets" will be needed for this word
    // ==   total octets - 1.
    int get_cont_octet_out_count(wchar_t word) const ;

    virtual bool do_always_noconv() const throw() { return false; }

    // UTF-8 isn't really stateful since we rewind on partial conversions
    virtual std::codecvt_base::result do_unshift(
        std::mbstate_t&,
        char * from,
        char * /*to*/,
        char * & next
    ) const 
    {
        next = from;
        return ok;
    }

    virtual int do_encoding() const throw() {
        const int variable_byte_external_encoding=0;
        return variable_byte_external_encoding;
    }

    // How many char objects can I process to get <= max_limit
    // wchar_t objects?
    virtual int do_length(
        const std::mbstate_t &,
        const char * from,
        const char * from_end, 
        std::size_t max_limit
        ) const;

    // Largest possible value do_length(state,from,from_end,1) could return.
    virtual int do_max_length() const throw () {
        return 6; // largest UTF-8 encoding of a UCS-4 character
    }
};

/////////////////////////////////////////////////////////////////////
////// �ж� UTF-16 Little Endian \ UTF-16 Big Endian �� UTF-8
// UTF-16 LE ���� 1, BE ���� 2, UTF-8 ���� 3, ���򷵻� 0
// ��ǰ���ֽ�Ϊ 0xFFFE ��Ϊ UTF-16 LE
//                0xFEFF ��Ϊ UTF-16 BE
// ��ǰ���ֽ�Ϊ 0xEF 0xBB 0xBF ��Ϊ UTF-8
// ����Ѱ�ҵ�һ�� >127 ���ֽڡ����Ҳ��������� Unicode
// ���ҵ������� UTF-8 ��׼�жϺ����ֽڣ�������һ���ַ��������Ѱ����һ�� >127 ���ֽ�
// ���Ҳ��������� UTF-8
// ���ҵ��������㣬���� Unicode������Ļ�����Ѱ�ң�ֱ���ҵ�10���ַ�Ϊֹ��
// �ж���Ϻ� stream λ�ûص�����λ�á����� BOM ͷ������ȥ��

int IsStreamUnicode(std::wistream &wistrm);

const wchar_t utf_bom = 0xFEFF;



} // namespace tvt


#endif //_CODECVT_UTF16_H_
