#ifndef HELPER_H
#define HELPER_H

#include "pin.H"

#define KILO 1024
#define MEGA (KILO*KILO)
#define GIGA (KILO*MEGA)

typedef UINT64 CACHE_STATS; // type of cache hit/miss counters

/*!
 *  @brief Cache tag - self clearing on creation
 */
class CACHE_TAG
{
  private:
    ADDRINT _tag;
    string _cacheLineOwner;

  public:
    //CACHE_TAG(ADDRINT tag = 0) { _tag = tag; }
    CACHE_TAG(ADDRINT tag = 0, string cacheLineOwner = "DEFAULT");
    bool operator==(const CACHE_TAG &right) const;
    ADDRINT addr() const;
    void setCacheLineOwner(std::string cacheLineOwner);
    std::string updateCacheLineOwner(std::string cacheLineOwner);
    std::string getCacheLineOwner();
};

typedef enum 
{
	ACCESS_TYPE_LOAD=0,
	ACCESS_TYPE_STORE=1,
	ACCESS_TYPE_NUM=2
} ACCESS_TYPE;

/*! RMR (rodric@gmail.com) 
 *   - temporary work around because decstr()
 *     casts 64 bit ints to 32 bit ones
 */
std::string local_decstr(UINT64 v, UINT32 w);

/*!
 *  @brief Checks if n is a power of 2.
 *  @returns true if n is power of 2
 */
static inline bool IsPower2(UINT32 n)
{
    return ((n & (n - 1)) == 0);
}

/*!
 *  @brief Computes floor(log2(n))
 *  Works by finding position of MSB set.
 *  @returns -1 if n == 0.
 */
static inline INT32 FloorLog2(UINT32 n)
{
    INT32 p = 0;

    if (n == 0) return -1;

    if (n & 0xffff0000) { p += 16; n >>= 16; }
    if (n & 0x0000ff00)	{ p +=  8; n >>=  8; }
    if (n & 0x000000f0) { p +=  4; n >>=  4; }
    if (n & 0x0000000c) { p +=  2; n >>=  2; }
    if (n & 0x00000002) { p +=  1; }

    return p;
}

/*!
 *  @brief Computes floor(log2(n))
 *  Works by finding position of MSB set.
 *  @returns -1 if n == 0.
 */
static inline INT32 CeilLog2(UINT32 n)
{
    return FloorLog2(n - 1) + 1;
}

uint32_t create_mask(int n);

#define RGEN_MAX UINT_MAX


#endif
