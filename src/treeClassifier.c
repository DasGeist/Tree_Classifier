/*
TreeClassifier.c - A simple implementation of an entropy-based decision-tree classifer
Copyright (c) 2020 Sérgio F. da S. Jr.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "treeClassifier.h"

/*
This macro might make some of the code slightly more easily readable
*/
#define foreach(var,code) while(var){(code);var=var->next;}

tree_ll *ll_push(tree_ll** list,void* item)
{
    tree_ll* prev=NULL;
    if(!list)return NULL;
    /*Address of the pointer at the end of the list*/
    while((*list)!=NULL)
    {
        prev=*list;
        list=&(*list)->next;
    }
    /*And put our item there*/
    *list=malloc(sizeof(tree_ll));
    (*list)->next=NULL;
    (*list)->prev=prev;
    (*list)->self=item;
    return *list;
}
void* ll_pop(tree_ll **list)
{
    void* ret;
    if(!list||!(*list))return NULL;
    /*We find the last node of the list*/
    while((*list)->next!=NULL)
        list=&(*list)->next;
    ret=(*list)->self;
    if((*list)->prev!=NULL)
        (*list)->prev->next=NULL;
    free(*list);
    *list=NULL;
    return ret;
}
void ll_free(tree_ll **list)
{
    if(!list||!(*list))return;
    if((*list)->next!=NULL)ll_free(&(*list)->next);
    free(*list);
    *list=NULL;
}
void ll_free_self(tree_ll **list)
{
    if(!list||!(*list))return;
    if((*list)->next!=NULL)ll_free(&(*list)->next);
    free(*list);
    if((*list)->self)free((*list)->self);
    (*list)->self=NULL;
    *list=NULL;
}
tree_ll* ll_search(tree_ll** list,char func(void* item,void* arg),void* arg)
{
    if(!list||!(*list)||!func)return NULL;
    if(func((*list)->self,arg))return *list;
    else return ll_search(&(*list)->next,func,arg);
}
int ll_len(tree_ll** list)
{
    int l=0;
    if(!list||!(*list))return l;
    return ll_len(&(*list)->next)+1;
}
tree_ll** ll_to_array(tree_ll** list)
{
    tree_ll** ret;
    int l=ll_len(list),i;
    if(l==0)return NULL;
    ret=malloc(sizeof(tree_ll*)*l);
    for(i=0;i<l;i++)
    {
        ret[i]=(*list);
        list=&(*list)->next;
    }
    return ret;
}
tree_ll* array_to_ll(tree_ll** array,int len)
{
    int i;
    if(len==0)return NULL;
    array[0]->prev=NULL;
    array[0]->next=array[1];
    for(i=1;i<len-1;i++)
    {
        array[i]->prev=array[i-1];
        array[i]->next=array[i+1];
    }
    array[len-1]->prev=array[len-2];
    array[len-1]->next=NULL;
    return array[0];
}

char findLabel(void* lab,void* labname)
{
    return strcmp(((label*)lab)->name,(char*)labname)==0;
}

label* Label(char* name,char type)
{
    if(!name)return NULL;
    label* ret=malloc(sizeof(label));
    strcpy(ret->name,name);
    ret->type=type;
    ret->sublabels=NULL;
    return ret;
}

