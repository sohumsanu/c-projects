#include "mbstrings.h"

/* mbslen - multi-byte string length
 * - Description: returns the number of UTF-8 code points ("characters")
 * in a multibyte string. If the argument is NULL or an invalid UTF-8
 * string is passed, returns -1.
 *
 * - Arguments: A pointer to a character array (`bytes`), consisting of UTF-8
 * variable-length encoded multibyte code points.
 *
 * - Return: returns the actual number of UTF-8 code points in `src`. If an
 * invalid sequence of bytes is encountered, return -1.
 *
 * - Hints:
 * UTF-8 characters are encoded in 1 to 4 bytes. The number of leading 1s in the
 * highest order byte indicates the length (in bytes) of the character. For
 * example, a character with the encoding 1111.... is 4 bytes long, a character
 * with the encoding 1110.... is 3 bytes long, and a character with the encoding
 * 1100.... is 2 bytes long. Single-byte UTF-8 characters were designed to be
 * compatible with ASCII. As such, the first bit of a 1-byte UTF-8 character is
 * 0.......
 *
 * You will need bitwise operations for this part of the assignment!
 */
size_t mbslen(const char* bytes) {
    // TODO: implement!
    size_t count = 0;
    for(int i = 0; bytes[i] != '\0'; i++){
        if((bytes[i] & 0b11111000) == 0b11110000){
            count += 1;
            i+=3;
        } else if ((bytes[i] & 0b11110000) == 0b11100000){
            count +=1;
            i+=2;
        } else if ((bytes[i] & 0b11100000) == 0b11000000){
            count +=1;
            i+=1;
        } else if ((bytes[i] & 0b10000000) == 0b00000000){
            count +=1;
        } else {
            return -1;
        }

    }
    return count;
}
