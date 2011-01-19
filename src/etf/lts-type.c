#include <string.h>

#include "etf/runtime.h"
#include "etf/lts-type.h"
#include "etf/stringindex.h"

struct lts_type_s {
int state_length;
char** state_name;
int* state_type;
int state_label_count;
char** state_label_name;
int* state_label_type;
int edge_label_count;
char** edge_label_name;
int* edge_label_type;
string_index_t type_db;
};


lts_type_t lts_type_create(){
	lts_type_t ltstype=(lts_type_t)RTmalloc(sizeof(struct lts_type_s));
	ltstype->state_length=0;
	ltstype->state_name=NULL;
	ltstype->state_type=NULL;
	ltstype->state_label_count=0;
	ltstype->state_label_name=NULL;
	ltstype->state_label_type=NULL;
	ltstype->edge_label_count=0;
	ltstype->edge_label_name=NULL;
	ltstype->edge_label_type=NULL;
	ltstype->type_db=SIcreate();
	return ltstype;
}

#define STRDUP(s) (s==NULL)?NULL:strdup(s);

lts_type_t lts_type_permute(lts_type_t t0,int *pi){
    lts_type_t t=RT_NEW(struct lts_type_s);
    
    // clone state signature
	t->state_length=t0->state_length;
	t->state_name=(char**)RTmalloc(t0->state_length*sizeof(char*));
	t->state_type=(int*)RTmalloc(t0->state_length*sizeof(int));
	for(int i=0;i<t0->state_length;i++){
		t->state_name[i]=STRDUP(t0->state_name[pi[i]]);
		t->state_type[i]=t0->state_type[pi[i]];
	}
	// clone state label signature
	t->state_label_count=t0->state_label_count;
	t->state_label_name=(char**)RTmalloc(t->state_label_count*sizeof(char*));
	t->state_label_type=(int*)RTmalloc(t->state_label_count*sizeof(int));
	for(int i=0;i<t0->state_label_count;i++){
	    t->state_label_name[i]=strdup(t0->state_label_name[i]);
	    t->state_label_type[i]=t0->state_label_type[i];
	}
	// clone edge label signature
	t->edge_label_count=t0->edge_label_count;
	t->edge_label_name=(char**)RTmalloc(t0->edge_label_count*sizeof(char*));
	t->edge_label_type=(int*)RTmalloc(t0->edge_label_count*sizeof(int));
	for(int i=0;i<t0->edge_label_count;i++){
	    t->edge_label_name[i]=strdup(t0->edge_label_name[i]);
	    t->edge_label_type[i]=t0->edge_label_type[i];
	}
	// clone type numbering
	t->type_db=SIcreate();
	int N=SIgetCount(t0->type_db);
	for(int i=0;i<N;i++){
	    SIputAt(t->type_db,SIget(t0->type_db,i),i);
	}
    return t;
}
/*
void lts_type_print(log_t log, lts_type_t t){
    log_printf(log,"The state vector is:\n");
 	for(int i=0;i<t->state_length;i++){
 	    log_printf(log,"%4d: %s:%s\n",i,
                   t->state_name[i],
                   SIget(t->type_db,t->state_type[i]));
	}
	log_printf(log,"The state labels are:\n");
	for(int i=0;i<t->state_label_count;i++){
	    log_printf(log,"%4d: %s:%s\n",i,
                   t->state_label_name[i],
                   SIget(t->type_db,t->state_label_type[i]));
	}
	log_printf(log,"The edge labels are:\n");
	for(int i=0;i<t->edge_label_count;i++){
	    log_printf(log,"%4d: %s:%s\n",i,
                   t->edge_label_name[i],
                   SIget(t->type_db,t->edge_label_type[i]));
	}
    log_printf(log,"The registered types are:\n");
 	int N=SIgetCount(t->type_db);
	for(int i=0;i<N;i++){
	    log_printf(log,"%4d: %s\n",i,SIget(t->type_db,i));
	}
}
*/
void lts_type_destroy(lts_type_t *t){
	Warning(info,"Need to define ownership and implement destroy of LTS type.");
	free(*t);
	*t=NULL;
}

void lts_type_set_state_length(lts_type_t  t,int length){
	t->state_length=length;
	t->state_name=(char**)RTmalloc(length*sizeof(char*));
	t->state_type=(int*)RTmalloc(length*sizeof(int));
	for(int i=0;i<length;i++){
		t->state_name[i]=NULL;
		t->state_type[i]=SI_INDEX_FAILED;
	}
}