dataset* csv_to_dataset(const char* fname)
{
    if(!fname)return NULL;
    FILE* fp=fopen(fname,"r");
    if(!fp)return NULL;
    dataset* ret=malloc(sizeof(dataset));
    char curItem[64];
    char *tmp;
    char *ln;
    double titem;
    double *item;
    tree_ll* entry;
    tree_ll* current_label;
    tree_ll* srch;
    int line=3;
    ret->col_labels=NULL;
    ret->lines=NULL;
    /*We use the first line our col_labels*/
    while(fscanf(fp,"%[^,\n]",curItem))
    {
        ln=malloc(64);
        strncpy(ln,curItem,64);
        ll_push(&ret->col_labels,Label(ln,LABEL_CAT));
        if(fgetc(fp)=='\n')break;
    }
    /*Then we scan the first line*/
    entry=NULL;
    current_label=ret->col_labels;
    while(fscanf(fp,"%[^,\n]",curItem)>0)
    {
        titem=strtod(curItem,&tmp);
        if(*tmp!=0)
        {
            /*If it's not a number, we create a label for it*/
            ln=malloc(64);
            strncpy(ln,curItem,64);
            srch=ll_push(&((label*)current_label->self)->sublabels,Label(ln,LABEL_CAT));
            ll_push(&entry,srch->self);
        }
        else
        {
            /*If it's a number, we allocate a new space to store it and change the field type*/
            ((label*)current_label->self)->type=LABEL_NUM;
            item=malloc(sizeof(double));
            *item=titem;
            ll_push(&entry,item);
        }
        if((curItem[0]=fgetc(fp))=='\n'){
            ll_push(&ret->lines,entry);
            break;
        }
        else if(curItem[0]==-1) goto end;
        else current_label=current_label->next;
    }
    /*Then we scan each remaining line*/
    entry=NULL;
    current_label=ret->col_labels;
    while(ftell(fp)!=-1&&fscanf(fp,"%[^,\n]",curItem)>0)
    {
        if(((label*)current_label->self)->type==LABEL_NUM)
        {
            titem=strtod(curItem,&tmp);
            if(*tmp!=0){
                /*If its's not a number, we throw an error*/
                printf("Format error at line %d (position %ld): csv contains invalid value \"%s\" for numerical field \"%s\"\n",line,ftell(fp),curItem,((label*)current_label->self)->name);
                fclose(fp);
                exit(-1);
            }
            else
            {
                /*If it's a number, we allocate a new space to store it*/
                item=malloc(sizeof(double));
                *item=titem;
                ll_push(&entry,item);
            }
        }
        else
        {
            /*We check if it's a valid label. If it doesn't exist, we create it*/
            srch=ll_search(&((label*)current_label->self)->sublabels,findLabel,curItem);
            if(srch==NULL)
            {
                ln=malloc(64);
                strncpy(ln,curItem,64);
                srch=ll_push(&((label*)current_label->self)->sublabels,Label(ln,LABEL_CAT));
            }
            ll_push(&entry,srch->self);
        }
        if((curItem[0]=fgetc(fp))=='\n'){
            ll_push(&ret->lines,entry);
            entry=NULL;
            current_label=ret->col_labels;
            line++;
        }
        else if(curItem[0]==-1) break;
        else current_label=current_label->next;
    }
    end:
    printf("%d entries in dataset.\n",line-2);
    fclose(fp);
    return ret;
}

void* get_entry_by_label_name(tree_ll* line,tree_ll* column,char* labelname)
{
    if(!line||!column)return NULL;
    foreach(column,{
        if(strncmp(((label*)column->self)->name,labelname,64)==0)goto found;
        line=line->next;
    });
    return NULL;
    found:
    return line->self;
}

label* select_label(tree_ll* columns,char* labelname)
{
    if(!columns)return 0;
    foreach(columns,{
        if(strncmp(((label*)columns->self)->name,labelname,64)==0)goto found;
    });
    return NULL;
    found:
    return columns->self;
}

int select_label_index(tree_ll* columns,char* labelname)
{
    if(!columns)return 0;
    int i=0;
    foreach(columns,{
        if(strncmp(((label*)columns->self)->name,labelname,64)==0)goto found;
        i++;
    });
    return -1;
    found:
    return i;
}

label* select_label_by_index(tree_ll* columns,int idx)
{
    if(!columns)return 0;
    int i=0;
    foreach(columns,{
        if(i==idx)goto found;
        i++;
    });
    return NULL;
    found:
    return columns->self;
}

void printDataset(dataset* ds)
{
    tree_ll *cur,*field,*entry;
    if(!ds)return;
    cur=ds->col_labels;
    while(cur)
    {
        printf("|%s\t",((label*)cur->self)->name);
        cur=cur->next;
    }
    printf("|\n");
    entry=ds->lines;
    while(entry)
    {
        field=ds->col_labels;
        cur=(tree_ll*)entry->self;
        while(field)
        {
            if(((label*)field->self)->type==LABEL_NUM)
                printf("|%.3lf\t",*(double*)((tree_ll*)cur->self));
            else if(((label*)field->self)->type==LABEL_CAT)
                printf("|%s\t",((label*)((tree_ll*)cur->self))->name);
            field=field->next;
            cur=cur->next;
        }
        printf("|\n");
        entry=entry->next;
    }
}

void infoDataset(dataset* ds)
{
    if(!ds)return;
    tree_ll *field,*entry;
    field=ds->col_labels;
    printf("Length: %d\n",ll_len(&ds->lines));
    while(field)
    {
        printf("Field \"%s\":\r\n\t*Type: ",((label*)field->self)->name);
        if(((label*)field->self)->type==LABEL_NUM)
            printf("Numerical\r\n\t*Mean: %.4lf\r\n\t*Standard deviation: %.4lf\r\n",
                mean(ds,((label*)field->self)->name),
                std_dev(ds,((label*)field->self)->name)
            );
        else
        {
            printf("Categorical\r\n\t*Labels: [ ");
            entry=((label*)field->self)->sublabels;
            while(entry)
            {
                printf("%s (count: %.0lf) ",((label*)entry->self)->name,reduce(ds,((label*)field->self)->name,r_count,entry->self,0));
                entry=entry->next;
            }
            printf("]\r\n");
        }
        field=field->next;
    }
}

