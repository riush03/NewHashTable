#include <stdio.h>
#include <string.h> // for strcmp()
#include <ctype.h> // for isprint()

#include "hashtools.h"

/** check if the two keys are the same */
int
doKeysMatch(AAKeyType key1, size_t key1len, AAKeyType key2, size_t key2len)
{
	/** if the lengths don't match, the keys can't match */
	if (key1len != key2len)
		return 0;

	return memcmp(key1, key2, key1len) == 0;
}

/* provide the hex representation of a value */
static char toHex(int val)
{
	if (val < 10) return (char) ('0' + val);
	return (char) ('a' + (val - 10));
}

/**
 * Provide the key in a printable form.  Uses a static buffer,
 * which means that not only is this not thread-safe, but
 * even runs into trouble if called twice in the same printf().
 *
 * That said, is does provide a memory clean way to give a 
 * printable string return value to the calling code
 */
int
printableKey(char *buffer, int bufferlen, AAKeyType key, size_t printlen)
{
	int i, allChars = 1;
	char *loadptr;


	for (i = 0; allChars && i < printlen; i++) {
		if ( ! isprint(key[i])) allChars = 0;
	}

	if (allChars) {
		snprintf(buffer, bufferlen, "char key:[%s]", (char *) key);
	} else {
		snprintf(buffer, bufferlen, "hex key:[0x");
		loadptr = &buffer[strlen(buffer)];
		for (i = 0; i < printlen && loadptr - buffer < bufferlen - 4; i++) {
			*loadptr++ = toHex((key[i] & 0xf0) >> 4); // top nybble -> first hext digit
			*loadptr++ = toHex(key[i] & 0x0f);        // bottom nybble -> second digit
		}
		*loadptr++ = ']';
		*loadptr++ = 0;
	}
	return 1;
}

/**
 * Calculate a hash value based on the length of the key
 *
 * Calculate an integer index in the range [0...size-1] for
 * 		the given string key
 *
 *  @param  key  key to calculate mapping upon
 *  @param  size boundary for range of allowable return values
 *  @return      integer index associated with key
 *
 *  @see    HashAlgorithm
 */
HashIndex hashByLength(AAKeyType key, size_t keyLength, HashIndex size)
{
	return keyLength % size;
}



/**
 * Calculate a hash value based on the sum of the values in the key
 *
 * Calculate an integer index in the range [0...size-1] for
 * 		the given string key, based on the sum of the values
 *		in the key
 *
 *  param  key  key to calculate mapping upon
 *  param  size boundary for range of allowable return values
 *  return      integer index associated with key
 */
HashIndex hashBySum(AAKeyType key, size_t keyLength, HashIndex size)
{
	HashIndex sum = 0;

	/**
	 * TO DO: you will need to implement a summation based
	 * hashing algorithm here, using a sum-of-bytes
	 * strategy such as that discussed in class.  Take
	 * a look at HashByLength if you want an example
	 * of a "working" (but not very smart) hashing
	 * algorithm.
	 */
    for (size_t i = 0; i < keyLength; i++) {
        sum += key[i];
    }

    return sum % size;
}


/**
 * Locate an empty position in the given array, starting the
 * search at the indicated index, and restricting the search
 * to locations in the range [0...size-1]
 *
 *  @param  index where to begin the search
 *  @param  AssociativeArray associated AssociativeArray we are probing
 *  @param  invalidEndsSearch should the identification of a
 *				KeyDataPair marked invalid end our search?
 *				This is true if we are looking for a location
 *				to insert new data
 *  @return index of location where search stopped, or -1 if
 *				search failed
 *
 *  @see    HashProbe
 */
HashIndex linearProbe(AssociativeArray *hashTable,
		AAKeyType key, size_t keylength,
		int index, int invalidEndsSearch, int *cost
	)
{
	int startIndex = index;  // Store the original index to detect a full table.
	while (1) {
		if (hashTable->table[index].validity == HASH_EMPTY || (invalidEndsSearch && hashTable->table[index].validity != HASH_USED)) {
			return index;  // Found an empty or deleted slot.
		}

		(*cost)++;  // Increment the cost of probing.

		index = (index + 1) % hashTable->size;  // Linear probing: move to the next slot.

		// If we've returned to the original index, the table is full.
		if (index == startIndex) {
			return -1;  // Search failed, the table is full.
		}
	}
}

/**
 * Locate an empty position in the given array, starting the
 * search at the indicated index, and restricting the search
 * to locations in the range [0...size-1]
 *
 *  @param  index where to begin the search
 *  @param  hashTable associated HashTable we are probing
 *  @param  invalidEndsSearch should the identification of a
 *				KeyDataPair marked invalid end our search?
 *				This is true if we are looking for a location
 *				to insert new data
 *  @return index of location where search stopped, or -1 if
 *				search failed
 *
 *  @see    HashProbe
 */
HashIndex quadraticProbe(AssociativeArray *hashTable, AAKeyType key, size_t keylen,
		int startIndex, int invalidEndsSearch,
		int *cost
	)
{
	int index = startIndex;
	int step = 1;
	while (1) {
		if (hashTable->table[index].validity == HASH_EMPTY || (invalidEndsSearch && hashTable->table[index].validity != HASH_USED)) {
			return index;  // Found an empty or deleted slot.
		}

		(*cost)++;  // Increment the cost of probing.

		index = (startIndex + step * step) % hashTable->size;  // Quadratic probing: move to the next slot.
		step++;

		// If we've returned to the original index, the table is full.
		if (index == startIndex) {
			return -1;  // Search failed, the table is full.
		}
	}
}



/**
 * Locate an empty position in the given array, starting the
 * search at the indicated index, and restricting the search
 * to locations in the range [0...size-1]
 *
 *  @param  index where to begin the search
 *  @param  hashTable associated HashTable we are probing
 *  @param  invalidEndsSearch should the identification of a
 *				KeyDataPair marked invalid end our search?
 *				This is true if we are looking for a location
 *				to insert new data
 *  @return index of location where search stopped, or -1 if
 *				search failed
 *
 *  @see    HashProbe
 */
HashIndex doubleHashProbe(AssociativeArray *hashTable, AAKeyType key, size_t keylen,
		int startIndex, int invalidEndsSearch,
		int *cost
	)
{
	/**
	 * TO DO: you will need to implement an algorithm
	 * that calls a second hash function (listed
	 * in the hashTable) and uses the value obtained
	 * as a result from that as the step size.
	 *
	 * Beyond that, the algorithm proceeds as with
	 * the above strategies.
	 */
	HashIndex index = startIndex;
    HashIndex step = 1;
    while (1) {
        if (hashTable->table[index].validity == HASH_EMPTY || (invalidEndsSearch && hashTable->table[index].validity != HASH_USED)) {
            return index; // Found an empty or deleted slot
        }
        (*cost)++;
        HashIndex stepSize = aarray->hashAlgorithmSecondary(key, keylen, aarray->size);
        index = (index + stepSize * step) % aarray->size; // Double hashing: move to the next slot using a second hash
        step++;
        if (index == startIndex) {
            return -1; // Search failed, the table is full
        }
    }
}

