#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <library/StrStuff/AntiCaps.h>
#include <stdlib.h>

void usage(void){
    fprintf(stderr,"usage: anticaps.exe MODE blah blah blah\n   or: anticaps.exe MODE infile.txt\nin both cases, output to stdout (redirect to file if you need)\nMODE is one of \"stream\", \"buf255254\", \"destream\", \"debuf255254\", \"iterator\",\"deiterator\".\n");
}
int main(int argc,char**argv){
    FILE *f;
    const char * modestring=NULL;
    int mode=0;
    char *inputString = NULL;
    if(argc<2) {
        usage();
        return __LINE__;
    }
    modestring = argv[1];
    if(strcmp(modestring,"stream")==0||strcmp(modestring,"STEAM")==0||strcmp(modestring,"Stream")==0) {
        mode=1;
    } else if(strcmp(modestring,"buf255254")==0||strcmp(modestring,"BUF255254")==0||strcmp(modestring,"Buf255254")==0||strcmp(modestring,"buff255254")==0||strcmp(modestring,"BUFF255254")==0||strcmp(modestring,"Buff255254")==0) {
        mode=2;
    } else if(strcmp(modestring,"destream")==0||strcmp(modestring,"DESTEAM")==0||strcmp(modestring,"Destream")==0||strcmp(modestring,"DeStream")==0) {
        mode=-1;
    } else if(strcmp(modestring,"debuf255254")==0||strcmp(modestring,"DEBUF255254")==0||strcmp(modestring,"DeBuf255254")==0||strcmp(modestring,"Debuf255254")==0||strcmp(modestring,"debuff255254")==0||strcmp(modestring,"DEBUFF255254")==0||strcmp(modestring,"DeBuff255254")==0||strcmp(modestring,"Debuff255254")==0) {
        mode=-2;
    } else if (strcmp(modestring, "iterator") == 0 || strcmp(modestring, "ITERATOR") == 0 || strcmp(modestring, "Iterator") == 0) {
        mode = 3;
    }
    else if (strcmp(modestring, "deiterator") == 0 || strcmp(modestring, "DEITERATOR") == 0 || strcmp(modestring, "DeIterator") == 0 || strcmp(modestring, "Deiterator") == 0) {
        mode = -3;
    }
    if(mode!=1 && mode!=2 && mode!=-1 && mode!=-2 && mode != 3 && mode != -3) {
        usage();
        fprintf(stderr,"wtf is '%s'?\n",modestring);
        return __LINE__;
    }
    f = fopen(argv[2],"r");
    if(!f) {
        int i;
        size_t n;
        size_t tot=0;
        for(i=2;i<argc;++i){
            tot+=strlen(argv[i])+1;
        }
        inputString=malloc(tot);
        if(inputString==NULL){
            perror("cant alloc for argvs combiner");
            return __LINE__;
        }
        n = 0;
        for(i = 2; i < argc; ++i){
            memcpy(inputString + n, argv[i], strlen(argv[i]));
            inputString[n + strlen(argv[i])] = (i==argc-1) ? '\0' : ' ';
            n += strlen(argv[i])+1;
        }
    }
    else {
        long len;
        fseek(f, 0, SEEK_END);
        len = ftell(f);
        if (len < 1) {
            perror("bad file length");
            return __LINE__;
        }
        inputString = malloc(len + 1);
        if (inputString == NULL) {
            perror("failed to alloc");
            return __LINE__;
        }
        fseek(f, 0, SEEK_SET);
        fread(inputString, 1, len, f);
        inputString[len] = '\0';
        fclose(f);
    }
    switch(mode) {
    case 1:
        {
            struct anticap_stream_microstate ms;
            char* izard = inputString;
            anticaps_initstate(&ms);
            while (*izard != '\0') {
                char outc = anticaps_encstream(*izard, &ms);
                fputc(outc, stdout); fflush(stdout);
                ++izard;
            }
        }
        break;
    case 3:
        {
            size_t n = 0;
            for(n = 0; n < strlen(inputString); ++n){
                char c = anticaps_encra(inputString,n);
                fputc(c, stdout);
            }
            fputc('\n', stdout);
        }
        break;
    case -3:
        {
            size_t n=0;
            for(n = 0; n < strlen(inputString); ++n){
                char c = anticaps_decra(inputString,n);
                fputc(c, stdout);
            }
            fputc('\n', stdout);
        }
    case 2:
        {
            size_t obsz = strlen(inputString) + 100;
            char * outbuf = malloc(obsz);
            size_t wrote;
            if(outbuf == NULL){
                perror("cant alloc outbuf case 2");
                return __LINE__;
            }
            wrote = anticaps_encbuf2buf(inputString, 0, outbuf, obsz, ANTICAPS_ALLCAPS_MARKER(255) | ANTICAPS_TOGCAPS_MARKER(254));
            outbuf[wrote] = '\0';
        }
        break;
    case -2:
        break;
    }
    return 0;
}
