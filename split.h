/*
	split.h -- A template-based implementaion of Perl-like functions for
	           splitting a string (split()) and joining collections (e.g.
	           arrays, vectors, lists, sets) into one big string (join).
	
	Based on a seminal work on this community post (in Portuguese, plus C++):
	<https://www.vivaolinux.com.br/topico/C-C++/Como-fazer-Split-String-com-C>.
	
	Author: Paulo A. P. Pires
	Copyright 2018-2020, Paulo A. P. Pires

	This file is temporarily licensed for general use.  Please submit
	suggestions and improvements back to me, so I can ad them to the
	repository.
*/


#ifndef ORG_PPIRES_SPLIT_H__
#define ORG_PPIRES_SPLIT_H__


#include <iterator>
#include <limits>
#include <locale>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>


namespace {

const std::string SPLIT_H_RCSID{"$Id: split.h,v 1.2 2020/08/03 09:38:10 pappires Exp pappires $"};

}

namespace org::ppires {

//constexpr size_t split_max=std::numeric_limits<std::vector<std::string>::size_type>::max();
constexpr size_t split_max=std::numeric_limits<size_t>::max();


/****** Split functions with arguments that are based on std::basic_string_view. ******/
template<
	class char_t, class char_traits_t,
	class out_ch_alloc_t=std::allocator<char_t>,
	class out_str_alloc_t=std::allocator<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>>
>
inline std::vector<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t>
split(
	const std::basic_string_view<char_t, char_traits_t> str,
	char_t sep, size_t max_fields=0,
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t(),
	const out_str_alloc_t &alloc_str=out_str_alloc_t()
){
	std::vector<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t> result;
	const size_t str_len=str.length();
	if(str_len){
		size_t a=0, b;
		if(max_fields--){
			do {
				b=(result.size()>=max_fields? str.npos: str.find(sep, a));
				result.emplace_back(str, a, b-a, alloc_ch);
				a=b+1;
			} while(b!=str.npos && a<=str_len);
		}
		else {
			size_t trailing_empty=0;
			do {
				b=str.find(sep, a);
				if(b==a)
					++trailing_empty;
				else {
					for(; trailing_empty; --trailing_empty)
						result.emplace_back();
					result.emplace_back(str, a, b-a, alloc_ch);
				}
				a=b+1;
			} while(b!=str.npos && a<str_len);
		}
	}
	return result;
}

template<
	class char_t, class char_traits_t,
	class out_ch_alloc_t=std::allocator<char_t>,
	class out_str_alloc_t=std::allocator<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>>
>
inline std::vector<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t>
split(
	const std::basic_string_view<char_t, char_traits_t> str,
	const std::basic_string_view<char_t, char_traits_t> sep,
	size_t max_fields=0,
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t(),
	const out_str_alloc_t &alloc_str=out_str_alloc_t()
){
	std::vector<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t> result;
	const size_t str_len=str.length();
	if(str_len){
		const size_t sep_len=sep.length();
		const size_t empty_sep=!sep_len;
		size_t a=0, b;
		if(max_fields--){
			do {
				b=(result.size()>=max_fields? str.npos: str.find(sep, a)+empty_sep);
				result.emplace_back(str, a, b-a, alloc_ch);
				a=b+sep_len;
			} while(b!=str.npos && a<=str_len);
		}
		else {
			size_t trailing_empty=0;
			do {
				b=str.find(sep, a)+empty_sep;
				if(b==a)
					++trailing_empty;
				else {
					for(; trailing_empty; --trailing_empty)
						result.emplace_back();
					result.emplace_back(str, a, b-a, alloc_ch);
				}
				a=b+sep_len;
			} while(b!=str.npos && a<str_len);
		}
	}
	return result;
}

template<
	class char_t, class char_traits_t,
	class regex_traits_t,
	class out_ch_alloc_t=std::allocator<char_t>,
	class out_str_alloc_t=std::allocator<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>>
>
inline std::vector<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t>
split(
	const std::basic_string_view<char_t, char_traits_t> str,
	const std::basic_regex<char_t, regex_traits_t> &sep_re,
	size_t max_fields=0,
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t(),
	const out_str_alloc_t &alloc_str=out_str_alloc_t()
){
	std::vector<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t> result;
	const size_t str_len=str.length();
	if(str_len){
		std::match_results<decltype(str.begin())> sep;
		auto a=str.cbegin();
		if(max_fields--){
			auto b=a;
			do {
				size_t sep_len=0;
				if(
					result.size()<max_fields &&
					regex_search(a, str.end(), sep, sep_re)
				){
					sep_len=sep.length(0);
					b=a+sep.position(0)+!sep_len;
				}
				else
					b=str.cend();
				result.emplace_back(a, b, alloc_ch);
				a=b+sep_len;
			} while(a!=str.cend());
			if(b!=str.cend() && result.size()<max_fields)
				result.emplace_back(alloc_ch);
		}
		else {
			size_t trailing_empty=0;
			do {
				auto b=a;
				size_t sep_len=0;
				if(regex_search(a, str.end(), sep, sep_re)){
					sep_len=sep.length(0);
					b=a+sep.position(0)+!sep_len;
				}
				else
					b=str.cend();
				if(b==a)
					++trailing_empty;
				else {
					for(; trailing_empty; --trailing_empty)
						result.emplace_back();
					result.emplace_back(a, b, alloc_ch);
				}
				a=b+sep_len;
			} while(a!=str.cend());
		}
	}
	return result;
}

template<
	class char_t, class char_traits_t,
	class out_ch_alloc_t=std::allocator<char_t>,
	class out_str_alloc_t=std::allocator<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>>
>
inline std::vector<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t>
split(
	const std::basic_string_view<char_t, char_traits_t> str,
	size_t max_fields=0,
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t(),
	const out_str_alloc_t &alloc_str=out_str_alloc_t()
){
	static const char sep_re_8[]="\\s+";
	static std::unique_ptr<std::basic_regex<char_t>> sep_re;
	if(!sep_re){
		auto &converter=
			std::use_facet<std::codecvt<char_t, char, std::mbstate_t>>(std::locale())
		;
		std::mbstate_t mbs{};
		std::basic_string<char_t, char_traits_t> sep_str(sizeof sep_re_8, char_t());
		const char *from_next;
		char_t *to_next;
		converter.in(
			mbs,
			sep_re_8, sep_re_8+sizeof sep_re_8, from_next,
			&sep_str[0], &sep_str[sep_str.size()], to_next
		);
		sep_str.resize(to_next-&sep_str[0]);
		sep_re.reset(new std::basic_regex<char_t>(sep_str));
	}
	return split(str, *sep_re, max_fields, alloc_ch, alloc_str);
}

template<
	class char_traits_t,
	class out_ch_alloc_t=std::allocator<char>,
	class out_str_alloc_t=std::allocator<std::basic_string<char, char_traits_t, out_ch_alloc_t>>
>
inline std::vector<std::basic_string<char, char_traits_t, out_ch_alloc_t>, out_str_alloc_t>
split(
	const std::basic_string_view<char, char_traits_t> str,
	size_t max_fields=0,
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t(),
	const out_str_alloc_t &alloc_str=out_str_alloc_t()
){
	return split(str, std::regex("\\s+"), max_fields, alloc_ch, alloc_str);
}

template<
	class char_traits_t,
	class out_ch_alloc_t=std::allocator<wchar_t>,
	class out_str_alloc_t=std::allocator<std::basic_string<wchar_t, char_traits_t, out_ch_alloc_t>>
>
inline std::vector<std::basic_string<wchar_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t>
split(
	const std::basic_string_view<wchar_t, char_traits_t> str,
	size_t max_fields=0,
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t(),
	const out_str_alloc_t &alloc_str=out_str_alloc_t()
){
	return split(str, std::wregex(L"\\s+"), max_fields, alloc_ch, alloc_str);
}

template<
	class char_traits_t,
	class out_ch_alloc_t=std::allocator<char32_t>,
	class out_str_alloc_t=std::allocator<std::basic_string<char32_t, char_traits_t, out_ch_alloc_t>>
>
inline std::vector<std::basic_string<char32_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t>
split(
	const std::basic_string_view<char32_t, char_traits_t> str,
	size_t max_fields=0,
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t(),
	const out_str_alloc_t &alloc_str=out_str_alloc_t()
){
	return split(str, std::basic_regex<char32_t>(U"\\s+"), max_fields, alloc_ch, alloc_str);
}

template<
	class char_traits_t,
	class out_ch_alloc_t=std::allocator<char16_t>,
	class out_str_alloc_t=std::allocator<std::basic_string<char16_t, char_traits_t, out_ch_alloc_t>>
>
inline std::vector<std::basic_string<char16_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t>
split(
	const std::basic_string_view<char16_t, char_traits_t> str,
	size_t max_fields=0,
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t(),
	const out_str_alloc_t &alloc_str=out_str_alloc_t()
){
	return split(str, std::basic_regex<char16_t>(u"\\s+"), max_fields, alloc_ch, alloc_str);
}

#if __cplusplus >= 202002L
template<
	class char_traits_t,
	class out_ch_alloc_t=std::allocator<char8_t>,
	class out_str_alloc_t=std::allocator<std::basic_string<char8_t, char_traits_t, out_ch_alloc_t>>
>
inline std::vector<std::basic_string<char8_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t>
split(
	const std::basic_string_view<char8_t, char_traits_t> str,
	size_t max_fields=0,
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t(),
	const out_str_alloc_t &alloc_str=out_str_alloc_t()
){
	return split(str, std::basic_regex<char8_t>(u8"\\s+"), max_fields, alloc_ch, alloc_str);
}
#endif


/****** Split functions with at least one argument based on std::basic_string. ******/
template<
	class char_t, class char_traits_t,
	class in_ch_alloc_t, class out_ch_alloc_t=in_ch_alloc_t,
	class out_str_alloc_t=std::allocator<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>>
>
inline std::vector<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t>
split(
	const std::basic_string<char_t, char_traits_t, in_ch_alloc_t> &str,
	char_t sep,
	size_t max_fields=0,
	const out_str_alloc_t &alloc_str=out_str_alloc_t()
){
	return
		split(
			std::basic_string_view<char_t, char_traits_t>(str),
			sep, max_fields,
			out_ch_alloc_t(), alloc_str
		)
	;
}

template<
	class char_t, class char_traits_t,
	class in_ch_alloc_t, class out_ch_alloc_t=in_ch_alloc_t,
	class out_str_alloc_t=std::allocator<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>>
>
inline std::vector<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t>
split(
	const std::basic_string<char_t, char_traits_t, in_ch_alloc_t> &str,
	const std::basic_string_view<char_t, char_traits_t> sep,
	size_t max_fields=0,
	const out_str_alloc_t &alloc_str=out_str_alloc_t()
){
	return
		split(
			std::basic_string_view<char_t, char_traits_t>(str),
			sep, max_fields,
			out_ch_alloc_t(), alloc_str
		)
	;
}

template<
	class char_t, class char_traits_t,
	class in_ch_alloc_t, class out_ch_alloc_t=in_ch_alloc_t,
	class out_str_alloc_t=std::allocator<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>>
>
inline std::vector<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t>
split(
	const std::basic_string_view<char_t, char_traits_t> str,
	const std::basic_string<char_t, char_traits_t, in_ch_alloc_t> &sep,
	size_t max_fields=0,
	const out_str_alloc_t &alloc_str=out_str_alloc_t()
){
	return
		split(
			str,
			std::basic_string_view<char_t, char_traits_t>(sep), max_fields,
			out_ch_alloc_t(), alloc_str
		)
	;
}

template<
	class char_t, class char_traits_t,
	class in_ch_alloc_t, class out_ch_alloc_t=in_ch_alloc_t,
	class sep_ch_alloc_t,
	class out_str_alloc_t=std::allocator<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>>
>
inline std::vector<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t>
split(
	const std::basic_string<char_t, char_traits_t, in_ch_alloc_t> &str,
	const std::basic_string<char_t, char_traits_t, sep_ch_alloc_t> &sep,
	size_t max_fields=0,
	const out_str_alloc_t &alloc_str=out_str_alloc_t()
){
	return
		split(
			std::basic_string_view<char_t, char_traits_t>(str),
			std::basic_string_view<char_t, char_traits_t>(sep), max_fields,
			out_ch_alloc_t(), alloc_str
		)
	;
}

template<
	class char_t, class char_traits_t,
	class regex_traits_t,
	class in_ch_alloc_t, class out_ch_alloc_t=in_ch_alloc_t,
	class out_str_alloc_t=std::allocator<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>>
>
inline std::vector<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t>
split(
	const std::basic_string<char_t, char_traits_t, in_ch_alloc_t> &str,
	const std::basic_regex<char_t, regex_traits_t> &sep_re,
	size_t max_fields=0,
	const out_str_alloc_t &alloc_str=out_str_alloc_t()
){
	return
		split(
			std::basic_string_view<char_t, char_traits_t>(str),
			sep_re, max_fields,
			out_ch_alloc_t(), alloc_str
		)
	;
}

template<
	class char_t, class char_traits_t,
	class in_ch_alloc_t, class out_ch_alloc_t=in_ch_alloc_t,
	class out_str_alloc_t=std::allocator<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>>
>
inline std::vector<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t>
split(
	const std::basic_string<char_t, char_traits_t, in_ch_alloc_t> &str,
	size_t max_fields=0,
	const out_str_alloc_t &alloc_str=out_str_alloc_t()
){
	return
		split(
			std::basic_string_view<char_t, char_traits_t>(str),
			max_fields,
			out_ch_alloc_t(), alloc_str
		)
	;
}


/****** Split functions with argument(s) that is(are) pointer(s) to characters ******/
template<
	class char_t, class char_traits_t=std::char_traits<char_t>,
	class out_ch_alloc_t=std::allocator<char_t>,
	class out_str_alloc_t=std::allocator<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>>
>
inline std::vector<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t>
split(
	const char_t *str, char_t sep, size_t max_fields=0,
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t(),
	const out_str_alloc_t &alloc_str=out_str_alloc_t()
){
	return
		split(
			std::basic_string_view<char_t, char_traits_t>(str),
			sep, max_fields,
			alloc_ch, alloc_str
		)
	;
}

template<
	class char_t, class char_traits_t,
	class out_ch_alloc_t=std::allocator<char_t>,
	class out_str_alloc_t=std::allocator<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>>
>
inline std::vector<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t>
split(
	const char_t *str, const std::basic_string_view<char_t, char_traits_t> sep,
	size_t max_fields=0,
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t(),
	const out_str_alloc_t &alloc_str=out_str_alloc_t()
){
	return
		split(
			std::basic_string_view<char_t, char_traits_t>(str),
			sep, max_fields,
			alloc_ch, alloc_str
		)
	;
}

template<
	class char_t, class char_traits_t,
	class out_ch_alloc_t=std::allocator<char_t>,
	class out_str_alloc_t=std::allocator<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>>
>
inline std::vector<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t>
split(
	const std::basic_string_view<char_t, char_traits_t> str, const char_t *sep,
	size_t max_fields=0,
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t(),
	const out_str_alloc_t &alloc_str=out_str_alloc_t()
){
	return
		split(
			str, std::basic_string_view<char_t, char_traits_t>(sep),
			max_fields,
			alloc_ch, alloc_str
		)
	;
}

template<
	class char_t, class char_traits_t=std::char_traits<char_t>,
	class out_ch_alloc_t=std::allocator<char_t>,
	class out_str_alloc_t=std::allocator<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>>
>
inline std::vector<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t>
split(
	const char_t *str, const char_t *sep, size_t max_fields=0,
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t(),
	const out_str_alloc_t &alloc_str=out_str_alloc_t()
){
	return
		split(
			std::basic_string_view<char_t, char_traits_t>(str),
			std::basic_string_view<char_t, char_traits_t>(sep), max_fields,
			alloc_ch, alloc_str
		)
	;
}

template<
	class char_t, class char_traits_t=std::char_traits<char_t>,
	class regex_traits_t,
	class out_ch_alloc_t=std::allocator<char_t>,
	class out_str_alloc_t=std::allocator<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>>
>
inline std::vector<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t>
split(
	const char_t *str, const std::basic_regex<char_t, regex_traits_t> &sep_re,
	size_t max_fields=0,
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t(),
	const out_str_alloc_t &alloc_str=out_str_alloc_t()
){
	return
		split(
			std::basic_string_view<char_t, char_traits_t>(str),
			sep_re, max_fields,
			alloc_ch, alloc_str
		)
	;
}

template<
	class char_t, class char_traits_t=std::char_traits<char_t>,
	class out_ch_alloc_t=std::allocator<char_t>,
	class out_str_alloc_t=std::allocator<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>>
>
inline std::vector<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t>
split(
	const char_t *str, size_t max_fields=0,
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t(),
	const out_str_alloc_t &alloc_str=out_str_alloc_t()
){
	return
		split(
			std::basic_string_view<char_t, char_traits_t>(str),
			max_fields,
			alloc_ch, alloc_str
		)
	;
}


/****** Split functions with mixed character pointers and std::basic_string arguments ******/
template<
	class char_t, class char_traits_t,
	class in_ch_alloc_t, class out_ch_alloc_t=in_ch_alloc_t,
	class out_str_alloc_t=std::allocator<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>>
>
inline std::vector<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t>
split(
	const char_t *str,
	const std::basic_string<char_t, char_traits_t, in_ch_alloc_t> &sep,
	size_t max_fields=0,
	const out_str_alloc_t &alloc_str=out_str_alloc_t()
){
	return
		split(
			std::basic_string_view<char_t, char_traits_t>(str),
			std::basic_string_view<char_t, char_traits_t>(sep), max_fields,
			out_ch_alloc_t(), alloc_str
		)
	;
}

template<
	class char_t, class char_traits_t,
	class in_ch_alloc_t, class out_ch_alloc_t=in_ch_alloc_t,
	class out_str_alloc_t=std::allocator<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>>
>
inline std::vector<std::basic_string<char_t, char_traits_t, out_ch_alloc_t>, out_str_alloc_t>
split(
	const std::basic_string<char_t, char_traits_t, in_ch_alloc_t> &str,
	const char_t *sep, size_t max_fields=0,
	const out_str_alloc_t &alloc_str=out_str_alloc_t()
){
	return
		split(
			str,
			std::basic_string_view<char_t, char_traits_t>(sep), max_fields,
			out_ch_alloc_t(), alloc_str
		)
	;
}




/****** Functions that join split things into a bigger string. ******/
template<
	class char_t, class char_traits_t=std::char_traits<char_t>,
	class out_ch_alloc_t=std::allocator<char_t>,
	class input_iter_t,
	class joiner_t, class last_joiner_t
>
inline std::basic_string<char_t, char_traits_t, out_ch_alloc_t>
basic_join(
	input_iter_t first, input_iter_t last,
	const joiner_t &joiner, const last_joiner_t &last_joiner,
	const std::locale &out_locale=std::locale(),
	const out_ch_alloc_t alloc_ch=out_ch_alloc_t()
){
	std::basic_ostringstream<char_t, char_traits_t, out_ch_alloc_t>
		output{
			std::basic_string<char_t, char_traits_t, out_ch_alloc_t>(alloc_ch)
		}
	;
	output.imbue(out_locale);
	if(first!=last){
		output << *first;
		if(++first!=last){
			auto second=first;
			while(++second!=last)
				output << joiner << *first++;
			output << last_joiner << *first;
		}
	}
	return output.str();
}

template<
	class char_t=char, class char_traits_t=std::char_traits<char_t>,
	class out_ch_alloc_t=std::allocator<char_t>,
	class input_iter_t,
	class joiner_t, class last_joiner_t
>
inline std::basic_string<char_t, char_traits_t, out_ch_alloc_t>
join(
	input_iter_t first, input_iter_t last,
	const joiner_t &joiner, const last_joiner_t &last_joiner,
	const std::locale &out_locale=std::locale(),
	const out_ch_alloc_t alloc_ch=out_ch_alloc_t()
){
	return
		basic_join<char_t, char_traits_t>(
			first, last, joiner, last_joiner, out_locale, alloc_ch
		)
	;
}

template<
    class char_t=char, class char_traits_t=std::char_traits<char_t>,
    class out_ch_alloc_t=std::allocator<char_t>,
    class container_t,
    class joiner_t, class last_joiner_t
>
inline std::basic_string<char_t, char_traits_t, out_ch_alloc_t>
join(
    const container_t &cont,
    const joiner_t &joiner, const last_joiner_t &last_joiner,
    const std::locale &out_locale=std::locale(),
    const out_ch_alloc_t alloc_ch=out_ch_alloc_t()
){
	return
		basic_join<char_t, char_traits_t>(
			std::begin(cont), std::end(cont),
			joiner, last_joiner,
			out_locale, alloc_ch
		)
	;
}

template<
	class char_t=char, class char_traits_t=std::char_traits<char_t>,
	class out_ch_alloc_t=std::allocator<char_t>,
	class input_iter_t,
	class joiner_t
>
inline std::basic_string<char_t, char_traits_t, out_ch_alloc_t>
join(
	input_iter_t first, input_iter_t last,
	const joiner_t &joiner,
	const std::locale &out_locale=std::locale(),
	const out_ch_alloc_t alloc_ch=out_ch_alloc_t()
){
	return
		basic_join<char_t, char_traits_t>(
			first, last,
			joiner, joiner,
			out_locale, alloc_ch
		)
	;
}

template<
    class char_t=char, class char_traits_t=std::char_traits<char_t>,
    class out_ch_alloc_t=std::allocator<char_t>,
    class container_t,
    class joiner_t, class last_joiner_t
>
inline std::basic_string<char_t, char_traits_t, out_ch_alloc_t>
join(
    const container_t &cont,
    const joiner_t &joiner,
    const std::locale &out_locale=std::locale(),
    const out_ch_alloc_t alloc_ch=out_ch_alloc_t()
){
	return
		basic_join<char_t, char_traits_t>(
			std::begin(cont), std::end(cont),
			joiner, joiner,
			out_locale, alloc_ch
		)
	;
}

template<class input_iter_t>
inline std::string join(
	input_iter_t first, input_iter_t last,
	char joiner=' ',
	const std::locale &out_locale=std::locale()
){
	return basic_join<char>(first, last, joiner, joiner, out_locale);
}

template<class container_t>
inline std::string join(
	const container_t &cont,
	char joiner=' ',
	const std::locale &out_locale=std::locale()
){
	return
		basic_join<char>(
			std::begin(cont), std::end(cont), joiner, joiner, out_locale
		)
	;
}

template<class input_iter_t>
inline std::wstring join(
	input_iter_t first, input_iter_t last,
	wchar_t joiner=L' ',
	const std::locale &out_locale=std::locale()
){
	return basic_join<wchar_t>(first, last, joiner, joiner, out_locale);
}

template<class container_t>
inline std::wstring join(
	const container_t &cont,
	wchar_t joiner=L' ',
	const std::locale &out_locale=std::locale()
){
	return
		basic_join<wchar_t>(
			std::begin(cont), std::end(cont), joiner, joiner, out_locale
		)
	;
}

template<
	class char_t, class char_traits_t,
	class out_ch_alloc_t=std::allocator<char_t>,
	class input_iter_t
>
inline std::basic_string<char_t, char_traits_t, out_ch_alloc_t>
join(
	input_iter_t first, input_iter_t last,
	std::basic_string_view<char_t, char_traits_t> joiner,
	const std::locale &out_locale=std::locale(),
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t()
){
	return
		basic_join<char_t, char_traits_t>(
			first, last, joiner, joiner, out_locale, alloc_ch
		)
	;
}

template<
	class char_t, class char_traits_t,
	class out_ch_alloc_t=std::allocator<char_t>,
	class input_iter_t
>
inline std::basic_string<char_t, char_traits_t, out_ch_alloc_t>
join(
	input_iter_t first, input_iter_t last,
	std::basic_string_view<char_t, char_traits_t> joiner,
	std::basic_string_view<char_t, char_traits_t> last_joiner,
	const std::locale &out_locale=std::locale(),
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t()
){
	return
		basic_join<char_t, char_traits_t>(
			first, last,
			joiner, last_joiner,
			out_locale, alloc_ch
		)
	;
}

template<
	class char_t, class char_traits_t,
	class out_ch_alloc_t=std::allocator<char_t>,
	class container_t
>
inline std::basic_string<char_t, char_traits_t, out_ch_alloc_t>
join(
	const container_t &cont,
	std::basic_string_view<char_t, char_traits_t> joiner,
	const std::locale &out_locale=std::locale(),
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t()
){
	return
		basic_join<char_t, char_traits_t>(
			std::begin(cont), std::end(cont),
			joiner, joiner,
			out_locale, alloc_ch
		)
	;
}

template<
	class char_t, class char_traits_t,
	class out_ch_alloc_t=std::allocator<char_t>,
	class container_t
>
inline std::basic_string<char_t, char_traits_t, out_ch_alloc_t>
join(
	const container_t &cont,
	std::basic_string_view<char_t, char_traits_t> joiner,
	std::basic_string_view<char_t, char_traits_t> last_joiner,
	const std::locale &out_locale=std::locale(),
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t()
){
	return
		basic_join<char_t, char_traits_t>(
			std::begin(cont), std::end(cont),
			joiner, last_joiner,
			out_locale, alloc_ch
		)
	;
}

template<
	class char_t, class char_traits_t, class in_ch_alloc_t,
	class out_ch_alloc_t=std::allocator<char_t>,
	class input_iter_t
>
inline std::basic_string<char_t, char_traits_t, out_ch_alloc_t>
join(
	input_iter_t first, input_iter_t last,
	const std::basic_string<char_t, char_traits_t, in_ch_alloc_t> &joiner,
	const std::locale &out_locale=std::locale(),
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t()
){
	return
		basic_join<char_t, char_traits_t>(
			first, last,
			joiner, joiner,
			out_locale, alloc_ch
		)
	;
}

template<
	class char_t, class char_traits_t,
	class in_ch_alloc_t, class in2_ch_alloc_t,
	class out_ch_alloc_t=std::allocator<char_t>,
	class input_iter_t
>
inline std::basic_string<char_t, char_traits_t, out_ch_alloc_t>
join(
	input_iter_t first, input_iter_t last,
	const std::basic_string<char_t, char_traits_t, in_ch_alloc_t> &joiner,
	const std::basic_string<char_t, char_traits_t, in2_ch_alloc_t> &last_joiner,
	const std::locale &out_locale=std::locale(),
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t()
){
	return
		basic_join<char_t, char_traits_t>(
			first, last,
			joiner, last_joiner,
			out_locale, alloc_ch
		)
	;
}

template<
	class char_t, class char_traits_t, class in_ch_alloc_t,
	class out_ch_alloc_t=std::allocator<char_t>,
	class container_t
>
inline std::basic_string<char_t, char_traits_t, out_ch_alloc_t>
join(
	const container_t &cont,
	const std::basic_string<char_t, char_traits_t, in_ch_alloc_t> &joiner,
	const std::locale &out_locale=std::locale(),
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t()
){
	return
		basic_join<char_t, char_traits_t>(
			std::begin(cont), std::end(cont),
			joiner, joiner,
			out_locale, alloc_ch
		)
	;
}

template<
	class char_t, class char_traits_t,
	class in_ch_alloc_t, class in2_ch_alloc_t,
	class out_ch_alloc_t=std::allocator<char_t>,
	class container_t
>
inline std::basic_string<char_t, char_traits_t, out_ch_alloc_t>
join(
	const container_t &cont,
	const std::basic_string<char_t, char_traits_t, in_ch_alloc_t> &joiner,
	const std::basic_string<char_t, char_traits_t, in2_ch_alloc_t> &last_joiner,
	const std::locale &out_locale=std::locale(),
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t()
){
	return
		basic_join<char_t, char_traits_t>(
			std::begin(cont), std::end(cont),
			joiner, last_joiner,
			out_locale, alloc_ch
		)
	;
}

template<
	class char_t, class char_traits_t=std::char_traits<char_t>,
	class out_ch_alloc_t=std::allocator<char_t>,
	class input_iter_t
>
inline std::basic_string<char_t, char_traits_t, out_ch_alloc_t>
join(
	input_iter_t first, input_iter_t last,
	const char_t *joiner,
	const std::locale &out_locale=std::locale(),
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t()
){
	return
		basic_join<char_t, char_traits_t>(
			first, last,
			joiner, joiner,
			out_locale, alloc_ch
		)
	;
}

template<
	class char_t, class char_traits_t=std::char_traits<char_t>,
	class out_ch_alloc_t=std::allocator<char_t>,
	class input_iter_t
>
inline std::basic_string<char_t, char_traits_t, out_ch_alloc_t>
join(
	input_iter_t first, input_iter_t last,
	const char_t *joiner, const char_t *last_joiner,
	const std::locale &out_locale=std::locale(),
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t()
){
	return
		basic_join<char_t, char_traits_t>(
			first, last,
			joiner, last_joiner,
			out_locale, alloc_ch
		)
	;
}

template<
	class char_t, class char_traits_t=std::char_traits<char_t>,
	class out_ch_alloc_t=std::allocator<char_t>,
	class container_t
>
inline std::basic_string<char_t, char_traits_t, out_ch_alloc_t>
join(
	const container_t &cont,
	const char_t *joiner,
	const std::locale &out_locale=std::locale(),
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t()
){
	return
		basic_join<char_t, char_traits_t>(
			std::begin(cont), std::end(cont),
			joiner, joiner,
			out_locale, alloc_ch
		)
	;
}

template<
	class char_t, class char_traits_t=std::char_traits<char_t>,
	class out_ch_alloc_t=std::allocator<char_t>,
	class container_t
>
inline std::basic_string<char_t, char_traits_t, out_ch_alloc_t>
join(
	const container_t &cont,
	const char_t *joiner, const char_t *last_joiner,
	const std::locale &out_locale=std::locale(),
	const out_ch_alloc_t &alloc_ch=out_ch_alloc_t()
){
	return
		basic_join<char_t, char_traits_t>(
			std::begin(cont), std::end(cont),
			joiner, last_joiner,
			out_locale, alloc_ch
		)
	;
}

}	// namespace org::ppires.


#endif	// !defined(ORG_PPIRES_SPLIT_H__)