int __sortbyfield;
char __sortbyfieldtype;

int __sortby(const void* a, const void *b)
{
    tree_ll *fa=(*((tree_ll**)a))->self,*fb=(*((tree_ll**)b))->self;
    double va,vb;
    int i;
    for(i=0;i<__sortbyfield;i++)
    {
        fa=fa->next;
        fb=fb->next;
    }
    if(__sortbyfieldtype==LABEL_NUM)
    {
        va=(*((double*)fa->self));
        vb=(*((double*)fb->self));
        return (va>vb)?1:(va==vb?0:-1);
    }
    else
    {
        return (strncmp(((label*)fa->self)->name,((label*)fb->self)->name,64));
    }
}

int __rsortby(const void* a, const void *b)
{
    tree_ll *fa=(*((tree_ll**)a))->self,*fb=(*((tree_ll**)b))->self;
    double va,vb;
    int i;
    for(i=0;i<__sortbyfield;i++)
    {
        fa=fa->next;
        fb=fb->next;
    }
    if(__sortbyfieldtype==LABEL_NUM)
    {
        va=(*((double*)fa->self));
        vb=(*((double*)fb->self));
        return (va>vb)?-1:(va==vb?0:1);
    }
    else
    {
        return 1-(strncmp(((label*)fa->self)->name,((label*)fb->self)->name,64));
    }
}

void sort_by(dataset* dataset,char* field,char reverse)
{
    if(!dataset||!(dataset->col_labels))return;
    int idx=0;
    tree_ll* lab=dataset->col_labels;
    tree_ll**ll;
    while(lab)
    {
        if(strncmp(((label*)lab->self)->name,field,64)==0)goto found;
        lab=lab->next;
        idx++;
    }
    printf("KeyError: Field \"%s\" does not exist in dataset. Could not sort.\n",field);
    return;
    found:
    __sortbyfield=idx;
    __sortbyfieldtype=((label*)lab->self)->type;
    ll=ll_to_array(&dataset->lines);
    if(reverse)qsort(ll,ll_len(&dataset->lines),sizeof(tree_ll*),__rsortby);
    else qsort(ll,ll_len(&dataset->lines),sizeof(tree_ll*),__sortby);
    dataset->lines=array_to_ll(ll,ll_len(&dataset->lines));
    free(ll);
    return;
}

double reduce(dataset* ds,char* field,void func(char field_type,void* field,double* acc,void* arg),void* arg,double init)
{
    if(!ds||!(ds->col_labels))return 0;
    int idx=0,i;
    double ret=init;
    tree_ll* lab=ds->col_labels;
    tree_ll* line;
    tree_ll* entry;
    while(lab)
    {
        if(strncmp(((label*)lab->self)->name,field,64)==0)goto found;
        lab=lab->next;
        idx++;
    }
    printf("KeyError: Field \"%s\" does not exist in dataset. Could not apply reduction.\n",field);
    return 0;
    found:
    line=ds->lines;
    while(line)
    {
        entry=(tree_ll*)line->self;
        for(i=0;i<idx;i++)entry=entry->next;
        func(((label*)lab->self)->type,entry->self,&ret,arg);
        line=line->next;
    }
    return ret;
}

/*
Reduction function for counting elements with a certain label
field_type=LABEL_CAT
arg=label* acc=0
*/
void r_count(char field_type,void* field,double* acc,void* arg)
{
    if(field_type!=LABEL_CAT)return;
    *acc+=field==arg?1:0;
}

/*
Reduction function for summing all elements
field_type=LABEL_NUM
arg=NULL acc=0
*/
void r_sum(char field_type,void* field,double* acc,void* arg)
{
    if(field_type!=LABEL_NUM)return;
    *acc+=*(double*)field;
}

/*
Reduction function for calculating the sum of all squared deviations from <double* arg>
field_type=LABEL_NUM
arg=&double acc=0
*/
void r_dev(char field_type,void* field,double* acc,void* arg)
{
    if(field_type!=LABEL_NUM)return;
    *acc+=((*(double*)field)-*(double*)arg)*((*(double*)field)-*(double*)arg);
}

