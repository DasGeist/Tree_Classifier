import random
import csv
import matplotlib.pyplot as plt
import seaborn as sb
import pandas
def classifier(item):
    return "orange" if (item[0]*item[1])*(-1 if random.randint(0,100)<5 else 1)>=0 else "blue"
le=5000
lims=[(-10,10),(-10,10)]
items=[[round(l[0]+(random.random()*(l[1]-l[0])),7) for l in lims] for i in range(le)]

labels=[['x','y','colour']]
#labels=[['x','y','x_c','y_c','colour']]
items=[item+[classifier(item)] for item in items]
#items=[item+["left" if item[0]<0 else "right","upper" if item[1]>0 else "lower",classifier(item)] for item in items]
train_l=random.sample(items,int(le/2))
sb.pairplot(pandas.DataFrame(items,columns=labels[0]),hue='colour',x_vars='x',y_vars='y')
plt.show()
with open("datasets/train.csv",'w') as train:
    with open("datasets/test.csv",'w') as test:
        trainwriter = csv.writer(train, delimiter=',',
                            quotechar='"', quoting=csv.QUOTE_MINIMAL,dialect=csv.unix_dialect)
        trainwriter.writerows(labels+train_l)
        testwriter = csv.writer(test, delimiter=',',
                            quotechar='"', quoting=csv.QUOTE_MINIMAL,dialect=csv.unix_dialect)
        testwriter.writerows(labels+items)