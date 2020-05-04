#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "treeClassifier.h"

int main()
{
    srand(time(NULL));
    printf("Loading training dataset...\n");
    dataset* data=csv_to_dataset("datasets/test.csv");
    dataset* train=sample_dataset(data,ll_len(&data->lines)/2,"colour");
    dataset* prune=sample_dataset(train,ll_len(&train->lines)/2,"colour");
    tree_node* root=NULL;
    if(!data||!train)
    {
        printf("File not found.\n");
        return 1;
    }
    printf("Dataset info:\n#data:\n");
    infoDataset(data);
    printf("#train\n");
    infoDataset(train);
    printf("#prune...\n");
    infoDataset(prune);
    printf("Fitting...\n");
    fit_tree(&root,train,0,"colour");
    printf("Fitting completed.\nScore: %.2lf\nSize: %d\n",tree_score(root,data,"colour")*100,tree_size(root));
    printf("Pruning...\n");
    printf("Pruning improved score by %.4lf on pruning dataset.\n",prune_tree(&root,prune,"colour"));
    printf("Pruning completed.\nScore: %.2lf\nSize: %d\n",tree_score(root,data,"colour")*100,tree_size(root));
    if(tree_size(root)<10)print_tree(root);
    return 0;
}