int lts_type_get_state_length(lts_type_t  t){
	return t->state_length;
}

void lts_type_set_state_name(lts_type_t  t,int idx,const char* name){
	t->state_name[idx]=strdup(name);
}

char* lts_type_get_state_name(lts_type_t  t,int idx){
	return t->state_name[idx];
}

void lts_type_set_state_type(lts_type_t  t,int idx,const char* name){
	t->state_type[idx]=SIput(t->type_db,name);
}

char* lts_type_get_state_type(lts_type_t  t,int idx){
	if(t->state_type[idx]==SI_INDEX_FAILED) {
		return NULL;
	} else {
		return SIget(t->type_db,t->state_type[idx]);
	}
}

void lts_type_set_state_typeno(lts_type_t  t,int idx,int typeno){
	t->state_type[idx]=typeno;
}

int lts_type_get_state_typeno(lts_type_t  t,int idx){
	return t->state_type[idx];
}

void lts_type_set_state_label_count(lts_type_t  t,int count){
	t->state_label_count=count;
	t->state_label_name=(char**)RTmalloc(count*sizeof(char*));
	t->state_label_type=(int*)RTmalloc(count*sizeof(int));
}
int lts_type_get_state_label_count(lts_type_t  t){
	return t->state_label_count;
}
void lts_type_set_state_label_name(lts_type_t  t,int label,const char*name){
	t->state_label_name[label]=strdup(name);
}
void lts_type_set_state_label_type(lts_type_t  t,int label,const char*name){
	t->state_label_type[label]=SIput(t->type_db,name);
}
void lts_type_set_state_label_typeno(lts_type_t  t,int label,int typeno){
	t->state_label_type[label]=typeno;
}
char* lts_type_get_state_label_name(lts_type_t  t,int label){
	return t->state_label_name[label];
}
char* lts_type_get_state_label_type(lts_type_t  t,int label){
	if(t->state_label_type[label]==SI_INDEX_FAILED) {
		return NULL;
	} else {
		return SIget(t->type_db,t->state_label_type[label]);
	}
}
int lts_type_get_state_label_typeno(lts_type_t  t,int label){
	return t->state_label_type[label];
}


void lts_type_set_edge_label_count(lts_type_t  t,int count){
	t->edge_label_count=count;
	t->edge_label_name=(char**)RTmalloc(count*sizeof(char*));
	t->edge_label_type=(int*)RTmalloc(count*sizeof(int));
}
int lts_type_get_edge_label_count(lts_type_t  t){
	return t->edge_label_count;
}
void lts_type_set_edge_label_name(lts_type_t  t,int label,const char*name){
	t->edge_label_name[label]=strdup(name);
}
void lts_type_set_edge_label_type(lts_type_t  t,int label,const char*name){
	t->edge_label_type[label]=SIput(t->type_db,name);
}
void lts_type_set_edge_label_typeno(lts_type_t  t,int label,int typeno){
	t->edge_label_type[label]=typeno;
}
char* lts_type_get_edge_label_name(lts_type_t  t,int label){
	return t->edge_label_name[label];
}
char* lts_type_get_edge_label_type(lts_type_t  t,int label){
	if(t->edge_label_type[label]==SI_INDEX_FAILED) {
		return NULL;
	} else {
		return SIget(t->type_db,t->edge_label_type[label]);
	}
}
int lts_type_get_edge_label_typeno(lts_type_t  t,int label){
	return t->edge_label_type[label];
}


int lts_type_get_type_count(lts_type_t  t){
	return SIgetCount(t->type_db);
}

int lts_type_add_type(lts_type_t  t,const char *name,int *is_new){
	if (is_new) {
		int type_no=SIlookup(t->type_db,name);
		if (type_no!=SI_INDEX_FAILED) {
			*is_new=0;
			return type_no;
		}
		*is_new=1;
	}
	return SIput(t->type_db,name);
}

char* lts_type_get_type(lts_type_t  t,int typeno){
	return SIget(t->type_db,typeno);
}

