// -*- C++ -*- Time-stamp: <96/04/09 13:28:53 ptr>
#ifndef __LA_tmp_h
#define __LA_tmp_h

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

template <class ForwardIterator1, class ForwardIterator2,class T>
inline
void __iter_swap( ForwardIterator1 a, ForwardIterator2 b, T * )
{
  T tmp = *a;
  *a = *b;
  *b = tmp;
}

template <class ForwardIterator1, class ForwardIterator2>
inline
void iter_swap(ForwardIterator1 a, ForwardIterator2 b)
{
  __iter_swap(a, b, &(*a) );
}

template <class BidirectionalIterator>
void reverse(BidirectionalIterator first, BidirectionalIterator last)
{
  while ( first < last )
    iter_swap( first++, --last );
}

template <class InputIterator, class OutputIterator>
OutputIterator copy(InputIterator first, InputIterator last,
		    OutputIterator result) {
    while (first != last) *result++ = *first++;
    return result;
}

template <class BidirectionalIterator1, class BidirectionalIterator2>
BidirectionalIterator2 copy_backward(BidirectionalIterator1 first, 
				     BidirectionalIterator1 last, 
				     BidirectionalIterator2 result) {
    while (first != last) *--result = *--last;
    return result;
}

template <class ForwardIterator, class T>
void fill(ForwardIterator first, ForwardIterator last, const T& value) {
    while (first != last) *first++ = value;
}

#endif // __LA_tmp_h