double mean(dataset* ds,char* field)
{
    if(!ds||!(ds->col_labels))return 0;
    int idx=0;
    tree_ll* lab=ds->col_labels;
    while(lab)
    {
        if(strncmp(((label*)lab->self)->name,field,64)==0)goto found;
        lab=lab->next;
        idx++;
    }
    printf("KeyError: Field \"%s\" does not exist in dataset. Could not calculate mean.\n",field);
    return 0;
    found:
    return reduce(ds,field,r_sum,NULL,0)/ll_len(&ds->lines);
}

double variance(dataset* ds,char* field)
{
    if(!ds||!(ds->col_labels))return 0;
    int idx=0,len;
    double mean=0;
    tree_ll* lab=ds->col_labels;
    while(lab)
    {
        if(strncmp(((label*)lab->self)->name,field,64)==0)goto found;
        lab=lab->next;
        idx++;
    }
    printf("KeyError: Field \"%s\" does not exist in dataset. Could not calculate variance.\n",field);
    return 0;
    found:
    len=ll_len(&ds->lines);
    mean=reduce(ds,field,r_sum,NULL,0)/len;
    return reduce(ds,field,r_dev,&mean,0)/len;
}

double std_dev(dataset* ds,char* field)
{
    return sqrt(variance(ds,field));
}

double class_entropy(dataset* ds,char* field)
{
    if(!ds||!(ds->col_labels))return 0;
    int len;
    double entropy=0,p;
    tree_ll* lab=ds->col_labels,*clab;
    while(lab)
    {
        if(strncmp(((label*)lab->self)->name,field,64)==0)goto found;
        lab=lab->next;
    }
    printf("KeyError: Field \"%s\" does not exist in dataset. Could not calculate class entropy.\n",field);
    return 0;
    found:
    if(((label*)lab->self)->type!=LABEL_CAT)return 0;
    len=ll_len(&ds->lines);
    clab=((label*)lab->self)->sublabels;
    while(clab)
    {
        p=reduce(ds,field,r_count,clab->self,0)/len;
        entropy+=p==0?0:p*log(p);
        clab=clab->next;
    }
    return -entropy;
}
double attribute_entropy(dataset* ds,char* field,char* classfield)
{
    if(!ds||!(ds->col_labels))return 0;
    int len,idx=0;
    double entropy=0;
    tree_ll* lab=ds->col_labels,*clab;
    dataset* sub;
    f_namefilter conf;
    while(lab)
    {
        if(strncmp(((label*)lab->self)->name,field,64)==0)goto found;
        lab=lab->next;
        idx++;
    }
    printf("KeyError: Field \"%s\" does not exist in dataset. Could not calculate attribute entropy.\n",field);
    return 0;
    found:
    len=ll_len(&ds->lines);
    if(((label*)lab->self)->type==LABEL_CAT)
    {
        clab=((label*)lab->self)->sublabels;
        while(clab)
        {
            conf.field_index=idx;
            conf.target=clab->self;
            sub=filter_dataset(ds,f_by_name,&conf);

            entropy+=(ll_len(&sub->lines)/(double)len)*class_entropy(sub,classfield);

            ll_free(&sub->lines);
            free(sub);
            clab=clab->next;
        }
    }
    return entropy;
}
double attribute_num_entropy(dataset* ds,char* field,char* classfield,double threshold)
{
    if(!ds||!(ds->col_labels))return 0;
    int len,idx=0;
    double entropy=0;
    tree_ll* lab=ds->col_labels;
    dataset* sub;
    f_numberfilter conf;
    while(lab)
    {
        if(strncmp(((label*)lab->self)->name,field,64)==0)goto found;
        lab=lab->next;
        idx++;
    }
    printf("KeyError: Field \"%s\" does not exist in dataset. Could not calculate attribute entropy.\n",field);
    return 0;
    found:
    len=ll_len(&ds->lines);
    if(((label*)lab->self)->type==LABEL_NUM)
    {
        conf.field_index=idx;
        conf.target=threshold;
        conf.bt=0;
        sub=filter_dataset(ds,f_by_number,&conf);
        entropy+=(ll_len(&sub->lines)/(double)len)*class_entropy(sub,classfield);
        ll_free(&sub->lines);
        free(sub);
        conf.bt=1;
        sub=filter_dataset(ds,f_by_number,&conf);
        entropy+=(ll_len(&sub->lines)/(double)len)*class_entropy(sub,classfield);
        ll_free(&sub->lines);
        free(sub);
    }
    return entropy;
}

