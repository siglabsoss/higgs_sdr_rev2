#include <functional>

////////////////
// modify x in place to add y.  this behaves as expected
//   a = {1,2,3,3}
//   b = {1,2,3,3}
//   c = {}
//   VEC_APPEND(c,a)
//   VEC_APPEND(c,b)
//   c = 1,2,3,7,7
#define VEC_APPEND(x,y) ((x).insert((x).end(), (y).begin(), (y).end()))

///////////////
//
// Does weird reverse adding because we are consuming our lists using .back() and .pop_back()
// see handleDataNegIndex(), inStreamAppend()
#define VEC_R_APPEND(x,y) ((x).insert((x).begin(), (y).rbegin(), (y).rend()))
#define VEC_R_APPEND2(x,y) ((x).insert((x).end(), (y).rbegin(), (y).rend()))

// returns true or false
#define VECTOR_FIND(vector,item) (std::find((vector).begin(), (vector).end(), (item)) != (vector).end())


// FIXME this is Very ugly
// it should be doing += chunk
// see
// https://stackoverflow.com/questions/14226952/partitioning-batch-chunk-a-container-into-equal-sized-pieces-using-std-algorithm
template < typename Iterator >
void for_each_interval(
    Iterator begin
  , Iterator end
  , size_t interval_size
  , std::function<void( Iterator, Iterator )> operation )
{
  auto to = begin;

  while ( to != end )
  {
    auto from = to;

    auto counter = interval_size;
    while ( counter > 0 && to != end )
    {
      ++to;
      --counter;
    }

    operation( from, to );
  }
}


