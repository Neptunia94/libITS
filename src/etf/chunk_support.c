#include <ctype.h>
#include "chunk_support.h"
#include "runtime.h"

// Comment this line to get debug output
#define eWarning if(0) Warning


#define VALID_IDX(i,c) if (i>=c.len) Fatal(1,error,"chunk overflow")

static const char HEX[16]="0123456789ABCDEF";

void chunk_encode_copy(chunk dst,chunk src,char esc){
	chunk_len k=0;
	for(chunk_len i=0;i<src.len;i++){
		if (isprint(src.data[i])) {
			if (src.data[i]==esc){
				VALID_IDX(k+1,dst);
				dst.data[k]=esc;
				dst.data[k+1]=esc;
				k+=2;
				VALID_IDX(k,dst);
			} else {
				VALID_IDX(k,dst);
				dst.data[k]=src.data[i];
				k++;
			}
		} else {
			VALID_IDX(k+2,dst);
			dst.data[k]=esc;
			dst.data[k+1]=HEX[(src.data[i]>>4)&(char)0x0F];
			dst.data[k+2]=HEX[src.data[i]&(char)0x0F];
			k+=3;
		}
	}
	if(k<dst.len) {
		dst.data[k]=0;
	}
}

static int hex_decode(char c){
	switch(c){
	case '0'...'9': return (c-'0');
	case 'a'...'f': return (10+c-'a');
	case 'A'...'F': return (10+c-'A');
	default: Fatal(1,error,"%c is not a hex digit",c); return 0;
	}
}

void chunk_decode_copy(chunk dst,chunk src,char esc){
	chunk_len i=0,j=0;
	while(i<src.len){
		if (src.data[i]==0) break;
		VALID_IDX(j,dst);
		if (src.data[i]==esc){
			if (src.data[i+1]==esc){
				dst.data[j]=esc;
				i+=2;
			} else {
				dst.data[j]=(hex_decode(src.data[i+1])<<4)+hex_decode(src.data[i+2]);
				i+=3;
			}
		} else {
			dst.data[j]=src.data[i];
			i++;
		}
		j++;
	}
	dst.len=j;
}

void chunk2string(chunk src,size_t dst_size,char*dst){
	eWarning(debug,"encoding chunk of length %d",src.len);
/* verbatim disabled
	int verbatim=src.len;
	if (src.len==0 || (src.data[0]=='#' && src.data[src.len-1]=='#')) {
		verbatim=0;
	} else {
		for(uint32_t i=0;i<src.len;i++){
			if (!isprint(src.data[i]) || isspace(src.data[i])) {
				verbatim=0;
				break;
			}
		}
		if (verbatim){
			int number=1;
			for(uint32_t i=0;i<src.len;i++){
				if (!isdigit(src.data[i])){
					number=0;
					break;
				}
			}
			if (number) verbatim=0;
		}
	}
	if (verbatim) {
		if (dst_size < src.len+1) Fatal(1,error,"chunk overflow");
		for(uint32_t i=0;i<src.len;i++){
			dst[i]=src.data[i];
		}
		dst[src.len]=0;
		return;
	}
*/
	int quotable=1;
	for(uint32_t i=0;i<src.len;i++){
		if (!isprint(src.data[i])) {
			quotable=0;
			break;
		}
	}
	if (quotable){
		unsigned int k=0;
#define PUT_CHAR(c) { if ( dst_size <= k ) { Fatal(1,error,"chunk overflow"); } else { dst[k]=c; k++ ; } }
		PUT_CHAR('"');
		for(uint32_t i=0;i<src.len;i++){
			switch(src.data[i]){
			case '"': PUT_CHAR('\\'); PUT_CHAR('"'); continue;
			case '\\': PUT_CHAR('\\'); PUT_CHAR('\\'); continue;
			default: PUT_CHAR(src.data[i]);
			}
		}
		PUT_CHAR('"');
		PUT_CHAR(0);
#undef PUT_CHAR
	} else {
		if (dst_size < 2*src.len+3) Fatal(1,error,"chunk overflow");
		dst[0]='#';
		for(uint32_t i=0;i<src.len;i++){
			dst[2*i+1]=HEX[(src.data[i]>>4)&(char)0x0F];;
			dst[2*i+2]=HEX[src.data[i]&(char)0x0F];
		}
		dst[2*src.len+1]='#';
		dst[2*src.len+2]=0;
	}
}

void string2chunk(char*src,chunk *dst){
	uint32_t len=strlen(src);
	eWarning(debug,"decoding \"%s\" (%d)",src,len);
	if (src[0]=='#' && src[len-1]=='#') {
		eWarning(debug,"hex");
		len=len/2 - 1;
		if (dst->len<len) Fatal(1,error,"chunk overflow");
		dst->len=len;
		for(uint32_t i=0;i<len;i++){
			dst->data[i]=(hex_decode(src[2*i+1])<<4)+hex_decode(src[2*i+2]);
		}
	} else if (src[0]=='"' && src[len-1]=='"') {
		eWarning(debug,"quoted");
		len=len-2;
		if (dst->len<len) Fatal(1,error,"chunk overflow");
		dst->len=0;
#define PUT_CHAR(c) { dst->data[dst->len]=c; dst->len++ ; }
		for(uint32_t i=0;i<len;i++) {
			if (!isprint(src[i+1])) Fatal(1,error,"non-printable character in source");
			if (src[i+1]=='"') Fatal(1,error,"unquoted \" in string");
			if (src[i+1]=='\\') {
				if (i+1==len) Fatal(1,error,"bad escape sequence");
				switch(src[i+2]){
				case '\\': PUT_CHAR('\\'); i++; continue;
				case '"': PUT_CHAR('"'); i++; continue;
				default: Fatal(1,error,"unknown escape sequence");
				}
			}
			PUT_CHAR(src[i+1]);
		}
#undef PUT_CHAR
	} else {
		eWarning(debug,"verbatim");
		if (dst->len<len) Fatal(1,error,"chunk overflow");
		dst->len=len;
		for(uint32_t i=0;i<len;i++) {
			if (!isprint(src[i])) Fatal(1,error,"non-printable character in source");
			if (isspace(src[i])) Fatal(1,error,"white space in source");
			dst->data[i]=src[i];
		}
	}
	eWarning(debug,"return length is %d",dst->len);
}



