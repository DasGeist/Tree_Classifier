#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "treeClassifier.h"

int main()
{
    srand(time(NULL));
    printf("Loading training dataset...\n");
    dataset* data=csv_to_dataset("datasets/iris.data");
    dataset* train=sample_dataset(data,ll_len(&data->lines)/2,"class");
    forest test=NULL,random=NULL;
    int forest_max_size=50;
    double improvement;
    if(!data||!train)
    {
        printf("File not found.\n");
        return 1;
    }
    printf("Dataset info:\n#data:\n");
    infoDataset(data);
    printf("#train\n");
    infoDataset(train);
    printf("Fitting entropy-based forest...\n");
    fit_forest(&test,train,"class",forest_max_size,0.7);
    do
    {
        improvement=fit_forest(&test,train,"class",forest_max_size,0.7);
        printf("Secondary fitting cycle - improvement: %.2lf\n",improvement);
    }
    while(improvement>0.01||improvement<0);
    printf("Fitting completed.\nScore: %.2lf\nSize: %d\n",forest_score(test,data,"class")*100,ll_len(&test));
    printf("Fitting random forest:\n");
    fit_random_forest(&random,train,"class",forest_max_size,0.7);
    do
    {
        improvement=fit_random_forest(&random,train,"class",forest_max_size,0.7);
        printf("Secondary fitting cycle - improvement: %.2lf\n",improvement);
    }
    while(improvement>0.01||improvement<0);
    printf("Fitting completed.\nScore: %.2lf\nSize: %d\n",forest_score(random,data,"class")*100,ll_len(&random));
    return 0;
}