#ifndef SAFESQL_HPP_
#define SAFESQL_HPP_

#include "assert.hpp"
#include "config.hpp"
#include "type_traits.hpp"

#include <iostream>		// for std::ostream
#include <sstream>		// for std::ostringstream
#include <string>		// for std::basic_string, std::to_string
#include <type_traits>	// for std::is_arithmetic, std::decay, std::is_same,  std::true_type, std::false_type
#include <utility>		// for std::swap
#include <vector>		// for std::vector

namespace fp {

	namespace detail {

		struct stringlike_tag { };
		struct numberlike_tag { };
		struct default_tag { };

		template<typename>
		struct is_string : Bool<false> { };

		template<typename CharT, typename Traits, typename Alloc>
		struct is_string<std::basic_string<CharT, Traits, Alloc>> : Bool<true> { };

		template<typename T>
		struct is_string_like : Conditional<
									std::is_same<Invoke<std::decay<T>>, char *>, 
									Bool<true>, 
									Conditional<
										is_string<T>, 
										Bool<true>, 
										Bool<false>
									>
								> { };

		template<typename T>
		struct stream_traits {
			using category = 	Conditional<
									std::is_arithmetic<T>, 
									numberlike_tag, 
									Conditional<
										is_string_like<T>, 
										stringlike_tag, 
										default_tag
									>
								>;
		};

		std::string escape(std::string s) {
			return std::move(s);
		}

		template<typename T>
		inline std::ostream & to_stream_impl(std::ostream & s, T const & v, stringlike_tag) {
			return s << '"' << escape(v) << '"';
		}

		template<typename T>
		inline std::ostream & to_stream_impl(std::ostream & s, T v, numberlike_tag) {
			using std::to_string;
			return s << to_string(v);
		}

		template<typename T>
		inline std::ostream & to_stream_impl(std::ostream & s, T const & v, default_tag) {
			using std::to_string;
			return s << escape(to_string(v));
		}
	}

	template<typename T>
	inline std::ostream & to_stream(std::ostream & s, T const & v) {
		return detail::to_stream_impl(s, v, typename detail::stream_traits<T>::category());
	}

	inline std::string substr(std::string const & s, std::size_t begin, std::size_t end) {
		return s.substr(begin, end - begin);
	}

	namespace sql {

		namespace detail {
			enum class placeholder { _ };
		}

		typedef detail::placeholder placeholder_t;
		constexpr auto _ = detail::placeholder::_;

		struct query;

		template<typename...>
		struct query_builder;

		template<typename H, typename... T>
		struct query_builder<H, T...> {
		protected:
			query const & _query;
		public:
			query_builder(query const & qry) : _query(qry) { }

			std::ostream & operator()(std::ostream &, H, T..., std::size_t = 0, std::size_t = 0);
		};

		template<typename H>
		struct query_builder<H> {
		protected:
			query const & _query;
		public:
			query_builder(query const & qry) : _query(qry) { }

			std::ostream & operator()(std::ostream &, H, std::size_t = 0, std::size_t = 0);
		};

		struct query {
		public:
			typedef query this_type;
			typedef std::size_t size_type;
		protected:
			template<typename...> friend struct query_builder;
			std::string _str;
			std::vector<size_type> _placeholders;
		public:
			query()
			: _str(), _placeholders()
			{ }

			query(query const & qry)
			: _str(qry._str), _placeholders(qry._placeholders)
			{ }

			query(query && qry) noexcept
			: _str(), _placeholders()
			{ swap(*this, qry);	}

			friend void swap(query & l, query & r) noexcept {
				using std::swap;
				swap(l._str, r._str);
				swap(l._placeholders, r._placeholders);
			}

			template<typename... Ts>
			std::string execute(Ts &&... ts) const {
				FP_ASSERT((sizeof...(Ts) == _placeholders.size()), "Argument count mismatch");
				std::ostringstream ss { };
				query_builder<Ts...> qb { *this };
				qb(ss, std::forward<Ts>(ts)...);
				return ss.str();
			}

			query & operator<<(std::string const & s) {
				_str.append(s);
				return *this;
			}

			query & operator<<(placeholder_t) {
				_placeholders.emplace_back(_str.size());
				return *this;
			}

			std::size_t placeholders() const {
				return _placeholders.size();
			}

			friend std::string to_string(query const & qry) {
				typedef typename std::vector<size_type>::const_iterator const_iterator;
				std::ostringstream ss { };
				std::size_t prev_pos = 0;
				const_iterator const end = qry._placeholders.end();
				for(const_iterator it = qry._placeholders.begin(); it != end; ++it) {
					ss << substr(qry._str, prev_pos, *it);
					ss << '?';
					prev_pos = *it;
				}
				ss << qry._str.substr(prev_pos);
				return ss.str();
			}
		};

		template<typename H, typename... T>
		std::ostream & query_builder<H, T...>::operator()(std::ostream & s, H h, T... t, std::size_t idx, std::size_t begin) {
			FP_ASSERT((idx < _query._placeholders.size()), "Index out of range");
			typename query::size_type const end = _query._placeholders[idx];
			FP_ASSERT((begin != end), "Range error");
			s << substr(_query._str, begin, end);
			to_stream(s, h);
			query_builder<T...> qb { _query };
			return qb(s, t..., idx + 1, end);
		}

		template<typename H>
		std::ostream & query_builder<H>::operator()(std::ostream & s, H h, std::size_t idx, std::size_t begin) {
			FP_ASSERT((idx < _query._placeholders.size()), "Index out of range");
			typename query::size_type const end = _query._placeholders[idx];
			FP_ASSERT((begin != end), "Range error");
			s << substr(_query._str, begin, end);
			to_stream(s, h);
			return s << _query._str.substr(end);
		}
	}
}

#endif
