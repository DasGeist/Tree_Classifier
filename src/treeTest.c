#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "treeClassifier.h"

int main()
{
    srand(time(NULL));
    printf("Loading training dataset...\n");
    dataset* data=csv_to_dataset("datasets/test.csv");
    tree_ll* line;
    dataset* train=sample_dataset(data,ll_len(&data->lines)/2,"colour");
    tree_node* root=NULL;
    if(!data||!train)
    {
        printf("File not found.\n");
        return 1;
    }
    printf("Dataset info:\n");
    infoDataset(train);
    printf("Fitting...\n");
    fit_tree(&root,train,2,"colour");
    printf("Fitting completed.\n");
    int tests=ll_len(&data->lines),right=0;
    label* lab;
    line=data->lines;
    while(line)
    {
        lab=classify(root,line->self,train->col_labels);
        right+=((tree_ll*)(line->self))->next->next->next->next->self==lab;
        line=line->next;
    }
    printf("%d right out of %d. (%.2lf)\n",right,tests,((double)right/(double)tests)*100);
    print_tree(root);
    return 0;
}