double optimize_threshold(dataset* ds,char* field,char* classfield)
{
    if(!ds||!field||!classfield||!(ds->col_labels)||ll_len(&ds->lines)==0)return 0;
    int pos,i,idx=0,dir=1,len=ll_len(&ds->lines);
    double threshold=0,pt,ent,pent=__DBL_MAX__;
    tree_ll* lab=ds->col_labels,*line;
    while(lab)
    {
        if(strncmp(((label*)lab->self)->name,field,64)==0)goto found;
        lab=lab->next;
        idx++;
    }
    printf("KeyError: Field \"%s\" does not exist in dataset. Could not optimize threshold.\n",field);
    return 0;
    found:
    if(((label*)lab->self)->type==LABEL_NUM)
    {
        sort_by(ds,((label*)lab->self)->name,0);
        pos=ll_len(&ds->lines)/2;
        /*We select the line at the desired position*/
        line=ds->lines;
        for(i=0;i<pos;i++)line=line->next;
        /*And the attribute we are optimizing*/
        line=line->self;
        for(i=0;i<idx;i++)line=line->next;
        pt=*(double*)line->self;
        pent=attribute_num_entropy(ds,field,classfield,pt);
        while(dir>=-1&&pos+dir>0&&pos+dir<len)
        {
            pos+=dir;
            /*We select the line at the desired position*/
            line=ds->lines;
            for(i=0;i<pos;i++)line=line->next;
            /*And the attribute we are optimizing*/
            line=line->self;
            for(i=0;i<idx;i++)line=line->next;
            threshold=*(double*)line->self;
            ent=attribute_num_entropy(ds,field,classfield,threshold);
            if(ent>pent)
            {
                dir-=2;
                pos+=dir;
            }
            else
            {
                pent=ent;
                pt=threshold;
            }
        }
    }
    return threshold;
}

dataset* filter_dataset(dataset* ds,char func(tree_ll* line,void* arg),void* arg)
{
    if(!ds||!ds->col_labels||!ds->lines)return NULL;
    dataset* ret=malloc(sizeof(dataset));
    tree_ll* cur=ds->lines;
    ret->col_labels=ds->col_labels;
    ret->lines=NULL;
    while(cur)
    {
        if(func(cur->self,arg))ll_push(&ret->lines,cur->self);
        cur=cur->next;
    }
    return ret;
}

char f_by_name(tree_ll* line,void* arg)
{
    int i;
    for(i=0;i<((f_namefilter*)arg)->field_index;i++)line=line->next;
    return line->self==((f_namefilter*)arg)->target;
}

char f_by_number(tree_ll* line,void* arg)
{
    int i;
    char r;
    for(i=0;i<((f_numberfilter*)arg)->field_index;i++)line=line->next;
    r=(*((double*)line->self)<=((f_numberfilter*)arg)->target);
    if(((f_numberfilter*)arg)->bt)r=!r;
    return r;
}

double chi_squared(dataset* root,dataset** children,int len,label* classlabel)
{
    if(!root||!children||classlabel->type!=LABEL_CAT)return 0;
    int i;
    tree_ll* lab;
    double ret=0,count,expected;
    for(i=0;i<len;i++)
    {
        if(!children[i])return 0;
        lab=classlabel->sublabels;
        expected=reduce(root,classlabel->name,r_count,lab->self,0)*(1/(double)len);
        while(lab)
        {
            count=reduce(children[i],classlabel->name,r_count,lab->self,0);
            ret+=((count-expected)*(count-expected))/expected;
            lab=lab->next;
        }
    }
    return ret;
}

