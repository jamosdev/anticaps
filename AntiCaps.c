/**
@file AntiCaps.c
@brief Remove capitalisation in common case to make compressor find more matches

these are reversable. the enc/dec variant pairs are group in difference access patterns.

One is like an array-iterator (encra/decra - random access) and can be used like a getter/setter.

One is an array-to-array input/outbuf buffer (buffer at once, easy to use).

And one is a byte-wise fgetc/fputc style streaming interface which uses a circular buffer state.
DUE TO THIS, to keep things compatible, we have to limit the other modes to not look back further
than this buffer size or else it will not be byte exact output if mix/match variants encode and decode.
on the other hand, the bigger the buffer the more stack space is used. by default it is sized on the
average sentence size, BUT it is most effective so long as it is larger than the biggest word size
you will ever use it only has to look back from the start of the current word plus a bit of white
space to find a period, that is all.

*/
#include <ctype.h>
#include <library/StrStuff/AntiCaps.h>

void anticaps_initstate(struct anticap_stream_microstate* st) {
    st->start = 0;
    st->end = 0;
}
static int getlen(struct anticap_stream_microstate* st) {
    int len;
    if (st->start == st->end) {
        return 0;
    }
    if (st->start < st->end) {
        len = st->end - st->start;
    }
    else {
        len = ((sizeof st->circular) - st->start) + (0 + st->end);
    }
    return len;
}
static char lookback(struct anticap_stream_microstate* st, int negVal) {
    unsigned end = st->end;
    unsigned goBackBy = (negVal<0) ? - negVal : negVal /*should be a usage error exception?*/;
    unsigned len;
    if (goBackBy >= sizeof st->circular) {
        goBackBy = (sizeof st->circular) - 1;
    }
    len = getlen(st);
    if (goBackBy >= len && len>0) {
        goBackBy = len - 1;
    }
    if (len <= 0) {
        return '\0';
    }
    if (end >= goBackBy) {
        return st->circular[end - goBackBy];
    }
    {
        unsigned pos = (sizeof st->circular) - (goBackBy - end);
        return st->circular[pos];
    }
}
void advance(struct anticap_stream_microstate* st, char inbyte) {
    int len;
    st->circular[st->end] = inbyte;
    st->end++;
    if (st->end >= sizeof st->circular) {
        st->end = 0;
    }
    len = getlen(st);
    if (len >= sizeof st->circular) {
        st->start++;
        if (st->start >= sizeof st->circular) {
            st->start = 0;
        }
    }
}
char anticaps_encstream(char inbyte, struct anticap_stream_microstate* st) {
    unsigned sz = sizeof(st->circular);
    int len;
    int i;

    len = getlen(st);
    if (len == 0 && inbyte >= 'A' && inbyte <= 'Z') {
        advance(st, inbyte);
        return tolower(inbyte);
    }
    else if (len == 0 && inbyte >= 'a' && inbyte <= 'z') {
        advance(st, inbyte);
        return toupper(inbyte);
    }

    if ((inbyte >= 'a' && inbyte <= 'z')) {
        for (i = -1; i > (-len); --i) {
            char backc = lookback(st, i);
            if (backc == '.') {
                advance(st, inbyte);
                return toupper(inbyte);
            }
            else if (backc == ' ' || backc == '\r' || backc == '\n' || backc == '\t') {
                continue;
            }
            else {
                break;
            }
        }
        advance(st, inbyte);
        return inbyte;
    }
    else if (inbyte >= 'A' && inbyte <= 'Z') {
        for (i = -1; i > (-len); --i) {
            char backc = lookback(st, i);
            if (backc == '.') {
                advance(st, inbyte);
                return tolower(inbyte);
            }
            else if (backc == ' ' || backc == '\r' || backc == '\n' || backc != '\t') {
                continue;
            }
            else {
                break;
            }
        }
        advance(st, inbyte);
        return inbyte;
    }
    advance(st, inbyte);
    return inbyte;
}
char anticaps_decstream(char inbyte, struct anticap_stream_microstate* st) {
    int len;
    len = getlen(st);
    if (len == 0) {
        if (inbyte >= 'a' && inbyte <= 'z') {
            advance(st, inbyte);
            return toupper(inbyte);
        }
        else if (inbyte >= 'A' && inbyte <= 'Z') {
            advance(st, inbyte);
            return tolower(inbyte);
        }
    }
    advance(st, inbyte);
    return inbyte;
}
char anticaps_encra(const char*base, size_t index){
    char c = base[index];
    int count;
    if(c >= 'a' && c <= 'z' && index>0) {
        if(index==0) {
            return toupper(c);
        }
        count=0;
        while(index>0) {
            if(count>ANTICAPS_CIRCULAR_SZ) break;
            else ++count;
            --index;
            if(base[index] == '.') {
                return toupper(c);
            } else if(base[index] != ' ' && base[index] != '\r' && base[index] != '\n' && base[index] != '\t') {
                break;
            }
        }
        return c;
    } else if (c >= 'A' && c <= 'Z') {
        if(index==0) {
            return tolower(c);
        }
        count=0;
        while(index>0){
            if(count>ANTICAPS_CIRCULAR_SZ) break;
            else ++count;
            --index;
            if(base[index]=='.'){
                return tolower(c);
            } else if(base[index] != ' ' && base[index] != '\r' && base[index] != '\n' && base[index] != '\t') {
                break;
            }
        }
        return c;
    }
    return c;//'^';
}
char anticaps_decra(const char*base, size_t index){
    char c=base[index];
    int count;
    if(index==0 && c>='a' && c<='z') {
        return toupper(c);
    }
    if(index==0 && c>='A' && c<='Z') {
        return tolower(c);
    }
    if(!((c >= 'a' && c <= 'z')||(c >= 'A' && c <= 'Z'))) {
        return c;
    }
    if(c>='a' && c<='z' && index>0) {
        count=0;
        while(index>0){
            if(count>ANTICAPS_CIRCULAR_SZ) break;
            else ++count;
            --index;
            if(base[index]=='.') {
                return toupper(c);
            } else if(base[index]!=' '&&base[index]!='\t'&&base[index]!='\r'&&base[index]!='\n') {
                break;
            }
        }
        return c;
    } else if(c>='A' && c<='Z' && index>0){
        count=0;
        while(index>0){
            if(count>ANTICAPS_CIRCULAR_SZ) break;
            else ++count;
            --index;
            if(base[index]=='.'){
                return tolower(c);
            } else if(base[index]!=' '&&base[index]!='\t'&&base[index]!='\r'&&base[index]!='\n') {
                break;
            }
            return c;
        }
        return c;
    }
    return '?';
}
/*#define ANTICAPS_ALLCAPS_MARKER(BYTE) (0x10000000 | (BYTE&0x000000FF))*/
/*#define ANTICAPS_TOGCAPS_MARKER(BYTE) (0x01000000 | (BYTE&0x000000FF)<<8)*/
size_t anticaps_encbuf2buf(const char*in, size_t inlenOr0, char *out, size_t outsz, unsigned flags){
    return 0;
}
size_t anticaps_decbuf2buf(const char*in, size_t inlenOr0, char *out, size_t outsz, unsigned flags){
    return 0;
}

