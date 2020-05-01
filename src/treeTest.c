#include <stdio.h>
#include <stdlib.h>
#include "treeClassifier.h"

int main()
{
    printf("Loading training dataset...\n");
    dataset* train=csv_to_dataset("datasets/train.csv");
    tree_ll* line;
    //printf("Loading testing dataset...\n");
    //dataset* data=csv_to_dataset("datasets/test.csv");
    tree_node* root=NULL;
    if(!train)
    {
        printf("File not found.\n");
        return 1;
    }
    printf("Dataset info:\n");
    infoDataset(train);
    printf("Fitting...\n");
    fit_tree(&root,train,0.05,"colour");
    printf("Fitting completed.\n");
    int tests=500,right=0,i,idx,j;
    label* lab;
    for(i=0;i<tests;i++)
    {
        line=train->lines;
        idx=rand()%ll_len(&line);
        for(j=0;j<idx;j++)line=line->next;
        lab=classify(root,line->self,train->col_labels);
        right+=((tree_ll*)(line->self))->next->next->next->next->self==lab;
    }
    printf("%d right out of %d. (%.2lf)\n",right,tests,((double)right/(double)tests)*100);
    print_tree(root);
    return 0;
}