void fit_tree(tree_node** root,dataset* ds,double chi_square_significance_limit,char* classfield)
{
    if(!root||!ds)return;
    int i=0,mi,len;
    tree_ll* classlabel=ds->col_labels;
    while(classlabel)
    {
        if(strncmp(((label*)classlabel->self)->name,classfield,64)==0)goto found;
        classlabel=classlabel->next;
    }
    printf("KeyError: Field \"%s\" does not exist in dataset. Could not fit tree.\n",classfield);
    return;
    label* l=NULL;
    double entropy,thresh,pt;
    double gain,maxgain=0;
    tree_ll* lab=ds->col_labels;
    dataset** subsets;
    tree_ll* working;
    f_numberfilter nuconf;
    f_namefilter naconf;
    found:
    l=NULL;
    entropy=class_entropy(ds,classfield);
    lab=ds->col_labels;
    while(lab&&entropy)
    {
        if(strcmp(((label*)lab->self)->name,classfield))
        {
            if(((label*)lab->self)->type==LABEL_NUM)
            {
                thresh=optimize_threshold(ds,((label*)lab->self)->name,classfield);
                gain=entropy-attribute_num_entropy(ds,((label*)lab->self)->name,classfield,thresh);
            }
            else
            {
                gain=entropy-attribute_entropy(ds,((label*)lab->self)->name,classfield);
            }
            if(gain>maxgain)
            {
                l=((label*)lab->self);
                pt=thresh;
                maxgain=gain;
                mi=i;
            }
        }
        i++;
        lab=lab->next;
    }
    *root=malloc(sizeof(tree_node));
    if(!l||!entropy)
    {
        /*We choose the biggest count and set ourselves as a leaf node*/
        goto leaf;
    }
    if(l->type==LABEL_NUM)
    {
        (*root)->attribute=l;
        (*root)->partition=pt;
        (*root)->subtrees=NULL;
        subsets=malloc(sizeof(dataset*)*2);
        nuconf.bt=0;
        nuconf.field_index=mi;
        nuconf.target=pt;
        subsets[0]=filter_dataset(ds,f_by_number,&nuconf);
        nuconf.bt=1;
        subsets[1]=filter_dataset(ds,f_by_number,&nuconf);
        if(ll_len(&subsets[0]->lines)==0||ll_len(&subsets[1]->lines)==0)
        {
            ll_free(&subsets[0]->lines);
            ll_free(&subsets[1]->lines);
            free(subsets[0]);
            free(subsets[1]);
            goto leaf;
        }

        /*Chi-squared test*/
        if(chi_squared(ds,subsets,2,classlabel->self)<chi_square_significance_limit)
        {
            /*If it doesn't pass the test, we choose the biggest count and set ourselves as a leaf node*/
            ll_free(&subsets[0]->lines);
            ll_free(&subsets[1]->lines);
            free(subsets[0]);
            free(subsets[1]);
            goto leaf;
        }
        else
        {
            /*If the division is not statistically insignificant, we can keep it*/
            ll_push(&(*root)->subtrees,malloc(sizeof(tree_node*)));
            fit_tree((tree_node**)&(*root)->subtrees->self,subsets[0],chi_square_significance_limit,classfield);
            ll_push(&(*root)->subtrees,malloc(sizeof(tree_node*)));
            fit_tree((tree_node**)&(*root)->subtrees->next->self,subsets[1],chi_square_significance_limit,classfield);
            ll_free(&subsets[0]->lines);
            ll_free(&subsets[1]->lines);
            free(subsets[0]);
            free(subsets[1]);
        }
    }
    else
    {
        (*root)->attribute=l;
        (*root)->partition=0;
        (*root)->subtrees=NULL;
        len=ll_len(&l->sublabels);
        lab=l->sublabels;
        subsets=malloc(sizeof(dataset*)*len);
        naconf.field_index=mi;
        for(i=0;i<len;i++)
        {
            naconf.target=lab->self;
            subsets[i]=filter_dataset(ds,f_by_name,&naconf);
            if(ll_len(&subsets[i]->lines)==0)
            {
                len=i;
                for(i=0;i<=len;i++)
                {
                    ll_free(&subsets[i]->lines);
                    free(subsets[i]);
                }
                goto leaf;
            }
            lab=lab->next;
        }
        /*Chi-squared test*/
        if(chi_squared(ds,subsets,len,classlabel->self)<chi_square_significance_limit)
        {
            /*If it doesn't pass the test, we choose the biggest count and set ourselves as a leaf node*/
            for(i=0;i<len;i++)
            {
                ll_free(&subsets[i]->lines);
                free(subsets[i]);
            }
            goto leaf;
        }
        else
        {
            /*If the division is not statistically insignificant, we can keep it*/
            working=ll_push(&(*root)->subtrees,malloc(sizeof(tree_node*)));
            fit_tree((tree_node**)&working->self,subsets[0],chi_square_significance_limit,classfield);
            ll_free(&subsets[0]->lines);
            free(subsets[0]);
            for(i=1;i<len;i++)
            {
                working=ll_push(&working,malloc(sizeof(tree_node*)));
                fit_tree((tree_node**)&working->self,subsets[i],chi_square_significance_limit,classfield);
                ll_free(&subsets[i]->lines);
                free(subsets[i]);
            }
        }
    }
    return;
    leaf:
    lab=((label*)classlabel->self)->sublabels;
    mi=0;
    while(lab)
    {
        i=reduce(ds,classfield,r_count,lab->self,0);
        if(i>mi)
        {
            mi=i;
            l=lab->self;
        }
        lab=lab->next;
    }
    (*root)->attribute=l;
    (*root)->partition=0;
    (*root)->subtrees=NULL;
}