void lts_type_validate(lts_type_t t){
    if (t->state_length <= 0)
      fprintf(stderr,"unsupported state vector of size %d", t->state_length);
    for(int i=0;i<t->state_length;i++){
        if (t->state_name[i]==NULL) fprintf(stderr,"name of state variable %d undefined",i);
        if (t->state_type[i]<0) fprintf(stderr,"type of state variable %d undefined",i);
        if (SIget(t->type_db,t->state_type[i])==NULL) {
            fprintf(stderr,"type %d used for state variable %d, but undefined",t->state_type[i],i);
        }
    }
    for(int i=0;i<t->state_label_count;i++){
        if (t->state_label_name[i]==NULL) fprintf(stderr,"name of defined variable %d undefined",i);
        if (t->state_label_type[i]<0) fprintf(stderr,"type of defined variable %d undefined",i);
        if (SIget(t->type_db,t->state_label_type[i])==NULL) {
            fprintf(stderr,"type %d used for state label %d, but undefined",t->state_label_type[i],i);
        }
    }
    for(int i=0;i<t->edge_label_count;i++){
        if (t->edge_label_name[i]==NULL) fprintf(stderr,"name of edge label %d undefined",i);
        if (t->edge_label_type[i]<0) fprintf(stderr,"type of edge label %d undefined",i);
        if (SIget(t->type_db,t->edge_label_type[i])==NULL) {
            fprintf(stderr,"type %d used for edge label %d, but undefined",t->edge_label_type[i],i);
        }
    }
}

void lts_type_serialize(lts_type_t t,stream_t ds){
	DSwriteS(ds,"lts signature 1.0");
	uint32_t N=lts_type_get_state_length(t);
	Warning(debug,"state length is %d",N);
	DSwriteU32(ds,N);
	for(uint32_t i=0;i<N;i++){
		char*x=lts_type_get_state_name(t,i);
		if (x) DSwriteS(ds,x); else DSwriteS(ds,"");
		DSwriteU32(ds,lts_type_get_state_typeno(t,i));
	}
	N=lts_type_get_state_label_count(t);
	Warning(info,"%d state labels",N);
	DSwriteU32(ds,N);
	for(uint32_t i=0;i<N;i++){
		char*x=lts_type_get_state_label_name(t,i);
		if (x) DSwriteS(ds,x); else DSwriteS(ds,"");
		DSwriteU32(ds,lts_type_get_state_label_typeno(t,i));
	}
	N=lts_type_get_edge_label_count(t);
	Warning(info,"%d edge labels",N);
	DSwriteU32(ds,N);
	for(uint32_t i=0;i<N;i++){
		char*x=lts_type_get_edge_label_name(t,i);
		if (x) DSwriteS(ds,x); else DSwriteS(ds,"");
		DSwriteU32(ds,lts_type_get_edge_label_typeno(t,i));
		Warning(debug,"edge label %d is %s : %s",i,x,lts_type_get_edge_label_type(t,i));
	}
	N=lts_type_get_type_count(t);
	Warning(info,"%d types",N);
	DSwriteU32(ds,N);
	for(uint32_t i=0;i<N;i++){
		DSwriteS(ds,lts_type_get_type(t,i));
	}
}

lts_type_t lts_type_deserialize(stream_t ds){
	lts_type_t t=lts_type_create();
	char version[1024];
	DSreadS(ds,version,1024);
	if (strcmp(version,"lts signature 1.0")){
		Fatal(1,error,"cannot deserialize %s",version);
	}
	uint32_t N=DSreadU32(ds);
	Warning(info,"state length is %d",N);
	lts_type_set_state_length(t,N);
	for(uint32_t i=0;i<N;i++){
		char*x=DSreadSA(ds);
		if (strlen(x)) lts_type_set_state_name(t,i,x);
		free(x);
		lts_type_set_state_typeno(t,i,DSreadU32(ds));
	}
	N=DSreadU32(ds);
	Warning(info,"%d state labels",N);
	lts_type_set_state_label_count(t,N);
	for(uint32_t i=0;i<N;i++){
		char*x=DSreadSA(ds);
		if (strlen(x)) lts_type_set_state_label_name(t,i,x);
		free(x);
		lts_type_set_state_label_typeno(t,i,DSreadU32(ds));
	}
	N=DSreadU32(ds);
	Warning(info,"%d edge labels",N);
	lts_type_set_edge_label_count(t,N);
	for(uint32_t i=0;i<N;i++){
		char*x=DSreadSA(ds);
		if (strlen(x)) lts_type_set_edge_label_name(t,i,x);
		free(x);
		lts_type_set_edge_label_typeno(t,i,DSreadU32(ds));
	}
	N=DSreadU32(ds);
	Warning(info,"%d types",N);
	for(uint32_t i=0;i<N;i++){
		char*x=DSreadSA(ds);
		SIputAt(t->type_db,x,i);
		free(x);
	}
	return t;
}
