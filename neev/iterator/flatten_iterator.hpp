// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE.txt
// 
// (C) Copyright 2014 Pierre Talbot <ptalbot@hyc.io>

#ifndef NEEV_FLATTEN_ITERATOR_HPP
#define NEEV_FLATTEN_ITERATOR_HPP

#include <boost/integer_traits.hpp>

#include <boost/iterator.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/iterator_categories.hpp>

#include <boost/type_traits/is_class.hpp>
#include <boost/static_assert.hpp>

#include <neev/iterator/detail/iterator_traits.hpp>

namespace neev
{
  static const std::size_t guess_value = boost::integer_traits<std::size_t>::const_max;

  template <class Iterator, std::size_t n = guess_value>
  class flatten_iterator;

  namespace detail
  {
    template <class Iterator, std::size_t n>
    struct flatten_iterator_base
    {
      typedef boost::iterator_adaptor<
          flatten_iterator<Iterator, n>
        , Iterator
        , typename innermost_iterator_value<Iterator, n>::type
        , boost::incrementable_traversal_tag
      > type;
    };
  } // namespace detail

  template <class Iterator>
  class flatten_iterator<Iterator, 0>
  : public detail::flatten_iterator_base<Iterator, 0>::type
  {
    typedef typename detail::flatten_iterator_base<
      Iterator
    , 0
    >::type super_t;

    friend class boost::iterator_core_access;

   public:
      flatten_iterator() { }

      flatten_iterator(Iterator x, Iterator end_ = Iterator())
      : super_t(x)
      , m_end(end_)
      {
      }

      // template<class OtherIterator>
      // flatten_iterator(
      //     flatten_iterator<OtherIterator> const& t
      //     , typename enable_if_convertible<OtherIterator, Iterator>::type* = 0
      //     )
      //     : super_t(t.base()), m_end(t.end()) {}

      Iterator end() const { return m_end; }

   private:
      // Precondition: this != end()
      void increment()
      {
        ++this->base_reference();
      }

      typename super_t::reference dereference() const
      {
        return *this->base();
      }

      // void decrement()
      // {
      //   --this->base_reference();
      // }

      Iterator m_end;
  };
  
  template <class Iterator>
  class flatten_iterator<Iterator, guess_value>
  : public flatten_iterator<Iterator,
      detail::iterator_nested_depth<Iterator>::value>
  {
    typedef flatten_iterator<
      Iterator, 
      detail::iterator_nested_depth<Iterator>::value
    > super_t;
   
   public:
    flatten_iterator() { }

    flatten_iterator(Iterator x, Iterator end_ = Iterator())
    : super_t(x, end_)
    {
    }
  };

  template <class Iterator, std::size_t n>
  class flatten_iterator
  : public detail::flatten_iterator_base<Iterator, n>::type
  {
    typedef typename detail::flatten_iterator_base<
      Iterator
    , n
    >::type super_t;

    typedef flatten_iterator<
      typename detail::inner_iterator<Iterator>::type,
      n - 1
    > inner_iterator_type;

    friend class boost::iterator_core_access;

   public:
      flatten_iterator() {}

      flatten_iterator(Iterator x, Iterator end_ = Iterator())
      : super_t(x)
      , m_end(end_)
      {
        if(this->base() != end())
        {
          update_inner();
          skip_forward_empty_inner();
        }
      }

      // template<class OtherIterator>
      // flatten_iterator(
      //     flatten_iterator<OtherIterator> const& t
      //     , typename enable_if_convertible<OtherIterator, Iterator>::type* = 0
      //     )
      //     : super_t(t.base()), m_end(t.end()) {}

      Iterator end() const { return m_end; }

   private:
      void update_inner()
      {
        inner = inner_iterator_type(
          this->base_reference()->begin(), 
          this->base_reference()->end());
      }

      void skip_forward_empty_inner()
      {
        if(this->base() != end())
        {
          while(inner.base() == inner.end())
          {
            ++(this->base_reference());
            if(this->base() != end())
              update_inner();
            else break;
          }
        }
      }

      void skip_backward_empty_inner()
      {
        while(inner.base() == inner.end())
        {
          --(this->base_reference());
          update_inner();
        }
      }

      // Precondition: this != end()
      void increment()
      {
        ++inner;
        skip_forward_empty_inner();
      }

      typename super_t::reference dereference() const
      {
        return *inner;
      }

      // void decrement()
      // {
      // }

      inner_iterator_type inner;
      Iterator m_end;
  };

  template <class Iterator>
  flatten_iterator<Iterator>
  make_flatten_iterator(Iterator x, Iterator end = Iterator())
  {
      return flatten_iterator<Iterator>(x,end);
  }

} // namespace neev

#endif // NEEV_FLATTEN_ITERATOR_HPP