label* most_frequent_class(tree_node* root,dataset* ds,char* classfield)
{
    if(!root||!ds)return NULL;
    label* classlabel=select_label(ds->col_labels,classfield),*result;
    unsigned int len=ll_len(&classlabel->sublabels),occurences[len],i,imax=0,max=0;
    for(i=0;i<len;i++)occurences[i]=0;
    tree_ll* line=ds->lines;
    foreach(line,{
        result=classify(root,line->self,ds->col_labels);
        if(result)
        {
            occurences[select_label_index(classlabel->sublabels,result->name)]++;
        }
    });
    for(i=0;i<len;i++)
    {
        if(occurences[i]>max)
        {
            imax=i;
            max=occurences[i];
        }
    }
    return select_label_by_index(classlabel->sublabels,imax);
}

int tree_size(tree_node* root)
{
    int s=1;
    if(!root)return 0;
    tree_ll* subtree=root->subtrees;
    foreach(subtree,
    {
        s+=tree_size(subtree->self);
    });
    return s;
}

tree_node* clone_tree(tree_node* root)
{
    if(!root)return NULL;
    tree_node* ret=malloc(sizeof(tree_node));
    tree_ll* subtree;
    ret->attribute=root->attribute;
    ret->partition=root->partition;
    ret->subtrees=NULL;
    subtree=root->subtrees;
    foreach(subtree,{
        ll_push(&ret->subtrees,clone_tree(subtree->self));
    });
    return ret;
}

void free_tree(tree_node** root)
{
    if(!root||!(*root))return;
    tree_ll* subtree=(*root)->subtrees;
    foreach(subtree,{
        free_tree((tree_node**)&subtree->self);
    });
    free(*root);
    *root=NULL;
}

double _prune_tree(tree_node** root,dataset* ds,char* classfield,tree_node* orig_tree)
{
    double score=0,pscore=0;
    unsigned int idx=0;
    label* leaf=NULL;
    char all_leaves=1,all_eq=1,prune_anyways=0;
    tree_ll* subtree,*subsubtree;
    tree_node bkp;
    dataset* subset;
    f_numberfilter numconf;
    f_namefilter namconf;
    if(!root||!(*root)||!(*root)->subtrees||!ds||!ds->lines)return 0;
    pscore=tree_score(orig_tree,ds,classfield);
    subtree=(*root)->subtrees;
    foreach(subtree,{
        if((subsubtree=((tree_node*)(subtree->self))->subtrees))
        {
            all_leaves=0;
            foreach(subsubtree,({
                if(((tree_node*)subsubtree->self)->attribute->type==LABEL_NUM)
                {
                    numconf.bt=idx;
                    numconf.field_index=select_label_index(ds->col_labels,((tree_node*)subsubtree->self)->attribute->name);
                    numconf.target=((tree_node*)subsubtree->self)->partition;
                    subset=filter_dataset(ds,f_by_number,&numconf);
                }
                else
                {
                    namconf.field_index=select_label_index(ds->col_labels,((tree_node*)subsubtree->self)->attribute->name);
                    namconf.target=select_label_by_index(((tree_node*)subsubtree->self)->attribute->sublabels,idx);
                    subset=filter_dataset(ds,f_by_name,&namconf);
                }
                if(!subset->lines){
                    prune_anyways=1;
                    free(subset);
                    break;
                }
                while(prune_tree((tree_node**)&subsubtree->self,subset,classfield));
                ll_free(&subset->lines);
                free(subset);
            }));
        }
        else
        {
            if(all_eq)
            {
                if(leaf)
                {
                    all_eq=all_eq&&((tree_node*)(subtree->self))->attribute==leaf;
                }
                else
                {
                    leaf=((tree_node*)(subtree->self))->attribute;
                }
            }
        }
        idx++;
    });
    if(all_leaves)
    {
        if(all_eq)
        {
            subtree=(*root)->subtrees;
            foreach(subtree,{
                free(subtree->self);
            });
            ll_free(&(*root)->subtrees);
            (*root)->partition=0;
            (*root)->attribute=leaf;
            return -1;
        }
        else
        {
            bkp=**root;
            (*root)->attribute=most_frequent_class(&bkp,ds,classfield);
            (*root)->partition=0;
            (*root)->subtrees=NULL;
            score=tree_score(orig_tree,ds,classfield);
            if(score<pscore)
            {
                **root=bkp;
            }
            else
            {
                ll_free_self(&bkp.subtrees);
            }
        }
    }
    else if(prune_anyways)
    {
        bkp=**root;
        (*root)->attribute=most_frequent_class(&bkp,ds,classfield);
        (*root)->partition=0;
        (*root)->subtrees=NULL;
        score=tree_score(orig_tree,ds,classfield);
        if(score<pscore)
        {
            **root=bkp;
        }
        else
        {
            ll_free_self(&bkp.subtrees);
        }
    }
    else return 0;
    score=tree_score(orig_tree,ds,classfield);
    return pscore-score;
}

