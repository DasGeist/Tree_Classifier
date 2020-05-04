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
    //dataset* train=sample_dataset(data,5,"colour");
    forest test=NULL;
    int forest_max_size=10;
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
    printf("Fitting...\n");
    fit_forest(&test,train,"colour",forest_max_size,0.7);
    do
    {
        improvement=fit_forest(&test,train,"colour",forest_max_size,0.7);
        printf("Secondary fitting cycle - improvement: %.2lf\n",improvement);
    }
    while(improvement>0.05);
    printf("Fitting completed.\nScore: %.2lf\nSize: %d\n",forest_score(test,data,"colour")*100,ll_len(&test));
    return 0;
}