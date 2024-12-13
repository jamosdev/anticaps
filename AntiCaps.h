/**
@file AntiCaps.h
@brief text filter caps preprocessing

*/
#ifndef library_StrStuff_AntiCaps_h
#define library_StrStuff_AntiCaps_h
#include <stdint.h>//size_t
#ifndef AVG_CHARAS_PER_WORD
/** it is said to be 4.7, we will go 6 for safety */
#define AVG_CHARAS_PER_WORD    (5+1)
#endif
#ifndef AVG_WORDS_PER_SENTENCE
/* it is said to be 25, we will go 30 for safety */
#define AVG_WORDS_PER_SENTENCE (25+5)
#endif
/** we need this separately for limiting the other accessor variants for max compatibility */
#define ANTICAPS_CIRCULAR_SZ AVG_WORDS_PER_SENTENCE * (AVG_CHARAS_PER_WORD+1)
struct anticap_stream_microstate {
	char circular[ANTICAPS_CIRCULAR_SZ]; /**< it is +1 per word for the space between them */
	unsigned start;
	unsigned end;
};
void anticaps_initstate(struct anticap_stream_microstate* st);
char anticaps_encstream(char inbyte, struct anticap_stream_microstate* st);
char anticaps_decstream(char inbyte, struct anticap_stream_microstate* st);
/** mostly stateless and pretty much random accesds */
char anticaps_encra(const char*base, size_t index);
char anticaps_decra(const char*base, size_t index);
/** usually give this with value 255 */
#define ANTICAPS_ALLCAPS_MARKER(BYTE) (0x10000000 | (BYTE&0x000000FF))
/** usually give this with value 254 */
#define ANTICAPS_TOGCAPS_MARKER(BYTE) (0x01000000 | (BYTE&0x000000FF)<<8)
/** if outbuf doesn't have enough space (there will usually be size expansion with prefix bytes)
for how much of the input buffer is remaining vs how much the spare output buffer is remaining
then it just disables prefix bytes from that point on and it will still output all data but there
will be more caps than there needs to be */
size_t anticaps_encbuf2buf(const char*in, size_t inlenOr0, char *out, size_t outsz, unsigned flags);
size_t anticaps_decbuf2buf(const char*in, size_t inlenOr0, char *out, size_t outsz, unsigned flags);
#endif//library_StrStuff_AntiCaps_h
