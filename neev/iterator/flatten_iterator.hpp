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

  template <class Iterator, std::size_t depth = guess_value>
  class flatten_iterator;

  namespace detail
  {
    template <class Iterator, std::size_t depth>
    struct flatten_iterator_category_tag
    {
      typedef typename minimum_nested_iterator_category<
        Iterator, depth, boost::bidirectional_traversal_tag
      >::type type;
    };

    template <class Iterator, std::size_t depth>
    struct flatten_iterator_base
    {
      typedef boost::iterator_adaptor<
          flatten_iterator<Iterator, depth>
        , Iterator
        , typename innermost_iterator_value_type<Iterator, depth>::type
        , typename flatten_iterator_category_tag<Iterator, depth>::type
      > type;
    };

    template <class Iterator, std::size_t depth, bool is_bidirectional =
      boost::is_convertible<
        typename flatten_iterator_category_tag<Iterator, depth>::type,
        boost::bidirectional_traversal_tag
      >::value
    >
    class bidirectional_base
    : public detail::flatten_iterator_base<Iterator, depth>::type
    {
      typedef typename detail::flatten_iterator_base<
        Iterator, depth
      >::type super_t;

    public:
      bidirectional_base(){}
      
      bidirectional_base(Iterator x)
      : super_t(x)
      , m_begin(x)
      {}

      Iterator begin() const { return m_begin; }
    protected:
      Iterator m_begin;
    };

    template <class Iterator, std::size_t depth>
    class bidirectional_base<Iterator, depth, false>
    : public detail::flatten_iterator_base<Iterator, depth>::type
    {
      typedef typename detail::flatten_iterator_base<
        Iterator, depth
      >::type super_t;
    public:
      bidirectional_base(){}
      bidirectional_base(Iterator x)
      : super_t(x)
      {}
    };

  } // namespace detail

  template <class Iterator>
  class flatten_iterator<Iterator, 0>
  : public detail::bidirectional_base<Iterator, 0>
  {
    typedef detail::bidirectional_base<Iterator, 0> super_t;

    typedef Iterator base_iterator_type;

    typedef flatten_iterator<Iterator, 0> this_iterator;

    friend class boost::iterator_core_access;

    template<typename Ite, std::size_t d>
    friend class flatten_iterator;

   public:
    flatten_iterator() { }

    flatten_iterator(Iterator x, Iterator end_ = Iterator())
    : super_t(x)
    , m_end(end_)
    {}

    // template<class OtherIterator>
    // flatten_iterator(
    //     flatten_iterator<OtherIterator> const& t
    //     , typename boost::enable_if_convertible<OtherIterator, Iterator>::type* = 0
    //     )
    //     : super_t(t.base()), m_end(t.end()) {}

    Iterator end() const { return m_end; }

   private:
    bool decrement_impl(
      typename boost::enable_if_convertible<
        typename detail::flatten_iterator_category_tag<Iterator, 0>::type,
        boost::bidirectional_traversal_tag
      >::type* = 0)
    {
      if(this->base() != this->begin())
      {
        --(this->base_reference());
        return true;
      }
      return false;
    }

    bool equal(this_iterator const& x) const
    {
      return this->base() == x.base();
    }

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
    {}
  };

  template <class Iterator, std::size_t depth>
  class flatten_iterator
  : public detail::bidirectional_base<Iterator, depth>
  {
    typedef detail::bidirectional_base<Iterator, depth> super_t;

    typedef flatten_iterator<
      typename detail::inner_iterator<Iterator>::type,
      depth - 1
    > inner_iterator_type;

    typedef Iterator base_iterator_type;

    typedef flatten_iterator<Iterator, depth> this_iterator;

    friend class boost::iterator_core_access;

    template<typename Ite, std::size_t d>
    friend class flatten_iterator;

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
      //     , typename boost::enable_if_convertible<OtherIterator, Iterator>::type* = 0
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

      void increment()
      {
        ++inner;
        skip_forward_empty_inner();
      }

      bool update_inner_to_end(
        typename boost::enable_if_convertible<
          typename detail::flatten_iterator_category_tag<Iterator, depth>::type,
          boost::bidirectional_traversal_tag
        >::type* = 0)
      {
        typedef typename inner_iterator_type::base_iterator_type inner_base_iterator;
        inner_base_iterator b = this->base()->begin();
        inner_base_iterator e = this->base()->end();
        if(b == e)
          return false;
        else
        {
          inner = inner_iterator_type(e, e);
          inner.m_begin = b;
          inner.decrement_impl();
          return true;
        }
      }

      bool decrement_impl(
        typename boost::enable_if_convertible<
          typename detail::flatten_iterator_category_tag<Iterator, depth>::type,
          boost::bidirectional_traversal_tag
        >::type* = 0)
      {
        if(inner.decrement_impl())
          return true;
        else
        {
          while(this->base() != this->begin())
          {
            --(this->base_reference());
            if(update_inner_to_end())
              return true;
          }
          return false;
        }
      }

      void decrement(
        typename boost::enable_if_convertible<
          typename detail::flatten_iterator_category_tag<Iterator, depth>::type,
          boost::bidirectional_traversal_tag
        >::type* = 0)
      {
        decrement_impl();
      }

      bool equal(this_iterator const& x) const
      {
        return this->base() == x.base() && inner.equal(x.inner);
      }

      typename super_t::reference dereference() const
      {
        return *inner;
      }

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