double prune_tree(tree_node** root,dataset* ds,char* classfield)
{
    double prev=0,cur;
    while((cur=_prune_tree(root,ds,classfield,*root)))prev=cur;
    return prev;
}

label* classify(tree_node* root,tree_ll* line,tree_ll* columns)
{
    if(!root||!line)return NULL;
    if(!root->subtrees)return root->attribute;
    int idx=0,i;
    tree_ll* lab=columns,*subtree,*entry;
    while(lab)
    {
        if(strncmp(((label*)lab->self)->name,root->attribute->name,64)==0)goto found;
        lab=lab->next;
        idx++;
    }
    return NULL;
    found:
    if(root->attribute->type==LABEL_NUM)
    {
        lab=line;
        for(i=0;i<idx;i++)lab=lab->next;
        if(*(double*)lab->self<=root->partition)
        {
            return classify(root->subtrees->self,line,columns);
        }
        else return classify(root->subtrees->next->self,line,columns);
    }
    else
    {
        lab=root->attribute->sublabels;
        entry=line;
        for(i=0;i<idx;i++){entry=entry->next;}
        subtree=root->subtrees;
        while(lab)
        {
            if(entry->self==lab->self)return classify(subtree->self,line,columns);
            lab=lab->next;
            subtree=subtree->next;
        }
    }
    return NULL;
}

double tree_score(tree_node* root,dataset* ds,char* classfield)
{
    int tests=ll_len(&ds->lines),right=0;
    tree_ll* line;
    label* lab;
    line=ds->lines;
    while(line)
    {
        lab=classify(root,line->self,ds->col_labels);
        right+=get_entry_by_label_name(line->self,ds->col_labels,classfield)==lab;
        line=line->next;
    }
    return (double)right/(double)tests;
}

void _print_tree(tree_node* root,int l)
{
    if(!root)return;
    int i;
    tree_ll* curtree,*curatt;
    for(i=0;i<l;i++)printf("\t");
    printf("[%s]",root->attribute->name);
    curtree=root->subtrees;
    if(!curtree)printf("*\n");
    else
    {
        printf("\n");
        if(root->attribute->type==LABEL_NUM)
        {
            for(i=0;i<l;i++)printf("\t");
            printf("  <=(%.2lf)->\n",root->partition);
            _print_tree(curtree->self,l+1);
            curtree=curtree->next;
            for(i=0;i<l;i++)printf("\t");
            printf("   >(%.2lf)->\n",root->partition);
            _print_tree(curtree->self,l+1);
        }
        else
        {
            curatt=root->attribute->sublabels;
            while(curtree)
            {
                for(i=0;i<l;i++)printf("\t");
                printf("  (%s)->\n",((label*)curatt->self)->name);
                _print_tree(curtree->self,l+1);
                curtree=curtree->next;
                curatt=curatt->next;
            }
        }
    }
}
void print_tree(tree_node* root)
{
    if(!root)return;
    _print_tree(root,0);
}
dataset* sample_dataset(dataset* ds,int len,char* classfield)
{
    if(!ds||len==0)return NULL;
    label* classlabel=select_label(ds->col_labels,classfield);
    if(!classlabel)return NULL;
    char* selected;
    int cur,subs=ll_len(&classlabel->sublabels),olen=ll_len(&ds->lines),tlen,clen,slen,sel,i;
    f_namefilter conf;
    conf.field_index=select_label_index(ds->col_labels,classfield);
    tree_ll* cval,*line;
    dataset* ret=malloc(sizeof(dataset));
    ret->col_labels=ds->col_labels;
    ret->lines=NULL;
    dataset* subset;

    cval=classlabel->sublabels;
    for(cur=0;cur<subs;cur++)
    {
        conf.target=cval->self;
        subset=filter_dataset(ds,f_by_name,&conf);
        slen=ll_len(&subset->lines);
        clen=0;
        selected=calloc(slen,1);
        tlen=len*((double)slen/(double)olen);
        while(clen<tlen)
        {
            while(selected[(sel=rand()%slen)]);
            line=subset->lines;
            for(i=0;i<sel;i++)line=line->next;
            ll_push(&ret->lines,line->self);
            selected[sel]=1;
            clen++;
        }
        free(selected);
        ll_free(&subset->lines);
        free(subset);
        cval=cval->next;
    }
    return ret;
}