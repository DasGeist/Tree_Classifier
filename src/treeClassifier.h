/*
TreeClassifier.h - A simple implementation of an entropy-based decision-tree classifer
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

/*
A simple doubly-linked list structure
*/
typedef struct _tree_ll{
    struct _tree_ll* prev;
    struct _tree_ll* next;
    void* self;
    
} tree_ll;
/*Stack-like push function for the list*/
tree_ll *ll_push(tree_ll** list,void* item);
/*Stack-like pop function for the list*/
void* ll_pop(tree_ll **list);
/*Frees the list and all its children*/
void ll_free(tree_ll **list);
/*Frees the list, all its children and the 'self' pointers*/
void ll_free_self(tree_ll **list);
/*Searches the linked list for an item that causes <func> to return a non-null value*/
tree_ll* ll_search(tree_ll** list,char func(void* item,void* arg),void* arg);
/*Returns the length of the list*/
int ll_len(tree_ll** list);
/*Returns an array with the address of every member of the list*/
tree_ll** ll_to_array(tree_ll** list);
/*Takes a list of tree_ll* and makes them into a list in the order they were given. Returns the list's root*/
tree_ll* array_to_ll(tree_ll** array,int len);

#define LABEL_NUM 00
#define LABEL_CAT 01
/*
Label.
Contains a name (up to 63 characters), a type and its sublabels (class labels, if the parent is a column label, for example).
Types:
0 = Numerical attribute. (Only valid as a column label.)
1 = Categorical attribute (All labels with this type will be stored as pointers to a reference label)
*/
typedef struct _label{
    char name[64];
    char type;
    tree_ll *sublabels;
}label;

/*Label-finder-by-name function for ll_search*/
char findLabel(void* lab,void* labname);

/*Allocates a label*/
label* Label(char* name,char type);

/*A structure for holding organized data*/
typedef struct _dataset{
    tree_ll* col_labels;
    tree_ll* lines;
}dataset;

/*
Creates a dataset from a .csv file
It assumes the first lines contain the column labels, all numerical values are double-precision integers and everything else
is a categorical value (represented by a string).
*/
dataset* csv_to_dataset(const char* fname);

/*
Returns a line's entry at column <labelname>
*/
void* get_entry_by_label_name(tree_ll* line,tree_ll* columns,char* labelname);
/*
Selects a label from its name
*/
label* select_label(tree_ll* columns,char* labelname);
/*
Selects a label index from its name
*/
int select_label_index(tree_ll* columns,char* labelname);

/*
Prints a dataset
*/
void printDataset(dataset* ds);

/*
Prints a dataset's field information
*/
void infoDataset(dataset* ds);

/*
Sorts a dataset based on the specified field
reverse=0 for ascending order, 1 for reverse order
*/
void sort_by(dataset* ds,char* field,char reverse);

/*
Reduction function for counting elements with a certain label
field_type=LABEL_CAT
arg=label* acc=0
*/
void r_count(char field_type,void* field,double* acc,void* arg);
/*
Reduction function for summing all elements
field_type=LABEL_NUM
arg=NULL acc=0
*/
void r_sum(char field_type,void* field,double* acc,void* arg);
/*
Reduction function for calculating the sum of all squared deviations from <double* arg>
field_type=LABEL_NUM
arg=&double acc=0
*/
void r_dev(char field_type,void* field,double* acc,void* arg);
/*
A reduction function. Calls <func> for each entry of the dataset. It initializes the accumulator with the value of <init>
and passes the field's field_type,a pointer to the entry (double* for LABEL_NUM and label* for LABEL_CAT), a pointer to
the accumulator and the argument pointer that was passed to the function.
*/
double reduce(dataset* ds,char* field,void func(char field_type,void* entry,double* acc,void* arg),void* arg,double init);
/*
Calculates the mean of a field
*/
double mean(dataset* ds,char* field);
/*
Calculates the variance of a field
*/
double variance(dataset* ds,char* field);
/*
Calculates the standard deviation of a field
*/
double std_dev(dataset* ds,char* field);
/*
Calculates the entropy of a class field
*/
double class_entropy(dataset* ds,char* field);
/*
Calculates the entropy of an attribute field
*/
double attribute_entropy(dataset* ds,char* field,char* classfield);
/*
Calculates the entropy of a numerical attribute field when partitioned at threshold <threshold>
*/
double attribute_num_entropy(dataset* ds,char* field,char* classfield,double threshold);
/*
Returns a new dataset containing only the entries to which <func> returned a non-null value as an output
*/
dataset* filter_dataset(dataset* ds,char func(tree_ll* line,void* arg),void* arg);
/*
Jenks-like ( https://en.wikipedia.org/wiki/Jenks_natural_breaks_optimization ) method for optimizing a break.
It's used for finding a good threshold for a numerical attribute.
*/
double optimize_threshold(dataset* ds,char* field,char* classfield);

typedef struct _f_namefilter{
    int field_index;/*Index of the field on the line (label index on col_labels)*/
    label* target;/*Sublabel that is being filtered*/
}f_namefilter;
typedef struct _f_numberfilter{
    int field_index;/*Index of the field on the line (label index on col_labels)*/
    double target;/*Sublabel that is being filtered*/
    char bt;/*0 for filtering values <=target, 1 for >target*/
}f_numberfilter;

/*
Filter-function for selecting lines by label value.
arg=f_namefilter*
*/
char f_by_name(tree_ll* line,void* arg);
/*
Filter-function for selecting lines by label value.
arg=f_numberfilter*
*/
char f_by_number(tree_ll* line,void* arg);

/*Decision tree node*/
typedef struct _tree_node{
    label* attribute;/*For most nodes, it's the attribute that's being decided upon. For leaves, it's the class.*/
    double partition;/*For Numerical attributes, indicates the partition limit.*/
    tree_ll* subtrees;/*Subtrees*/
}tree_node;

/*Calculates the chi-squared value of the */
double chi_squared(dataset* root,dataset** children,int len,label* classlabel);
/*
Trains a tree based on a dataset
chi_square_significance_limit is generally 0.05
*/
void fit_tree(tree_node** root,dataset* ds,double chi_square_significance_limit,char* classfield);
/*
Returns the most frequent class from the set after being classified by the tree
*/
label* most_frequent_class(tree_node* root,dataset* ds,char* classfield);
/*
Returns the number of nodes (counting leaves) in a tree
*/
int tree_size(tree_node* root);
/*
Deep-clones a tree
*/
tree_node* clone_tree(tree_node* root);
/*
Deep-frees a tree
*/
void free_tree(tree_node** root);
/*
Prunes a tree's unecessary nodes (until pruning reduces accuracy)
Returns the score improvement or -1 if there was no improvement but the tree is still simpler
(ex: ((a a) b) to (a b) )
*/
double prune_tree(tree_node** root,dataset* ds,char* classfield);
/*
Classifies all entries on dataset <ds> using tree <root> (ignoring <classfield>) then returns the
success rate (double, 0=all classified wrong, 1=all classified correctly).
*/
double tree_score(tree_node* root,dataset* ds,char* classfield);
/*
Use a tree to classify a line.
*/
label* classify(tree_node* root,tree_ll* line,tree_ll* columns);
/*
Describe a tree
*/
void print_tree(tree_node* root);
/*
Generate a balanced sample (according to the distributions of <classfield> on <ds>) of size <len>
*/
dataset* sample_dataset(dataset* ds,int len,char* classfield);